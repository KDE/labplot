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

double nsl_stats_one_sample_t(const double sample[], size_t n, double hypothesized_mean, nsl_stats_tail_type tail, double* p_out);

double nsl_stats_independent_t(const double sample1[], size_t n1, const double sample2[], size_t n2, nsl_stats_tail_type tail, double* p_out);

double nsl_stats_welch_t(const double sample1[], size_t n1, const double sample2[], size_t n2, nsl_stats_tail_type tail, double* p_out);

double nsl_stats_anova_oneway_f(double** groups, size_t* sizes, size_t n_groups, double* p_out);

double nsl_stats_anova_oneway_repeated_f(double** groups, size_t n_samples, size_t n_groups, double* p_out);

double nsl_stats_mannwhitney_u(const double sample1[], size_t n1, const double sample2[], size_t n2, nsl_stats_tail_type tail, double* p_out);

double nsl_stats_kruskal_wallis_h(double** groups, size_t* sizes, size_t n_groups, double* p_out);

double nsl_stats_wilcoxon_w(const double sample1[], const double sample2[], size_t n, nsl_stats_tail_type tail, double* p_out);

double nsl_stats_friedman_q(double** groups, size_t n_samples, size_t n_groups, double* p_out);

double nsl_stats_chisq_ind_x2(double** table, size_t row, size_t column, double* p_out);

double nsl_stats_chisq_gof_x2(int* observed, int* expected, size_t n, size_t params_estimated, double* p_out);

double nsl_stats_log_rank_h(const double* time, const int* status, const size_t* g1_ind, size_t size1, const size_t* g2_ind, size_t size2, double* p_out);

__END_DECLS

#endif // NSL_STATISTICAL_TEST_H
