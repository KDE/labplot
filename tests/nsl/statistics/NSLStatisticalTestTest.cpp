/*
	File                 : NSLStatisticalTestTest.cpp
	Project              : LabPlot
	Description          : NSL Tests for statistical functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "NSLStatisticalTestTest.h"

extern "C" {
#include "backend/nsl/nsl_statistical_test.h"
}

void NSLStatisticalTestTest::testMannWhitney() {
	const double sample1[] = {1.0, 2.0, 3.0};
	const double sample2[] = {4.0, 5.0, 6.0};
	const size_t n1 = sizeof(sample1) / sizeof(sample1[0]);
	const size_t n2 = sizeof(sample2) / sizeof(sample2[0]);

	double U = nsl_stats_mannwhitney_u(sample1, n1, sample2, n2);
	printf("Mann-Whitney U: %f\n", U);
	double p = nsl_stats_mannwhitney_p(U, n1, n2);
	printf("Mann-Whitney p-value: %f\n", p);
	double expected_U = 0.0;
	double expected_p = 0.0808556;
	const double epsilon = 1e-4;
	QVERIFY(std::abs(U - expected_U) < epsilon);
	QVERIFY(std::abs(p - expected_p) < epsilon);
}
void NSLStatisticalTestTest::testAnovaOneWay() {
	const double group1[] = {1.0, 2.0, 3.0};
	const double group2[] = {2.0, 3.0, 4.0};
	const double group3[] = {5.0, 6.0, 7.0};
	size_t sizes[] = {3, 3, 3};
	const size_t n_groups = 3;

	double* groups[] = {const_cast<double*>(group1), const_cast<double*>(group2), const_cast<double*>(group3)};

	double F = nsl_stats_anova_oneway_f(groups, sizes, n_groups);
	printf("Computed F-statistic: %f", F);

	double p = nsl_stats_anova_oneway_p(groups, sizes, n_groups);
	printf("Computed p-value: %f\n", p);

	double expected_F = 13.0;
	double expected_p = 0.0065;

	const double epsilon = 1e-4;

	QVERIFY(std::abs(F - expected_F) < epsilon);
	QVERIFY(std::abs(p - expected_p) < epsilon);
}
void NSLStatisticalTestTest::testKruskalWallis() {
	const double group1[] = {1.0, 2.0, 3.0};
	const double group2[] = {2.0, 3.0, 4.0};
	const double group3[] = {5.0, 6.0, 7.0};
	size_t sizes[] = {3, 3, 3};
	const size_t n_groups = 3;

	double* groups[] = {const_cast<double*>(group1), const_cast<double*>(group2), const_cast<double*>(group3)};

	double H = nsl_stats_kruskal_wallis_h(groups, sizes, n_groups);
	double p = nsl_stats_kruskal_wallis_p(groups, sizes, n_groups);
	printf("Computed h-statistic: %f\n", H);
	printf("Computed p-value: %f\n", p);

	double expected_H = 6.0564;
	double expected_p = 0.0484;

	const double epsilon = 1e-4;

	QVERIFY(std::abs(H - expected_H) < epsilon);
	QVERIFY(std::abs(p - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testLogRankTest() {
	// ----------------------------------------------------------------------------------
	// Test Case 1: No Censored Data
	// ----------------------------------------------------------------------------------
	const double time1_group1[] = {5, 6, 6, 2, 4, 4};
	const int status1_group1[] = {1, 1, 1, 1, 1, 1}; // 1 = event occurred

	const double time1_group2[] = {1, 3, 3, 8, 9, 9};
	const int status1_group2[] = {1, 1, 1, 1, 1, 1}; // 1 = event occurred

	size_t group1_indices1[] = {0, 1, 2, 3, 4, 5};
	size_t size1_1 = sizeof(group1_indices1) / sizeof(group1_indices1[0]);

	size_t group2_indices1[] = {6, 7, 8, 9, 10, 11};
	size_t size2_1 = sizeof(group2_indices1) / sizeof(group2_indices1[0]);

	double time1_combined[12];
	int status1_combined[12];
	for (size_t i = 0; i < size1_1; i++) {
		time1_combined[i] = time1_group1[i];
		status1_combined[i] = status1_group1[i];
	}
	for (size_t i = 0; i < size2_1; i++) {
		time1_combined[size1_1 + i] = time1_group2[i];
		status1_combined[size1_1 + i] = status1_group2[i];
	}

	double H1 = nsl_stats_log_rank_test_statistic(time1_combined, status1_combined, group1_indices1, size1_1, group2_indices1, size2_1);

	double p1 = nsl_stats_log_rank_test_p(time1_combined, status1_combined, group1_indices1, size1_1, group2_indices1, size2_1);
	printf("Computed H-value: %f\n", H1);
	printf("Computed p-value: %f\n", p1);

	double expected_H1 = 1.009510;
	double expected_p1 = 0.31502028;

	double epsilon = 1e-4;
	QVERIFY(std::abs(H1 - expected_H1) < epsilon);
	QVERIFY(std::abs(p1 - expected_p1) < epsilon);

	// ----------------------------------------------------------------------------------
	// Test Case 2: Censored Data
	// ----------------------------------------------------------------------------------

	const double time2_group1[] = {5, 6, 6, 2, 4, 4};
	const int status2_group1[] = {1, 1, 0, 1, 1, 0}; // 1 = event occurred, 0 = censored

	const double time2_group2[] = {1, 3, 3, 8, 9, 9};
	const int status2_group2[] = {1, 0, 1, 1, 0, 1}; // 1 = event occurred, 0 = censored

	size_t group1_indices2[] = {0, 1, 2, 3, 4, 5};
	size_t size1_2 = sizeof(group1_indices2) / sizeof(group1_indices2[0]);

	size_t group2_indices2[] = {6, 7, 8, 9, 10, 11};
	size_t size2_2 = sizeof(group2_indices2) / sizeof(group2_indices2[0]);

	double time2_combined[12];
	int status2_combined[12];
	for (size_t i = 0; i < size1_2; i++) {
		time2_combined[i] = time2_group1[i];
		status2_combined[i] = status2_group1[i];
	}
	for (size_t i = 0; i < size2_2; i++) {
		time2_combined[size1_2 + i] = time2_group2[i];
		status2_combined[size1_2 + i] = status2_group2[i];
	}

	double H2 = nsl_stats_log_rank_test_statistic(time2_combined, status2_combined, group1_indices2, size1_2, group2_indices2, size2_2);

	double p2 = nsl_stats_log_rank_test_p(time2_combined, status2_combined, group1_indices2, size1_2, group2_indices2, size2_2);
	double expected_H2 = 0.586871;
	double expected_p2 = 0.44362;
	printf("Computed H-value: %f\n", H2);
	printf("Computed p-value: %f\n", p2);

	QVERIFY(std::abs(H2 - expected_H2) < epsilon);
	QVERIFY(std::abs(p2 - expected_p2) < epsilon);
}
void NSLStatisticalTestTest::testIndependentT() {
	const double sample1[] = {12.5, 13.3, 14.2, 12.7, 13.9};
	const double sample2[] = {15.1, 14.8, 15.5, 15.2, 14.9};

	size_t n1 = 5;
	size_t n2 = 5;

	double t_stat = nsl_stats_independent_t(sample1, n1, sample2, n2);
	double p_value = nsl_stats_independent_t_p(sample1, n1, sample2, n2, nsl_stats_tail_type_two);

	double expected_t = -5.0671;
	double expected_p = 0.0010;

	const double epsilon = 1e-4;

	printf("Computed t-statistic: %f\n", t_stat);
	printf("Computed p-value: %f\n", p_value);

	QVERIFY(std::abs(t_stat - expected_t) < epsilon);
	QVERIFY(std::abs(p_value - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testOneSampleT() {
	const double sample[] = {15.2, 14.8, 15.5, 14.9, 15.1};
	size_t n = 5;
	double hypothesized_mean = 15.0;
	double t_stat = nsl_stats_one_sample_t(sample, n, hypothesized_mean);
	double p_value = nsl_stats_one_sample_t_p(sample, n, hypothesized_mean, nsl_stats_tail_type_two);

	double expected_t = 0.8165;
	double expected_p = 0.4601;

	const double epsilon = 1e-4;
	printf("Computed t-statistic: %f\n", t_stat);
	printf("Computed p-value: %f\n", p_value);
	QVERIFY(std::abs(t_stat - expected_t) < epsilon);
	QVERIFY(std::abs(p_value - expected_p) < epsilon);
}

QTEST_MAIN(NSLStatisticalTestTest)
