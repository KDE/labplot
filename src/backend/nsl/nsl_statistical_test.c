/*
	File                 : nsl_stats.c
	Project              : LabPlot
	Description          : NSL statistical test functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-FileCopyrightText: 2025-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_statistical_test.h"
#include "nsl_stats.h"
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_statistics_double.h>
#include <math.h>
#include <stddef.h>

#define DECLARE_COMPARE_FUNCTION(struct_type, field_name)                                                                                                      \
	int compare_##struct_type##_##field_name(const void* a, const void* b) {                                                                                   \
		const struct_type* s1 = (const struct_type*)a;                                                                                                         \
		const struct_type* s2 = (const struct_type*)b;                                                                                                         \
		if (s1->field_name < s2->field_name)                                                                                                                   \
			return -1;                                                                                                                                         \
		if (s1->field_name > s2->field_name)                                                                                                                   \
			return 1;                                                                                                                                          \
		return 0;                                                                                                                                              \
	}

// Structure used for ranking in the Kruskal-Wallis and Mann-Whittney test
typedef struct {
	double value;
	size_t group;
	double rank;
} rank;
DECLARE_COMPARE_FUNCTION(rank, value)

// Observation structure for Log Rank test
typedef struct {
	double time;
	int status; // 1 = event occurred, 0 = censored
	size_t group; // 1 or 2
} observation;
DECLARE_COMPARE_FUNCTION(observation, time)

typedef struct {
	double abs_diff;
	int sign;
	int rank;
} signed_diff;
DECLARE_COMPARE_FUNCTION(signed_diff, abs_diff)

/* Mann-Whitney U test */
struct mannwhitney_test_result nsl_stats_mannwhitney_u(const double sample1[], size_t n1, const double sample2[], size_t n2, nsl_stats_tail_type tail) {
	size_t n = n1 + n2;

	rank* data = (rank*)malloc(n * sizeof(rank));

	for (size_t i = 0; i < n1; i++) {
		data[i].value = sample1[i];
		data[i].group = 0;
	}
	for (size_t i = 0; i < n2; i++) {
		data[n1 + i].value = sample2[i];
		data[n1 + i].group = 1;
	}

	qsort(data, n, sizeof(rank), compare_rank_value);

	double tie_term = 0.0;
	size_t i = 0;
	while (i < n) {
		size_t j = i + 1;
		while (j < n && data[j].value == data[i].value)
			j++;
		size_t tie_count = j - i;
		double avg_rank = ((double)(i + 1) + (double)j) / 2.0;
		for (size_t k = i; k < j; k++)
			data[k].rank = avg_rank;
		if (tie_count > 1)
			tie_term += (tie_count * tie_count * tie_count - tie_count) / 12.0;
		i = j;
	}

	double rank_sum1 = 0.0;
	double rank_sum2 = 0.0;
	for (size_t idx = 0; idx < n; idx++) {
		if (data[idx].group == 0)
			rank_sum1 += data[idx].rank;
		else if (data[idx].group == 1)
			rank_sum2 += data[idx].rank;
	}

	free(data);

	double U1 = n1 * n2 + (n1 * (n1 + 1)) / 2.0 - rank_sum1;
	double U2 = n1 * n2 + (n2 * (n2 + 1)) / 2.0 - rank_sum2;

	double mean_U = (n1 * n2) / 2.0;

	double U = fmin(U1, U2);

	double stderr_U = sqrt((n1 * n2) / (n * (n - 1.0))) * sqrt((((n * n * n) - n) / 12.0) - tie_term);

	double cc = NAN;
	switch (tail) {
	case nsl_stats_tail_type_two:
		cc = (U > mean_U) ? 0.5 : (U < mean_U ? -0.5 : 0.0);
		break;
	case nsl_stats_tail_type_positive:
		cc = 0.5;
		break;
	case nsl_stats_tail_type_negative:
		cc = -0.5;
		break;
	}

	double z = (U - mean_U - cc) / stderr_U;

	double norm_cdf = gsl_cdf_ugaussian_P(z);

	double p = NAN;
	switch (tail) {
	case nsl_stats_tail_type_two:
		p = 2.0 * fmin(norm_cdf, 1.0 - norm_cdf);
		break;
	case nsl_stats_tail_type_positive:
		p = 1.0 - norm_cdf;
		break;
	case nsl_stats_tail_type_negative:
		p = norm_cdf;
		break;
	}

	struct mannwhitney_test_result result;
	result.p = p;
	result.U = U;
	result.z = z;
	result.rank_sum1 = rank_sum1;
	result.rank_sum2 = rank_sum2;
	result.mean_rank1 = rank_sum1 / n1;
	result.mean_rank2 = rank_sum2 / n2;

	return result;
}

// one-way ANOVA
struct anova_oneway_test_result nsl_stats_anova_oneway_f(const double** groups, const size_t* sizes, size_t n_groups) {
	struct anova_oneway_test_result result;
	double grand_total = 0.0;
	size_t total_samples = 0;
	for (size_t i = 0; i < n_groups; i++) {
		for (size_t j = 0; j < sizes[i]; j++)
			grand_total += groups[i][j];

		total_samples += sizes[i];
	}
	double grand_mean = grand_total / total_samples;
	double ssb = 0.0;
	for (size_t i = 0; i < n_groups; i++) {
		double group_mean = gsl_stats_mean(groups[i], 1, sizes[i]);
		double diff = group_mean - grand_mean;
		ssb += sizes[i] * diff * diff;
	}
	double ms_between = ssb / (n_groups - 1);
	double ssw = 0.0;
	for (size_t i = 0; i < n_groups; i++)
		ssw += gsl_stats_variance(groups[i], 1, sizes[i]) * (sizes[i] - 1);

	const double delta = total_samples - n_groups;
	double F = NAN;
	if (delta != 0.) {
		const double ms_within = ssw / delta;
		result.ms_within_groups = ms_within;
		F = ms_between / ms_within;
	} else {
		result.p = NAN;
		return result;
	}

	size_t df_between = n_groups - 1;
	size_t df_within = 0;
	for (size_t i = 0; i < n_groups; i++)
		df_within += sizes[i];
	df_within -= n_groups;

	double p = nsl_stats_fdist_p(F, df_between, df_within);

	result.p = p;
	result.df_between_groups = df_between;
	result.df_within_groups = df_within;
	result.ss_between_groups = ssb;
	result.ss_within_groups = ssw;
	result.ms_between_groups = ms_between;
	result.F = F;

	return result;
}

/* one-way ANOVA with repeated measures */
struct anova_oneway_repeated_test_result nsl_stats_anova_oneway_repeated_f(const double** groups, size_t n_samples, size_t n_groups) {
	struct anova_oneway_repeated_test_result result;
	double grand_total = 0.0;
	size_t total_samples = n_samples * n_groups;
	double* sample_means = (double*)calloc(n_samples, sizeof(double));

	for (size_t i = 0; i < n_samples; i++) {
		for (size_t j = 0; j < n_groups; j++) {
			grand_total += groups[j][i];
			sample_means[i] += groups[j][i] / n_groups;
		}
	}

	if (total_samples == 0) {
		result.p = NAN;
		return result;
	}
	double grand_mean = grand_total / total_samples;

	double ssb = 0.0;
	for (size_t i = 0; i < n_groups; i++) {
		double group_mean = gsl_stats_mean(groups[i], 1, n_samples);
		double diff = group_mean - grand_mean;
		ssb += n_samples * diff * diff;
	}

	double ssw = 0.0;
	for (size_t i = 0; i < n_samples; i++) {
		for (size_t j = 0; j < n_groups; j++) {
			double diff = groups[j][i] - sample_means[i];
			ssw += diff * diff;
		}
	}

	free(sample_means);

	double sse = ssw - ssb;

	double dfb = n_groups - 1;
	double dfe = (n_groups - 1) * (n_samples - 1);

	if (dfb == 0. || dfe == 0.) {
		result.p = NAN;
		return result;
	}

	double msb = (ssb / dfb);
	double mse = (sse / dfe);

	double F = msb / mse;

	double p = nsl_stats_fdist_p(F, dfb, dfe);
	result.p = p;
	result.F = F;
	result.df_treatment = dfb;
	result.df_residuals = dfe;
	result.ss_treatment = ssb;
	result.ss_residuals = sse;
	result.ss_within_subjects = ssw;
	result.ms_treatment = msb;
	result.ms_residuals = mse;

	return result;
}

// Kruskal-Wallis
// improve soon
struct kruskal_wallis_test_result nsl_stats_kruskal_wallis_h(const double** groups, const size_t* sizes, size_t n_groups) {
	struct kruskal_wallis_test_result result;
	size_t total_samples = 0;
	size_t i, j;
	for (i = 0; i < n_groups; i++)
		total_samples += sizes[i];

	if (n_groups < 2) {
		fprintf(stderr, "Error: Kruskal-Wallis test requires at least two groups.\n");
		result.p = NAN;
		return result;
	}
	for (i = 0; i < n_groups; i++) {
		if (sizes[i] < 1) {
			fprintf(stderr, "Error: Each group must have at least one observation.\n");
			result.p = NAN;
			return result;
		}
	}
	rank* ranks = (rank*)malloc(total_samples * sizeof(rank));
	if (ranks == NULL) {
		fprintf(stderr, "Error: Memory allocation failed.\n");
		result.p = NAN;
		return result;
	}
	size_t idx = 0;

	for (i = 0; i < n_groups; i++) {
		for (j = 0; j < sizes[i]; j++)
			ranks[idx++] = (rank){groups[i][j], i, 0.0};
	}

	qsort(ranks, total_samples, sizeof(rank), compare_rank_value);

	double* rank_sum = (double*)calloc(n_groups, sizeof(double));
	if (rank_sum == NULL) {
		fprintf(stderr, "Error: Memory allocation failed.\n");
		free(ranks);
		result.p = NAN;
		return result;
	}
	size_t* tie_counts = NULL;
	size_t tie_count_size = 0;
	size_t tie_count_capacity = 0;
	size_t start = 0;

	while (start < total_samples) {
		size_t end = start;
		double sum_ranks = (double)(start + 1); // Ranks start at 1
		size_t count = 1;

		while (end + 1 < total_samples && ranks[end].value == ranks[end + 1].value) {
			end++;
			sum_ranks += (double)(end + 1);
			count++;
		}

		double average_rank = sum_ranks / (double)count;

		for (size_t k = start; k <= end; k++) {
			rank_sum[ranks[k].group] += average_rank;
		}

		if (count > 1) {
			if (tie_count_size == tie_count_capacity) {
				tie_count_capacity = (tie_count_capacity == 0) ? 4 : tie_count_capacity * 2;
				size_t* temp = (size_t*)realloc(tie_counts, tie_count_capacity * sizeof(size_t));
				if (temp == NULL) {
					fprintf(stderr, "Error: Memory allocation failed.\n");
					free(ranks);
					free(rank_sum);
					free(tie_counts);
					result.p = NAN;
					return result;
				}
				tie_counts = temp;
			}
			tie_counts[tie_count_size++] = count;
		}

		start = end + 1;
	}

	double H_numerator = 0.0;
	for (i = 0; i < n_groups; i++) {
		H_numerator += (rank_sum[i] * rank_sum[i]) / (double)sizes[i];
	}

	double H = (12.0 / ((double)total_samples * ((double)total_samples + 1.0))) * H_numerator - 3.0 * ((double)total_samples + 1.0);
	double sum_tj_correction = 0.0;
	for (i = 0; i < tie_count_size; i++) {
		double tj = (double)tie_counts[i];
		sum_tj_correction += (tj * tj * tj - tj);
	}
	double N_cubed_minus_N = ((double)total_samples * (double)total_samples * (double)total_samples) - ((double)total_samples);
	double tie_correction_factor = 1.0 - (sum_tj_correction / N_cubed_minus_N);

	double H_corrected = H / tie_correction_factor;

	free(ranks);
	free(rank_sum);
	free(tie_counts);

	double p = gsl_cdf_chisq_Q(H_corrected, n_groups - 1);
	if (p < 1.e-9)
		p = 0.0;

	result.p = p;
	result.H = H_corrected;
	result.df = n_groups - 1;
	return result;
}

// Log-Rank test
// improve soon
struct log_rank_test_result
nsl_stats_log_rank_h(const double* time, const int* status, const size_t* g1_ind, size_t size1, const size_t* g2_ind, size_t size2) {
	struct log_rank_test_result result;
	size_t total_size = size1 + size2;
	observation* observations = (observation*)malloc(total_size * sizeof(observation));
	if (observations == NULL) {
		fprintf(stderr, "Error: Memory allocation failed.\n");
		result.p = NAN;
		return result;
	}

	int event_count1 = 0;
	int event_count2 = 0;
	int censored_count1 = 0;
	int censored_count2 = 0;

	for (size_t i = 0; i < size1; i++) {
		observations[i].time = time[g1_ind[i]];
		int stat = status[g1_ind[i]];
		if (stat == 0)
			censored_count1 += 1;
		else if (stat == 1)
			event_count1 += 1;
		observations[i].status = stat;
		observations[i].group = 1;
	}
	for (size_t i = 0; i < size2; i++) {
		observations[size1 + i].time = time[g2_ind[i]];
		int stat = status[g2_ind[i]];
		if (stat == 0)
			censored_count2 += 1;
		else if (stat == 1)
			event_count2 += 1;
		observations[size1 + i].status = stat;
		observations[size1 + i].group = 2;
	}

	qsort(observations, total_size, sizeof(observation), compare_observation_time);

	size_t n_risk_1 = size1;
	size_t n_risk_2 = size2;
	size_t n_risk_total = total_size;

	double O1 = 0.0;
	double E1 = 0.0;
	double V1 = 0.0;

	size_t i = 0;
	while (i < total_size) {
		double current_time = observations[i].time;
		size_t j = i;

		while (j < total_size && observations[j].time == current_time) {
			j++;
		}

		size_t d = 0;
		size_t d1 = 0;
		// size_t d2 = 0;
		size_t n_risk_at_time_1 = n_risk_1;
		size_t n_risk_at_time_2 = n_risk_2;
		size_t n_risk_at_time_total = n_risk_total;

		for (size_t k = i; k < j; k++) {
			if (observations[k].status == 1) {
				d++;
				if (observations[k].group == 1)
					d1++;
				else {
					// d2++; // TODO: ?
				}
			}
		}

		if (d > 0) {
			double E1_t = ((double)n_risk_at_time_1 * (double)d) / (double)n_risk_at_time_total;
			E1 += E1_t;

			double V1_t = ((double)n_risk_at_time_1 * (double)n_risk_at_time_2 * (double)d * (double)(n_risk_at_time_total - d))
				/ (pow((double)n_risk_at_time_total, 2) * ((double)n_risk_at_time_total - 1.0));
			V1 += V1_t;

			O1 += (double)d1;
		}

		for (size_t k = i; k < j; k++) {
			if (observations[k].status == 1) {
				if (observations[k].group == 1)
					n_risk_1--;
				else
					n_risk_2--;
				n_risk_total--;
			} else {
				if (observations[k].group == 1)
					n_risk_1--;
				else
					n_risk_2--;
				n_risk_total--;
			}
		}

		i = j;
	}

	free(observations);

	if (V1 <= 0.0) {
		fprintf(stderr, "Error: Variance is zero or negative. Cannot compute test statistic.\n");
		result.p = NAN;
		return result;
	}
	double H = pow(O1 - E1, 2) / V1;
	double p = gsl_cdf_chisq_Q(H, 1);
	if (p < 1.e-9)
		p = 0.0;

	result.p = p;
	result.H = H;
	result.df = 0;
	result.event_count1 = event_count1;
	result.event_count2 = event_count2;
	result.censored_count1 = censored_count1;
	result.censored_count2 = censored_count2;
	result.total_count1 = size1;
	result.total_count2 = size2;

	return result;
}

/* Independent Sample Student's t-test */
struct independent_t_test_result nsl_stats_independent_t(const double sample1[], size_t n1, const double sample2[], size_t n2, nsl_stats_tail_type tail) {
	double mean1 = 0.0, mean2 = 0.0, var1 = 0.0, var2 = 0.0;
	for (size_t i = 0; i < n1; i++)
		mean1 += sample1[i];
	mean1 /= n1;

	for (size_t i = 0; i < n2; i++)
		mean2 += sample2[i];
	mean2 /= n2;
	for (size_t i = 0; i < n1; i++)
		var1 += (sample1[i] - mean1) * (sample1[i] - mean1);
	var1 /= (n1 - 1);

	for (size_t i = 0; i < n2; i++)
		var2 += (sample2[i] - mean2) * (sample2[i] - mean2);
	var2 /= (n2 - 1);
	double pooled_variance = ((n1 - 1) * var1 + (n2 - 1) * var2) / (n1 + n2 - 2);
	double standard_error = sqrt(pooled_variance * (1.0 / n1 + 1.0 / n2));
	double t_stat = (mean1 - mean2) / standard_error;

	size_t df = n1 + n2 - 2;
	double p_value = NAN;
	switch (tail) {
	case nsl_stats_tail_type_two:
		p_value = 2.0 * (1.0 - gsl_cdf_tdist_P(fabs(t_stat), df));
		break;
	case nsl_stats_tail_type_negative: // Left-tailed: p = two_tailed_p / 2 if t < 0, else 1 - two_tailed_p / 2
		p_value = gsl_cdf_tdist_P(t_stat, df);
		break;
	case nsl_stats_tail_type_positive: // Right-tailed: p = two_tailed_p / 2 if t > 0, else 1 - two_tailed_p / 2
		p_value = 1.0 - gsl_cdf_tdist_P(t_stat, df);
		break;
	}

	struct independent_t_test_result result;
	result.p = p_value;
	result.t = t_stat;
	result.df = df;
	result.mean1 = mean1;
	result.variance1 = var1;
	result.mean_standard_error1 = sqrt(var1 / n1);
	result.mean2 = mean2;
	result.variance2 = var2;
	result.mean_standard_error2 = sqrt(var2 / n2);
	result.pooled_variance = pooled_variance;
	result.mean_difference_standard_error = standard_error;
	result.mean_difference = fabs(mean1 - mean2);

	return result;
}

/* One Sample Student's t-test */
struct one_sample_t_test_result nsl_stats_one_sample_t(const double sample[], size_t n, double hypothesized_mean, nsl_stats_tail_type tail) {
	double mean = 0.0, variance = 0.0;
	for (size_t i = 0; i < n; i++)
		mean += sample[i];
	mean /= n;
	for (size_t i = 0; i < n; i++)
		variance += (sample[i] - mean) * (sample[i] - mean);
	variance /= (n - 1);
	double standard_error = sqrt(variance / n);
	double t_stat = (mean - hypothesized_mean) / standard_error;

	size_t df = n - 1;
	double p_value = 0.0;
	switch (tail) {
	case nsl_stats_tail_type_two: // Two-tailed test: p = 2 * (1 - P(T ≤ |t_stat|))
		p_value = 2.0 * (1.0 - gsl_cdf_tdist_P(fabs(t_stat), df));
		break;
	case nsl_stats_tail_type_negative: // Left-tailed test: p = P(T ≤ t_stat)
		p_value = gsl_cdf_tdist_P(t_stat, df);
		break;
	case nsl_stats_tail_type_positive: // Right-tailed test: p = P(T ≥ t_stat) = 1 - P(T ≤ t_stat)
		p_value = 1.0 - gsl_cdf_tdist_P(t_stat, df);
		break;
	}

	struct one_sample_t_test_result result;
	result.p = p_value;
	result.df = df;
	result.t = t_stat;
	result.mean_standard_error = standard_error;
	result.variance = variance;
	result.mean = mean;
	result.mean_difference = fabs(mean - hypothesized_mean);

	return result;
}

// Logistic Regression Function for Binary Classification
// Caller owns returned pointer and must free()
// TODO: return struct {b, W}
double* nsl_stats_logistic_regression(double** x, const int* y, int N, int n_in, int iterations, double lr) {
	if (N <= 0 || n_in <= 0 || x == NULL || y == NULL)
		return NULL;

	double* W = (double*)malloc(sizeof(double) * n_in);
	double* result = (double*)malloc(sizeof(double) * (n_in + 1));
	if (!W || !result)
		return NULL;

	for (int i = 0; i < n_in; i++)
		W[i] = 0.0;

	double b = 0.0;
	double y_pred, error;
	for (int epoch = 0; epoch < iterations; epoch++) {
		for (int i = 0; i < N; i++) {
			y_pred = b;
			for (int j = 0; j < n_in; j++)
				y_pred += W[j] * x[i][j];
			y_pred = 1.0 / (1.0 + exp(-y_pred));

			error = y[i] - y_pred;
			for (int j = 0; j < n_in; j++)
				W[j] += lr * error * x[i][j];
			b += lr * error;
		}
	}
	result[0] = b;
	for (int i = 0; i < n_in; i++)
		result[i + 1] = W[i];
	free(W);

	return result;
}

static int compare_time_univariate_cox(const void* a, const void* b) {
	double t1 = ((const double*)a)[0];
	double t2 = ((const double*)b)[0];
	if (t1 < t2)
		return -1;
	if (t1 > t2)
		return 1;
	return 0;
}

double nsl_univariate_cox_regression(const double* x, const double* time, const int* event, int N, int iterations, double lr) {
	double* time_idx = (double*)malloc(2 * N * sizeof(double));
	for (int i = 0; i < N; i++) {
		time_idx[2 * i] = time[i]; /* time */
		time_idx[2 * i + 1] = (double)i; /* original index */
	}
	qsort(time_idx, N, 2 * sizeof(double), compare_time_univariate_cox);
	double beta = 0.0;
	double* exp_bx = (double*)malloc(N * sizeof(double));

	for (int iter = 0; iter < iterations; iter++) {
		for (int i = 0; i < N; i++)
			exp_bx[i] = exp(beta * x[i]);

		double grad = 0.0;
		double S0 = 0.0;
		double S1 = 0.0;
		for (int k = N - 1; k >= 0; k--) {
			int i = (int)time_idx[2 * k + 1];
			S0 += exp_bx[i];
			S1 += x[i] * exp_bx[i];

			if (event[i] == 1)
				grad += x[i] - (S1 / S0);
		}
		beta += lr * grad;
	}
	double hr = exp(beta);
	free(exp_bx);
	free(time_idx);
	return hr;
}

/* Welch t-test */
struct welch_t_test_result nsl_stats_welch_t(const double sample1[], size_t n1, const double sample2[], size_t n2, nsl_stats_tail_type tail) {
	double mean1 = 0.0, mean2 = 0.0, var1 = 0.0, var2 = 0.0;
	for (size_t i = 0; i < n1; i++)
		mean1 += sample1[i];
	mean1 /= n1;

	for (size_t i = 0; i < n2; i++)
		mean2 += sample2[i];
	mean2 /= n2;
	for (size_t i = 0; i < n1; i++)
		var1 += (sample1[i] - mean1) * (sample1[i] - mean1);
	var1 /= (n1 - 1);

	for (size_t i = 0; i < n2; i++)
		var2 += (sample2[i] - mean2) * (sample2[i] - mean2);
	var2 /= (n2 - 1);

	double stderrsq1 = var1 / n1;
	double stderrsq2 = var2 / n2;
	double standard_error = sqrt(stderrsq1 + stderrsq2);

	double t_stat = (mean1 - mean2) / standard_error;

	double df = pow(stderrsq1 + stderrsq2, 2) / (pow(stderrsq1, 2) / (n1 - 1) + pow(stderrsq2, 2) / (n2 - 1));

	double p_value = 0.0;
	switch (tail) {
	case nsl_stats_tail_type_two:
		p_value = 2.0 * (1.0 - gsl_cdf_tdist_P(fabs(t_stat), df));
		break;
	case nsl_stats_tail_type_negative: // Left-tailed
		p_value = gsl_cdf_tdist_P(t_stat, df);
		break;
	case nsl_stats_tail_type_positive: // Right-tailed
		p_value = 1.0 - gsl_cdf_tdist_P(t_stat, df);
		break;
	}

	struct welch_t_test_result result;
	result.p = p_value;
	result.t = t_stat;
	result.df = df;
	result.mean1 = mean1;
	result.variance1 = var1;
	result.mean_standard_error1 = sqrt(stderrsq1);
	result.mean2 = mean2;
	result.variance2 = var2;
	result.mean_standard_error2 = sqrt(stderrsq2);
	result.mean_difference_standard_error = standard_error;
	result.mean_difference = fabs(mean1 - mean2);

	return result;
}

/* Wilcoxon signed rank test */
struct wilcoxon_test_result nsl_stats_wilcoxon_w(const double sample1[], const double sample2[], size_t n, nsl_stats_tail_type tail) {
	struct wilcoxon_test_result result;

	signed_diff* diffs = (signed_diff*)malloc(n * sizeof(signed_diff));

	size_t N = 0;
	for (size_t i = 0; i < n; ++i) {
		double d = sample1[i] - sample2[i];
		if (d != 0.0) { // only take non-zero differences
			diffs[N].abs_diff = fabs(d);
			diffs[N].sign = (d > 0.0) ? 1 : -1;
			N++;
		}
	}

	if (N == 0) {
		free(diffs);
		result.p = NAN;
		return result;
	}

	qsort(diffs, N, sizeof(signed_diff), compare_signed_diff_abs_diff);

	double tie_term = 0.0;
	double T_plus = 0.0;
	int nT_plus = 0;
	double T_minus = 0.0;
	int nT_minus = 0;
	int ties = 0;
	size_t i = 0;
	while (i < N) {
		size_t j = i + 1;
		while (j < N && diffs[j].abs_diff == diffs[i].abs_diff)
			j++;
		size_t tie_count = j - i;
		double avg_rank = ((double)(i + 1) + (double)j) / 2.0;
		for (size_t k = i; k < j; k++) {
			diffs[k].rank = avg_rank;
			if (diffs[k].sign == 1) {
				T_plus += avg_rank;
				nT_plus += 1;
			} else if (diffs[k].sign == -1) {
				T_minus += avg_rank;
				nT_minus += 1;
			}
		}
		if (tie_count > 1) {
			tie_term += tie_count * tie_count * tie_count - tie_count;
			ties += 1;
		}
		i = j;
	}

	double T = tail == nsl_stats_tail_type_two ? fmin(T_plus, T_minus) : T_plus;

	double mu_T = (double)N * (N + 1.0) / 4.0;
	double var_T = ((double)N * (N + 1.0) * (2.0 * N + 1.0) - tie_term / 2) / 24.0;

	if (var_T <= 0.0) {
		free(diffs);
		result.p = NAN;
		return result;
	}
	double sigma_T = sqrt(var_T);

	double cc = NAN;
	switch (tail) {
	case nsl_stats_tail_type_two:
		cc = (T > mu_T) ? 0.5 : (T < mu_T ? -0.5 : 0.0);
		break;
	case nsl_stats_tail_type_positive:
		cc = 0.5;
		break;
	case nsl_stats_tail_type_negative:
		cc = -0.5;
		break;
	}

	double z = (T - cc - mu_T) / sigma_T;

	double norm_cdf = gsl_cdf_ugaussian_P(z);

	double p = NAN;
	switch (tail) {
	case nsl_stats_tail_type_two:
		p = 2.0 * fmin(norm_cdf, 1.0 - norm_cdf);
		break;
	case nsl_stats_tail_type_positive:
		p = 1.0 - norm_cdf;
		break;
	case nsl_stats_tail_type_negative:
		p = norm_cdf;
		break;
	}

	free(diffs);

	result.p = p;
	result.z = z;
	result.W = T;
	result.tie_count = ties;
	result.positive_rank_sum = T_plus;
	result.positive_rank_mean = T_plus / nT_plus;
	result.positive_rank_count = nT_plus;
	result.negative_rank_sum = T_minus;
	result.negative_rank_mean = T_minus / nT_minus;
	result.negative_rank_count = nT_minus;

	return result;
}

/* Friedman test */
struct friedman_test_result nsl_stats_friedman_q(const double** groups, size_t n_samples, size_t n_groups) {
	rank* row = (rank*)malloc(sizeof(rank) * n_groups);
	double* rank_sums = (double*)calloc(n_groups, sizeof(double));

	for (size_t k = 0; k < n_samples; ++k) {
		for (size_t j = 0; j < n_groups; ++j) {
			row[j].value = groups[j][k];
			row[j].group = j;
		}

		qsort(row, n_groups, sizeof(rank), compare_rank_value);

		size_t i = 0;
		while (i < n_groups) {
			size_t j = i + 1;
			while (j < n_groups && row[j].value == row[i].value)
				j++;
			double avg_rank = ((double)(i + 1) + (double)j) / 2.0;
			for (size_t n = i; n < j; n++) {
				row[n].rank = avg_rank;
				rank_sums[row[n].group] += avg_rank;
			}
			i = j;
		}
	}

	free(row);

	double sum = 0.0;
	for (size_t j = 0; j < n_groups; ++j) {
		sum += pow((rank_sums[j] / n_samples) - (n_groups + 1.0) / 2, 2);
	}
	free(rank_sums);

	double Q = ((12.0 * n_samples) / (n_groups * (n_groups + 1))) * sum;

	int df = n_groups - 1;
	double p_value = gsl_cdf_chisq_Q(Q, (double)df);

	struct friedman_test_result result;
	result.p = p_value;
	result.Q = Q;
	result.df = df;

	return result;
}

/* Chi square Independence test */
struct chisq_ind_test_result nsl_stats_chisq_ind_x2(const long long** table, size_t row, size_t column) {
	long long total = 0;

	long long* row_sums = (long long*)calloc(row, sizeof(long long));
	for (size_t i = 0; i < row; ++i) {
		for (size_t j = 0; j < column; ++j) {
			row_sums[i] += table[j][i];
			total += table[j][i];
		}
	}

	long long* column_sums = (long long*)calloc(column, sizeof(long long));
	for (size_t j = 0; j < column; ++j) {
		for (size_t i = 0; i < row; ++i) {
			column_sums[j] += table[j][i];
		}
	}

	double x2 = 0.0;
	for (size_t i = 0; i < row; ++i) {
		for (size_t j = 0; j < column; ++j) {
			double expected = (row_sums[i] * column_sums[j]) / (double)total;
			double diff = table[j][i] - expected;
			x2 += (diff * diff) / expected;
		}
	}

	free(row_sums);
	free(column_sums);

	int df = (row - 1) * (column - 1);
	double p_value = gsl_cdf_chisq_Q(x2, (double)df);

	struct chisq_ind_test_result result;
	result.p = p_value;
	result.x2 = x2;
	result.total_observed_frequencies = total;
	result.df = df;

	return result;
}

/* Chi square Goodness of Fit Test */
struct chisq_gof_test_result nsl_stats_chisq_gof_x2(const long long* observed, const double* expected, size_t n, size_t params_estimated) {
	double x2 = 0.0;

	for (size_t i = 0; i < n; ++i) {
		double diff = observed[i] - expected[i];
		x2 += (diff * diff) / expected[i];
	}

	size_t df = n - 1 - params_estimated;

	double p_value = gsl_cdf_chisq_Q(x2, (double)df);

	struct chisq_gof_test_result result;
	result.x2 = x2;
	result.p = p_value;
	result.df = df;

	return result;
}

/* Mann-Kendall Test for monotonic trend detection */
struct mann_kendall_test_result nsl_stats_mann_kendall(const double sample[], size_t n, nsl_stats_tail_type tail) {
	struct mann_kendall_test_result result;
	result.n = n;
	result.S = 0.0;
	result.tau = NAN;
	result.z = NAN;
	result.p = NAN;

	if (n < 3) {
		fprintf(stderr, "Error: Mann-Kendall test requires at least 3 data points.\n");
		return result;
	}

	// calculate S statistic (sum of signs of all pairwise differences) and
	// the Sen's slope (median of all pairwise slopes)
	double S = 0.0;
	const double n_pairs = (double)(n * (n - 1)) / 2.0;
	double* slopes = (double*)malloc(sizeof(double) * (size_t)n_pairs);
	size_t slope_idx = 0;
	for (size_t i = 0; i < n - 1; i++) {
		for (size_t j = i + 1; j < n; j++) {
			double diff = sample[j] - sample[i];
			if (diff > 0.0)
				S += 1.0;
			else if (diff < 0.0)
				S -= 1.0;

			double slope = (sample[j] - sample[i]) / (double)(j - i);
			slopes[slope_idx++] = slope;
		}
	}
	result.S = S;

	gsl_sort(slopes, 1, (size_t)n_pairs);
	double slope = gsl_stats_median_from_sorted_data(slopes, 1, (size_t)n_pairs);
	free(slopes);
	result.slope = slope;

	// calculate Kendall's tau, the "correlation coefficient" for the trend, ranging from -1 to +1.
	result.tau = S / n_pairs;

	// calculate the variance of S:
	// for simplicity, we assume no ties in the data
	// variance without tie correction: Var(S) = n(n-1)(2n+5)/18
	double var_S = (double)(n * (n - 1) * (2 * n + 5)) / 18.0;

	// calculate Z-score
	double z;
	if (S > 0.0)
		z = (S - 1.0) / sqrt(var_S);
	else if (S < 0.0)
		z = (S + 1.0) / sqrt(var_S);
	else
		z = 0.0;
	result.z = z;

	// calculate p-value based on tail type
	double p;
	switch (tail) {
	case nsl_stats_tail_type_two:
		p = 2.0 * fmin(gsl_cdf_ugaussian_P(fabs(z)), 1.0 - gsl_cdf_ugaussian_P(fabs(z)));
		break;
	case nsl_stats_tail_type_positive: // testing for positive trend
		p = 1.0 - gsl_cdf_ugaussian_P(z);
		break;
	case nsl_stats_tail_type_negative: // testing for negative trend
		p = gsl_cdf_ugaussian_P(z);
		break;
	default:
		p = NAN;
		break;
	}
	result.p = p;

	return result;
}
/* Wald-Wolfowitz Runs Test for randomness */
struct wald_wolfowitz_runs_test_result nsl_stats_wald_wolfowitz_runs(const double sample[], size_t n, nsl_stats_tail_type tail) {
	struct wald_wolfowitz_runs_test_result result;
	result.n = n;
	result.runs = 0;
	result.z = NAN;
	result.p = NAN;

	if (n < 3) {
		fprintf(stderr, "Error: Wald-Wolfowitz runs test requires at least 3 data points.\n");
		return result;
	}

	// Calculate the median
	double* sorted = (double*)malloc(n * sizeof(double));
	for (size_t i = 0; i < n; i++)
		sorted[i] = sample[i];
	gsl_sort(sorted, 1, n);
	double median = gsl_stats_median_from_sorted_data(sorted, 1, n);
	free(sorted);

	// Count runs: a run is a sequence of consecutive values above or below the median
	// Values equal to the median are excluded from the analysis
	int runs = 0;
	int last_sign = 0; // 0 = unknown, 1 = above median, -1 = below median
	int valid_n = 0; // count of values not equal to median

	for (size_t i = 0; i < n; i++) {
		int current_sign = 0;
		if (sample[i] > median) {
			current_sign = 1;
			valid_n++;
		} else if (sample[i] < median) {
			current_sign = -1;
			valid_n++;
		} else {
			continue; // skip values equal to median
		}

		if (last_sign == 0) {
			// First valid value
			runs = 1;
		} else if (current_sign != last_sign) {
			// Sign changed, new run
			runs++;
		}
		last_sign = current_sign;
	}

	result.runs = runs;
	result.n = valid_n;

	if (valid_n < 3) {
		fprintf(stderr, "Error: Insufficient data points after removing median values.\n");
		return result;
	}

	// Count values above and below median
	int n1 = 0; // count above median
	int n2 = 0; // count below median
	for (size_t i = 0; i < n; i++) {
		if (sample[i] > median)
			n1++;
		else if (sample[i] < median)
			n2++;
	}

	// Calculate expected number of runs and variance
	// For the runs test: E(R) = (2*n1*n2)/(n1+n2) + 1
	//                    Var(R) = (2*n1*n2*(2*n1*n2 - n1 - n2)) / ((n1+n2)^2 * (n1+n2-1))
	double n_total = (double)(n1 + n2);
	double expected_runs = (2.0 * n1 * n2) / n_total + 1.0;
	double var_runs = (2.0 * n1 * n2 * (2.0 * n1 * n2 - n_total)) / (n_total * n_total * (n_total - 1.0));

	// Calculate z-score using continuity correction
	double z;
	if (runs > expected_runs)
		z = (runs - 0.5 - expected_runs) / sqrt(var_runs);
	else if (runs < expected_runs)
		z = (runs + 0.5 - expected_runs) / sqrt(var_runs);
	else
		z = 0.0;

	result.z = z;

	// Calculate p-value based on tail type
	double p;
	switch (tail) {
	case nsl_stats_tail_type_two:
		// Two-tailed: test for any non-randomness
		p = 2.0 * fmin(gsl_cdf_ugaussian_P(fabs(z)), 1.0 - gsl_cdf_ugaussian_P(fabs(z)));
		break;
	case nsl_stats_tail_type_negative:
		// Left-tailed: test for too few runs (clustering)
		p = gsl_cdf_ugaussian_P(z);
		break;
	case nsl_stats_tail_type_positive:
		// Right-tailed: test for too many runs (alternating)
		p = 1.0 - gsl_cdf_ugaussian_P(z);
		break;
	default:
		p = NAN;
		break;
	}
	result.p = p;

	return result;
}
