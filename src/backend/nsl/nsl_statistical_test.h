/*
	File                 : nsl_stats.h
	Project              : LabPlot
	Description          : NSL statistical test functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-FileCopyrightText: 2025-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_STATISTICAL_TEST_H
#define NSL_STATISTICAL_TEST_H

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS /* empty */
#define __END_DECLS /* empty */
#endif
__BEGIN_DECLS

#include <stdlib.h>

typedef enum { nsl_stats_tail_type_two, nsl_stats_tail_type_negative, nsl_stats_tail_type_positive } nsl_stats_tail_type;

struct one_sample_t_test_result {
	double mean;
	double variance;
	double mean_standard_error;
	double t;
	double p;
	int df;
	double mean_difference;
};
struct one_sample_t_test_result nsl_stats_one_sample_t(const double sample[], size_t n, double hypothesized_mean, nsl_stats_tail_type tail);

struct independent_t_test_result {
	double mean1;
	double variance1;
	double mean_standard_error1;
	double mean2;
	double variance2;
	double mean_standard_error2;
	double t;
	double p;
	int df;
	double pooled_variance;
	double mean_difference_standard_error;
	double mean_difference;
};
struct independent_t_test_result nsl_stats_independent_t(const double sample1[], size_t n1, const double sample2[], size_t n2, nsl_stats_tail_type tail);

struct welch_t_test_result {
	double mean1;
	double variance1;
	double mean_standard_error1;
	double mean2;
	double variance2;
	double mean_standard_error2;
	double t;
	double p;
	int df;
	double mean_difference_standard_error;
	double mean_difference;
};
struct welch_t_test_result nsl_stats_welch_t(const double sample1[], size_t n1, const double sample2[], size_t n2, nsl_stats_tail_type tail);

struct anova_oneway_test_result {
	double df_between_groups;
	double df_within_groups;
	double ss_between_groups;
	double ss_within_groups;
	double ms_between_groups;
	double ms_within_groups;
	double F;
	double p;
};
struct anova_oneway_test_result nsl_stats_anova_oneway_f(const double** groups, const size_t* sizes, size_t n_groups);

struct anova_oneway_repeated_test_result {
	double df_treatment;
	double df_residuals;
	double ss_treatment;
	double ss_residuals;
	double ss_within_subjects;
	double ms_treatment;
	double ms_residuals;
	double F;
	double p;
};
struct anova_oneway_repeated_test_result nsl_stats_anova_oneway_repeated_f(const double** groups, size_t n_samples, size_t n_groups);

struct mannwhitney_test_result {
	int rank_sum1;
	int rank_sum2;
	double mean_rank1;
	double mean_rank2;
	double U;
	double z;
	double p;
};
struct mannwhitney_test_result nsl_stats_mannwhitney_u(const double sample1[], size_t n1, const double sample2[], size_t n2, nsl_stats_tail_type tail);

struct kruskal_wallis_test_result {
	double p;
	double H;
	int df;
};
struct kruskal_wallis_test_result nsl_stats_kruskal_wallis_h(const double** groups, const size_t* sizes, size_t n_groups);

struct wilcoxon_test_result {
	int positive_rank_sum;
	double positive_rank_mean;
	int positive_rank_count;
	int negative_rank_sum;
	double negative_rank_mean;
	int negative_rank_count;
	int tie_count;
	double W;
	double z;
	double p;
};
struct wilcoxon_test_result nsl_stats_wilcoxon_w(const double sample1[], const double sample2[], size_t n, nsl_stats_tail_type tail);

struct friedman_test_result {
	double p;
	double Q;
	int df;
};
struct friedman_test_result nsl_stats_friedman_q(const double** groups, size_t n_samples, size_t n_groups);

struct chisq_ind_test_result {
	double p;
	double x2;
	int df;
	int total_observed_frequencies;
};
struct chisq_ind_test_result nsl_stats_chisq_ind_x2(const long long** table, size_t row, size_t column);

struct chisq_gof_test_result {
	double p;
	double x2;
	int df;
};
struct chisq_gof_test_result nsl_stats_chisq_gof_x2(const long long* observed, const double* expected, size_t n, size_t params_estimated);

struct log_rank_test_result {
	double H;
	int df;
	double p;
	int event_count1;
	int event_count2;
	int censored_count1;
	int censored_count2;
	int total_count1;
	int total_count2;
};
struct log_rank_test_result nsl_stats_log_rank_h(const double* time, const int* status, const size_t* g1_ind, size_t size1, const size_t* g2_ind, size_t size2);

struct mann_kendall_test_result {
	double S;
	double tau;
	double z;
	double p;
	int n;
	double slope;
};
struct mann_kendall_test_result nsl_stats_mann_kendall(const double sample[], size_t n, nsl_stats_tail_type tail);

struct wald_wolfowitz_runs_test_result {
	double z;
	double p;
	int runs;
	int n;
};
struct wald_wolfowitz_runs_test_result nsl_stats_wald_wolfowitz_runs(const double sample[], size_t n, nsl_stats_tail_type tail);

struct ramirez_runger_test_result {
	double z;
	double p;
	int runs;
	double expected_runs;
	int n_eff;
};
struct ramirez_runger_test_result nsl_stats_ramirez_runger(const double sample[], size_t n, nsl_stats_tail_type tail);

__END_DECLS

#endif // NSL_STATISTICAL_TEST_H
