/*
	File                 : nsl_stats.c
	Project              : LabPlot
	Description          : NSL statistical test functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_statistical_test.h"
#include "nsl_stats.h"
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_statistics_double.h>
#include <math.h>

/* Mann-Whitney U test */
double nsl_stats_mannwhitney_u(const double sample1[], size_t n1, const double sample2[], size_t n2) {
	size_t n = n1 + n2;
	Rank combined[1000];

	for (size_t i = 0; i < n1; i++) {
		combined[i].value = sample1[i];
		combined[i].group = 0;
	}
	for (size_t i = 0; i < n2; i++) {
		combined[n1 + i].value = sample2[i];
		combined[n1 + i].group = 1;
	}

	for (size_t i = 0; i < n - 1; i++) {
		for (size_t j = i + 1; j < n; j++) {
			if (combined[i].value > combined[j].value) {
				Rank temp = combined[i];
				combined[i] = combined[j];
				combined[j] = temp;
			}
		}
	}

	double ranks[1000];
	size_t i = 0;
	while (i < n) {
		size_t j = i;
		while (j < n && combined[j].value == combined[i].value)
			j++;
		double rank = (i + j + 1) / 2.0;
		for (size_t k = i; k < j; k++)
			ranks[k] = rank;
		i = j;
	}

	double rank_sum1 = 0.0;
	for (size_t idx = 0; idx < n; idx++) {
		if (combined[idx].group == 0)
			rank_sum1 += ranks[idx];
	}

	double U1 = rank_sum1 - (n1 * (n1 + 1)) / 2.0;
	// double U2 = n1 * n2 - U1;
	return U1;
}

double nsl_stats_mannwhitney_p(double U, size_t n1, size_t n2) {
	double mean_U = (n1 * n2) / 2.0;
	double variance_U = (n1 * n2 * (n1 + n2 + 1)) / 12.0;
	double sigma = sqrt(variance_U);

	double z_abs = fabs(U - mean_U) - 0.5;
	double z = z_abs / sigma;

	double p_value = 2.0 * (1.0 - 0.5 * (1.0 + erf(z / sqrt(2.0))));
	return p_value;
}

// one-way ANOVA
double nsl_stats_anova_oneway_f(double** groups, size_t* sizes, size_t n_groups) {
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
	if (delta != 0.) {
		const double ms_within = ssw / delta;
		return ms_between / ms_within;
	} else
		return NAN;
}

double nsl_stats_anova_oneway_p(double** groups, size_t* sizes, size_t n_groups) {
	double F = nsl_stats_anova_oneway_f(groups, sizes, n_groups);

	size_t df_between = n_groups - 1;
	size_t df_within = 0;
	for (size_t i = 0; i < n_groups; i++)
		df_within += sizes[i];
	df_within -= n_groups;

	double p = nsl_stats_fdist_p(F, df_between, df_within);

	return p;
}

// Helper function to compare Rank values for sorting
int compare_rank(const void* a, const void* b) {
	Rank* rank_a = (Rank*)a;
	Rank* rank_b = (Rank*)b;
	if (rank_a->value < rank_b->value)
		return -1;
	if (rank_a->value > rank_b->value)
		return 1;
	return 0;
}

// Kruskal-Wallis
double nsl_stats_kruskal_wallis_h(double** groups, size_t* sizes, size_t n_groups) {
	size_t total_samples = 0;
	size_t i, j;
	for (i = 0; i < n_groups; i++)
		total_samples += sizes[i];

	if (n_groups < 2) {
		fprintf(stderr, "Error: Kruskal-Wallis test requires at least two groups.\n");
		return NAN;
	}
	for (i = 0; i < n_groups; i++) {
		if (sizes[i] < 1) {
			fprintf(stderr, "Error: Each group must have at least one observation.\n");
			return NAN;
		}
	}
	Rank* ranks = (Rank*)malloc(total_samples * sizeof(Rank));
	if (ranks == NULL) {
		fprintf(stderr, "Error: Memory allocation failed.\n");
		return NAN;
	}
	size_t idx = 0;

	for (i = 0; i < n_groups; i++) {
		for (j = 0; j < sizes[i]; j++)
			ranks[idx++] = (Rank){groups[i][j], i};
	}

	qsort(ranks, total_samples, sizeof(Rank), compare_rank);

	double* rank_sum = (double*)calloc(n_groups, sizeof(double));
	if (rank_sum == NULL) {
		fprintf(stderr, "Error: Memory allocation failed.\n");
		free(ranks);
		return NAN;
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
				size_t* temp = realloc(tie_counts, tie_count_capacity * sizeof(size_t));
				if (temp == NULL) {
					fprintf(stderr, "Error: Memory allocation failed.\n");
					free(ranks);
					free(rank_sum);
					free(tie_counts);
					return NAN;
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

	return H_corrected;
}

double nsl_stats_kruskal_wallis_p(double** groups, size_t* sizes, size_t n_groups) {
	double H = nsl_stats_kruskal_wallis_h(groups, sizes, n_groups);
	double p = gsl_cdf_chisq_Q(H, n_groups - 1);
	if (p < 1.e-9)
		p = 0.0;
	return p;
}

// Compare function for sorting indices by time
int compare_time_log_test(const void* a, const void* b) {
	const Observation* obs_a = (const Observation*)a;
	const Observation* obs_b = (const Observation*)b;
	if (obs_a->time < obs_b->time)
		return -1;
	if (obs_a->time > obs_b->time)
		return 1;
	return 0;
}

// Log-Rank test
double nsl_stats_log_rank_test_statistic(const double* time,
										 const int* status,
										 const size_t* group1_indices,
										 size_t size1,
										 const size_t* group2_indices,
										 size_t size2) {
	size_t total_size = size1 + size2;
	Observation* observations = (Observation*)malloc(total_size * sizeof(Observation));
	if (observations == NULL) {
		fprintf(stderr, "Error: Memory allocation failed.\n");
		return NAN;
	}

	for (size_t i = 0; i < size1; i++) {
		observations[i].time = time[group1_indices[i]];
		observations[i].status = status[group1_indices[i]];
		observations[i].group = 1;
	}
	for (size_t i = 0; i < size2; i++) {
		observations[size1 + i].time = time[group2_indices[i]];
		observations[size1 + i].status = status[group2_indices[i]];
		observations[size1 + i].group = 2;
	}

	qsort(observations, total_size, sizeof(Observation), compare_time_log_test);

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
				if (observations[k].group == 1) {
					d1++;
				} else {
					// d2++;
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
				if (observations[k].group == 1) {
					n_risk_1--;
				} else {
					n_risk_2--;
				}
				n_risk_total--;
			} else {
				if (observations[k].group == 1) {
					n_risk_1--;
				} else {
					n_risk_2--;
				}
				n_risk_total--;
			}
		}

		i = j;
	}

	free(observations);

	if (V1 <= 0.0) {
		fprintf(stderr, "Error: Variance is zero or negative. Cannot compute test statistic.\n");
		return NAN;
	}
	double H = pow(O1 - E1, 2) / V1;
	return H;
}

double
nsl_stats_log_rank_test_p(const double* time, const int* status, const size_t* group1_indices, size_t size1, const size_t* group2_indices, size_t size2) {
	double log_rank_stat = nsl_stats_log_rank_test_statistic(time, status, group1_indices, size1, group2_indices, size2);
	double p = gsl_cdf_chisq_Q(log_rank_stat, 1);
	if (p < 1.e-9)
		p = 0.0;
	return p;
}

/* Independent Sample Student's t-test */
double nsl_stats_independent_t(const double sample1[], size_t n1, const double sample2[], size_t n2) {
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
	return t_stat;
}

double nsl_stats_independent_t_p(const double sample1[], size_t n1, const double sample2[], size_t n2) {
	size_t df = n1 + n2 - 2;
	double t_stat = nsl_stats_independent_t(sample1, n1, sample2, n2);
	double t_abs = fabs(t_stat);
	double p_value = 2.0 * (1.0 - gsl_cdf_tdist_P(t_abs, df));
	return p_value;
}

/* One Sample Student's t-test */
double nsl_stats_one_sample_t(const double sample[], size_t n, double hypothesized_mean) {
	double mean = 0.0, variance = 0.0;
	for (size_t i = 0; i < n; i++)
		mean += sample[i];
	mean /= n;
	for (size_t i = 0; i < n; i++)
		variance += (sample[i] - mean) * (sample[i] - mean);
	variance /= (n - 1);
	double standard_error = sqrt(variance / n);
	double t_stat = (mean - hypothesized_mean) / standard_error;
	return t_stat;
}

/* One Sample Student's t-test p-value
 * tail parameter:
 *    0 → two-tailed
 *    1 → left-tailed
 *    2 → right-tailed
 */
double nsl_stats_one_sample_t_p(const double sample[], size_t n, double hypothesized_mean, int tail) {
	size_t df = n - 1;
	double t_stat = nsl_stats_one_sample_t(sample, n, hypothesized_mean);
	double p_value = 0.0;
	switch (tail) {
	case 1: // Left-tailed test: p = P(T ≤ t_stat)
		p_value = gsl_cdf_tdist_P(t_stat, df);
		break;
	case 2: // Right-tailed test: p = P(T ≥ t_stat) = 1 - P(T ≤ t_stat)
		p_value = 1.0 - gsl_cdf_tdist_P(t_stat, df);
		break;
	default: // Two-tailed test: p = 2 * (1 - P(T ≤ |t_stat|))
		p_value = 2.0 * (1.0 - gsl_cdf_tdist_P(fabs(t_stat), df));
		break;
	}
	return p_value;
}

// Logistic Regression Function for Binary Classification
double* nsl_stats_logistic_regression(double** x, int* y, int N, int n_in, int iterations, double lr) {
	int i, j, epoch;
	double* result;
	double* W = 0;
	double b = 0.0;
	double y_pred, gradient;

	result = (double*)malloc(sizeof(double) * (n_in + 1));

	// TODO: W is uninitialized!
	for (i = 0; i < n_in; i++) {
		W[i] = 0.0;
	}
	result[0] = b;

	for (epoch = 0; epoch < iterations; epoch++) {
		for (i = 0; i < N; i++) {
			y_pred = b;
			for (j = 0; j < n_in; j++) {
				y_pred += W[j] * x[i][j];
			}
			y_pred = 1.0 / (1.0 + exp(-y_pred));

			gradient = y[i] - y_pred; // Error term
			for (j = 0; j < n_in; j++) {
				W[j] += lr * gradient * x[i][j];
			}
			b += lr * gradient;
		}
	}
	result[0] = b;
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

double nsl_univariate_cox_regression(double* x, double* time, int* event, int N, int iterations, double lr) {
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
