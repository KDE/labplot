/*
	File                 : nsl_stats.c
	Project              : LabPlot
	Description          : NSL statistics functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2017 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nsl_stats.h"
#include <float.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_statistics_double.h>
#include <math.h>

double nsl_stats_minimum(const double data[], const size_t n, size_t* index) {
	size_t i;

	double min = data[0];
	if (index != NULL)
		*index = 0;

	for (i = 1; i < n; i++) {
		if (data[i] < min) {
			min = data[i];
			if (index != NULL)
				*index = i;
		}
	}

	return min;
}

double nsl_stats_maximum(const double data[], const size_t n, size_t* index) {
	size_t i;

	double max = data[0];
	if (index != NULL)
		*index = 0;

	for (i = 1; i < n; i++) {
		if (data[i] > max) {
			max = data[i];
			if (index != NULL)
				*index = i;
		}
	}

	return max;
}

double nsl_stats_median(double data[], size_t stride, size_t n, nsl_stats_quantile_type type) {
	gsl_sort(data, stride, n);
	return nsl_stats_median_sorted(data, stride, n, type);
}

double nsl_stats_median_sorted(const double sorted_data[], size_t stride, size_t n, nsl_stats_quantile_type type) {
	return nsl_stats_quantile_sorted(sorted_data, stride, n, 0.5, type);
}

double nsl_stats_median_from_sorted_data(const double sorted_data[], size_t stride, size_t n) {
	// default method is number 7
	return nsl_stats_median_sorted(sorted_data, stride, n, nsl_stats_quantile_type7);
}

double nsl_stats_quantile(double data[], size_t stride, size_t n, double p, nsl_stats_quantile_type type) {
	gsl_sort(data, stride, n);
	return nsl_stats_quantile_sorted(data, stride, n, p, type);
}

double nsl_stats_quantile_sorted(const double d[], size_t stride, size_t n, double p, nsl_stats_quantile_type type) {
	switch (type) {
	case nsl_stats_quantile_type1: // h = Np + 1/2, x[ceil(h – 1/2)]
		if (p == 0.0)
			return d[0];
		else
			return d[((int)ceil(n * p) - 1) * stride];
	case nsl_stats_quantile_type2:
		if (p == 0.0)
			return d[0];
		else if (p == 1.0)
			return d[(n - 1) * stride];
		else
			return (d[((int)ceil(n * p) - 1) * stride] + d[((int)ceil(n * p + 1) - 1) * stride]) / 2.;
	case nsl_stats_quantile_type3:
		if (p <= 0.5 / n)
			return d[0];
		else
#ifdef _WIN32
			return d[((int)round(n * p) - 1) * stride];
#else
			return d[(lrint(n * p) - 1) * stride];
#endif
	case nsl_stats_quantile_type4:
		if (p < 1. / n)
			return d[0];
		else if (p == 1.0)
			return d[(n - 1) * stride];
		else {
			int i = (int)floor(n * p);
			return d[(i - 1) * stride] + (n * p - i) * (d[i * stride] - d[(i - 1) * stride]);
		}
	case nsl_stats_quantile_type5:
		if (p < 0.5 / n)
			return d[0];
		else if (p >= (n - 0.5) / n)
			return d[(n - 1) * stride];
		else {
			int i = (int)floor(n * p + 0.5);
			return d[(i - 1) * stride] + (n * p + 0.5 - i) * (d[i * stride] - d[(i - 1) * stride]);
		}
	case nsl_stats_quantile_type6:
		if (p < 1. / (n + 1.))
			return d[0];
		else if (p > n / (n + 1.))
			return d[(n - 1) * stride];
		else {
			int i = (int)floor((n + 1) * p);
			return d[(i - 1) * stride] + ((n + 1) * p - i) * (d[i * stride] - d[(i - 1) * stride]);
		}
	case nsl_stats_quantile_type7: // = gsl_stats_quantile_from_sorted_data(d, stride, n, p);
		if (p == 1.0 || n == 1)
			return d[(n - 1) * stride];
		else {
			int i = (int)floor((n - 1) * p + 1);
			return d[(i - 1) * stride] + ((n - 1) * p + 1 - i) * (d[i * stride] - d[(i - 1) * stride]);
		}
	case nsl_stats_quantile_type8:
		if (p < 2. / 3. / (n + 1. / 3.))
			return d[0];
		else if (p >= (n - 1. / 3.) / (n + 1. / 3.))
			return d[(n - 1) * stride];
		else {
			int i = (int)floor((n + 1. / 3.) * p + 1. / 3.);
			return d[(i - 1) * stride] + ((n + 1. / 3.) * p + 1. / 3. - i) * (d[i * stride] - d[(i - 1) * stride]);
		}
	case nsl_stats_quantile_type9:
		if (p < 5. / 8. / (n + 1. / 4.))
			return d[0];
		else if (p >= (n - 3. / 8.) / (n + 1. / 4.))
			return d[(n - 1) * stride];
		else {
			int i = (int)floor((n + 1. / 4.) * p + 3. / 8.);
			return d[(i - 1) * stride] + ((n + 1. / 4.) * p + 3. / 8. - i) * (d[i * stride] - d[(i - 1) * stride]);
		}
	}

	return 0;
}

double nsl_stats_quantile_from_sorted_data(const double sorted_data[], size_t stride, size_t n, double p) {
	return nsl_stats_quantile_sorted(sorted_data, stride, n, p, nsl_stats_quantile_type7);
}

/* R^2 and adj. R^2 */
double nsl_stats_rsquare(double sse, double sst) {
	return 1. - sse / sst;
}
double nsl_stats_rsquareAdj(double rsquare, size_t np, size_t dof, int version) {
	size_t n = np + dof;
	// see https://stats.stackexchange.com/questions/48703/what-is-the-adjusted-r-squared-formula-in-lm-in-r-and-how-should-it-be-interpret
	switch (version) {
	case 2:
		return 1. - (1. - rsquare) * (n - 1.) / dof;
	default:
		return 1. - (1. - rsquare) * (n - 1.) / (dof - 1.);
	}
}

/* t distribution */
double nsl_stats_tdist_t(double parameter, double error) {
	if (error > 0)
		return parameter / error;
	else
		return DBL_MAX;
}

double nsl_stats_tdist_p(double t, double dof) {
	double p = 2. * gsl_cdf_tdist_Q(fabs(t), dof);
	if (p < 1.e-9)
		p = 0;
	return p;
}
double nsl_stats_tdist_z(double alpha, double dof) {
	return gsl_cdf_tdist_Pinv(1. - alpha / 2., dof);
}
double nsl_stats_tdist_margin(double alpha, double dof, double error) {
	if (error == 0.)
		return 0.;

	return nsl_stats_tdist_z(alpha, dof) * error;
}

/* chi^2 distribution */
double nsl_stats_chisq_p(double t, double dof) {
	double p = gsl_cdf_chisq_Q(t, dof);
	if (p < 1.e-9)
		p = 0;
	return p;
}
double nsl_stats_chisq_low(double alpha, double n) {
	return 0.5 * gsl_cdf_chisq_Pinv(alpha / 2., 2 * n);
}
double nsl_stats_chisq_high(double alpha, double n) {
	return 0.5 * gsl_cdf_chisq_Pinv(1. - alpha / 2., 2 * n + 2);
}

/* F distribution */
double nsl_stats_fdist_F(double rsquare, size_t np, size_t dof) {
	// (sst/sse - 1.) * dof/(p-1) = dof/(p-1)/(1./R^2 - 1)
	if (np < 2)
		np = 2;
	return dof / (np - 1.) / (1. / rsquare - 1.);
}
double nsl_stats_fdist_p(double F, size_t np, double dof) {
	if (isnan(F))
		return 0.;

	double p = gsl_cdf_fdist_Q(F, (double)np, dof);
	if (p < 1.e-9)
		p = 0;
	return p;
}

/* log-likelihood */
double nsl_stats_logLik(double sse, size_t n) {
	double ll = -(double)n / 2. * log(sse / n) - n / 2. * log(2 * M_PI) - n / 2.;
	return ll;
}

/* Akaike information criterion */
double nsl_stats_aic(double sse, size_t n, size_t np, int version) {
	switch (version) {
	case 2:
		return n * log(sse / n) + 2. * np; // reduced formula
	case 3: {
		double aic = n * log(sse / n) + 2. * np;
		if (n < 40 * np) // bias correction
			aic += 2. * np * (np + 1.) / (n - np - 1.);
		return aic;
	}
	default:
		return n * log(sse / n) + 2. * (np + 1) + n * log(2. * M_PI) + n; // complete formula used in R
	}
}
double nsl_stats_aicc(double sse, size_t n, size_t np, int version) {
	double aic;
	switch (version) {
	case 2:
		aic = n * log(sse / n) + 2. * np;
		break;
	default:
		aic = n * log(sse / n) + 2. * (np + 1) + n * log(2. * M_PI) + n;
	}
	return aic + 2. * np * (np + 1.) / (n - np - 1.);
}

/* Bayasian information criterion */
double nsl_stats_bic(double sse, size_t n, size_t np, int version) {
	switch (version) {
	case 2:
		return n * log(sse / n) + np * log((double)n); // reduced formula
	default:
		return n * log(sse / n) + (np + 1) * log((double)n) + n + n * log(2. * M_PI); // complete formula used in R
	}
}

/* Mann-Whitney U test */
int compare_values(const void* a, const void* b) {
	double diff = ((double*)a)[0] - ((double*)b)[0]; // Compare based on the first element (value)
	return (diff > 0) - (diff < 0); // returns 1, 0, or -1
}

double nsl_stats_mannwhitney_u(const double sample1[], size_t n1, const double sample2[], size_t n2) {
	size_t n = n1 + n2;
	Rank combined[n];

	// Step 1: Combine samples and tag them
	for (size_t i = 0; i < n1; i++) {
		combined[i].value = sample1[i];
		combined[i].group = 0; // Group 0 for sample1
	}
	for (size_t i = 0; i < n2; i++) {
		combined[n1 + i].value = sample2[i];
		combined[n1 + i].group = 1; // Group 1 for sample2
	}

	// Step 2: Sort the combined array by value
	for (size_t i = 0; i < n - 1; i++) {
		for (size_t j = i + 1; j < n; j++) {
			if (combined[i].value > combined[j].value) {
				Rank temp = combined[i];
				combined[i] = combined[j];
				combined[j] = temp;
			}
		}
	}

	// Step 3: Assign ranks with tie handling
	double ranks[n];
	size_t i = 0;
	while (i < n) {
		size_t j = i;
		while (j < n && combined[j].value == combined[i].value)
			j++;

		double rank = (i + j + 1) / 2.0; // Average rank for tied values
		for (size_t k = i; k < j; k++)
			ranks[k] = rank;

		i = j;
	}

	// Step 4: Calculate rank sums for each sample
	double rank_sum1 = 0.0;
	for (size_t i = 0; i < n; i++) {
		if (combined[i].group == 0) // Sum ranks for sample1
			rank_sum1 += ranks[i];
	}

	// Step 5: Compute U statistic for sample1
	double U1 = rank_sum1 - (n1 * (n1 + 1)) / 2.0;
	double U2 = n1 * n2 - U1; // U2 is derived from U1

	return U1 < U2 ? U1 : U2; // Return the smaller U statistic for a two-sided test
}

double nsl_stats_mannwhitney_p(double U, size_t n1, size_t n2) {
	// Calculate mean and variance of U under the null hypothesis
	double mean_U = (n1 * n2) / 2.0;
	double variance_U = (n1 * n2 * (n1 + n2 + 1)) / 12.0;

	// Z-score calculation for large sample sizes
	double z = (U - mean_U) / sqrt(variance_U);

	// Convert z-score to p-value
	double p_value = 2.0 * (1.0 - 0.5 * (1.0 + erf(z / sqrt(2.0)))); // Two-tailed p-value

	return p_value;
}

// Calculate the F-statistic for one-way ANOVA
double nsl_stats_anova_oneway_f(double** groups, size_t* sizes, size_t n_groups) {
	// Step 1: Calculate the grand mean
	double grand_total = 0.0;
	size_t total_samples = 0;
	for (size_t i = 0; i < n_groups; i++) {
		for (size_t j = 0; j < sizes[i]; j++)
			grand_total += groups[i][j];

		total_samples += sizes[i];
	}
	double grand_mean = grand_total / total_samples;

	// Step 2: Calculate the between-group variance (SSB)
	double ssb = 0.0;
	for (size_t i = 0; i < n_groups; i++) {
		double group_mean = gsl_stats_mean(groups[i], 1, sizes[i]);
		ssb += sizes[i] * (group_mean - grand_mean) * (group_mean - grand_mean);
	}
	double ms_between = ssb / (n_groups - 1);

	// Step 3: Calculate the within-group variance (SSW)
	double ssw = 0.0;
	for (size_t i = 0; i < n_groups; i++) {
		ssw += gsl_stats_variance(groups[i], 1, sizes[i]) * (sizes[i] - 1);
	}
	double ms_within = ssw / (total_samples - n_groups);

	// Step 4: Calculate and return the F-statistic
	return ms_between / ms_within;
}

// Calculate the p-value for one-way ANOVA
double nsl_stats_anova_oneway_p(double** groups, size_t* sizes, size_t n_groups) {
	// Calculate F-statistic
	double F = nsl_stats_anova_oneway_f(groups, sizes, n_groups);
	// Step 5: Calculate and return the p-value using the F-distribution
	size_t df_between = n_groups - 1;
	size_t df_within = 0;
	for (size_t i = 0; i < n_groups; i++)
		df_within += sizes[i];

	df_within -= n_groups;

	return nsl_stats_fdist_p(F, df_between, df_within);
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

// Calculate the Kruskal-Wallis H statistic
double nsl_stats_kruskal_wallis_h(double** groups, size_t* sizes, size_t n_groups) {
	size_t total_samples = 0;
	for (size_t i = 0; i < n_groups; i++)
		total_samples += sizes[i];

	Rank* ranks = (Rank*)malloc(total_samples * sizeof(Rank));
	size_t idx = 0;

	// Step 1: Collect all data
	for (size_t i = 0; i < n_groups; i++) {
		for (size_t j = 0; j < sizes[i]; j++)
			ranks[idx++] = (Rank){groups[i][j], i};
	}

	// Step 2: Sort by value
	qsort(ranks, total_samples, sizeof(Rank), compare_rank);

	// Step 3: Assign ranks and handle ties
	double* rank_sum = (double*)calloc(n_groups, sizeof(double));
	for (size_t i = 0; i < total_samples; i++) {
		size_t start = i;
		double rank_avg = i + 1;

		// Find range of tied values
		while (i + 1 < total_samples && ranks[i].value == ranks[i + 1].value) {
			rank_avg += i + 2;
			i++;
		}
		rank_avg /= (i - start + 1);

		// Assign the average rank to each tied value
		for (size_t j = start; j <= i; j++)
			rank_sum[ranks[j].group] += rank_avg;
	}

	// Step 4: Calculate H statistic
	double H = 0.0;
	for (size_t i = 0; i < n_groups; i++)
		H += (rank_sum[i] * rank_sum[i]) / sizes[i];

	H = (12.0 / (total_samples * (total_samples + 1))) * H - 3 * (total_samples + 1);

	// Cleanup
	free(ranks);
	free(rank_sum);

	return H;
}

// Calculate the p-value for Kruskal-Wallis test
double nsl_stats_kruskal_wallis_p(double** groups, size_t* sizes, size_t n_groups) {
	double H = nsl_stats_kruskal_wallis_h(groups, sizes, n_groups);
	return 1.0 - gsl_cdf_chisq_P(H, n_groups - 1);
}

// Compare function for sorting indices by time
int compare_time(const void* a, const void* b, void* times) {
	double* time = (double*)times;
	size_t idx_a = *(size_t*)a;
	size_t idx_b = *(size_t*)b;

	if (time[idx_a] < time[idx_b])
		return -1;
	if (time[idx_a] > time[idx_b])
		return 1;
	return 0;
}

// Calculate the Log-Rank test statistic
double nsl_stats_log_rank_test_statistic(const double* time,
										 const int* status,
										 const size_t* group1_indices,
										 size_t size1,
										 const size_t* group2_indices,
										 size_t size2) {
	size_t total_size = size1 + size2;
	size_t* combined_indices = (size_t*)malloc(total_size * sizeof(size_t));
	for (size_t i = 0; i < size1; i++)
		combined_indices[i] = group1_indices[i];
	for (size_t i = 0; i < size2; i++)
		combined_indices[size1 + i] = group2_indices[i];

	// Sort combined indices by time
	qsort_r(combined_indices, total_size, sizeof(size_t), compare_time, (void*)time);

	size_t n_risk_1 = size1, n_risk_2 = size2, n_risk_total = total_size;
	double observed_events_1 = 0.0, expected_events_1 = 0.0, variance = 0.0;

	for (size_t i = 0; i < total_size; i++) {
		size_t idx = combined_indices[i];
		if (status[idx] == 1) { // Only consider event occurrences
			double event_rate_1 = (double)n_risk_1 / n_risk_total;
			double event_rate_2 = (double)n_risk_2 / n_risk_total;

			if (idx < size1) // Belongs to group 1
				observed_events_1++;

			expected_events_1 += event_rate_1;
			variance += event_rate_1 * event_rate_2 * (n_risk_total - 1.0) / n_risk_total;

			// Decrease the risk sets
			if (idx < size1)
				n_risk_1--;
			else
				n_risk_2--;

			n_risk_total--;
		}
	}

	// Calculate test statistic
	double observed_diff = observed_events_1 - expected_events_1;
	free(combined_indices);
	return (observed_diff * observed_diff) / variance;
}

// Calculate the p-value for Log-Rank test
double
nsl_stats_log_rank_test_p(const double* time, const int* status, const size_t* group1_indices, size_t size1, const size_t* group2_indices, size_t size2) {
	double chi_square_stat = nsl_stats_log_rank_test_statistic(time, status, group1_indices, size1, group2_indices, size2);
	return 1.0 - gsl_cdf_chisq_P(chi_square_stat, 1);
}
