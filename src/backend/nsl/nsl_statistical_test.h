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

/* Mann-Whitney U test */
double nsl_stats_mannwhitney_u(const double sample1[], size_t n1, const double sample2[], size_t n2);
double nsl_stats_mannwhitney_p(double U, size_t n1, size_t n2);
/* One Way Annova test */
double nsl_stats_anova_oneway_f(double** groups, size_t* sizes, size_t n_groups);
double nsl_stats_anova_oneway_p(double** groups, size_t* sizes, size_t n_groups);

// Function prototypes for Kruskal-Wallis Test
int compare_rank(const void*, const void*);
double nsl_stats_kruskal_wallis_h(double** groups, size_t* sizes, size_t n_groups);
double nsl_stats_kruskal_wallis_p(double** groups, size_t* sizes, size_t n_groups);
// Structure used for ranking in the Kruskal-Wallis test
typedef struct {
	double value;
	size_t group;
} Rank;
// Observation structure for log rank test
typedef struct {
	double time;
	int status; // 1 = event occurred, 0 = censored
	size_t group; // 1 or 2
} Observation;

// Function prototypes for Log-Rank Test
double nsl_stats_log_rank_test_statistic(const double* time,
										 const int* status,
										 const size_t* group1_indices,
										 size_t size1,
										 const size_t* group2_indices,
										 size_t size2);
double nsl_stats_log_rank_test_p(const double* time, const int* status, const size_t* group1_indices, size_t size1, const size_t* group2_indices, size_t size2);

/* Independent Sample Student's t-test */
double nsl_stats_independent_t(const double sample1[], size_t n1, const double sample2[], size_t n2);
double nsl_stats_independent_t_p(const double sample1[], size_t n1, const double sample2[], size_t n2);

/* One Sample Student's t-test */
double nsl_stats_one_sample_t(const double sample[], size_t n, double hypothesized_mean);
typedef enum { nsl_stats_tail_type_two, nsl_stats_tail_type_negative, nsl_stats_tail_type_positive } nsl_stats_tail_type;
double nsl_stats_one_sample_t_p(const double sample[], size_t n, double hypothesized_mean, nsl_stats_tail_type tail);

__END_DECLS

#endif // NSL_STATISTICAL_TEST_H
