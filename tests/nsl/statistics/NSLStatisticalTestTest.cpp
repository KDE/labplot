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

	double p = NAN;
	double U = nsl_stats_mannwhitney_u(sample1, n1, sample2, n2, nsl_stats_tail_type_two, &p);
	printf("Mann-Whitney U: %f\n", U);
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

	double p = NAN;
	double F = nsl_stats_anova_oneway_f(groups, sizes, n_groups, &p);
	printf("Computed F-statistic: %f", F);

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

	double p = NAN;
	double H = nsl_stats_kruskal_wallis_h(groups, sizes, n_groups, &p);
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

	double p1 = NAN;
	double H1 = nsl_stats_log_rank_h(time1_combined, status1_combined, group1_indices1, size1_1, group2_indices1, size2_1, &p1);

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

	double p2 = NAN;
	double H2 = nsl_stats_log_rank_h(time2_combined, status2_combined, group1_indices2, size1_2, group2_indices2, size2_2, &p2);

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

	double p_value = NAN;
	double t_stat = nsl_stats_independent_t(sample1, n1, sample2, n2, nsl_stats_tail_type_two, &p_value);

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
	double p_value = NAN;
	double t_stat = nsl_stats_one_sample_t(sample, n, hypothesized_mean, nsl_stats_tail_type_two, &p_value);

	double expected_t = 0.8165;
	double expected_p = 0.4601;

	const double epsilon = 1e-4;
	printf("Computed t-statistic: %f\n", t_stat);
	printf("Computed p-value: %f\n", p_value);
	QVERIFY(std::abs(t_stat - expected_t) < epsilon);
	QVERIFY(std::abs(p_value - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testAnovaOneWayRepeated() {
	const double group1[] = {7.0, 2.0, 5.0, 6.0, 4.0, 7.0, 4.0, 5.0};
	const double group2[] = {9.0, 3.0, 7.0, 6.0, 7.0, 8.0, 6.0, 3.0};
	const double group3[] = {8.0, 3.0, 6.0, 4.0, 5.0, 4.0, 4.0, 7.0};
	const size_t n_samples = 8;
	const size_t n_groups = 3;

	double* groups[] = {const_cast<double*>(group1), const_cast<double*>(group2), const_cast<double*>(group3)};

	double p = NAN;
	double F = nsl_stats_anova_oneway_repeated_f(groups, n_samples, n_groups, &p);
	printf("Computed F-statistic: %f", F);

	printf("Computed p-value: %f\n", p);

	double expected_F = 1.68647;
	double expected_p = 0.22069;

	const double epsilon = 1e-3;

	QVERIFY(std::abs(F - expected_F) < epsilon);
	QVERIFY(std::abs(p - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testWelchT() {
	const double sample1[] = {14.0, 15.0, 15.0, 15.0, 16.0, 18.0, 22.0, 23.0, 24.0, 25.0, 25.0};
	const double sample2[] = {10.0, 12.0, 14.0, 15.0, 18.0, 22.0, 24.0, 27.0, 31.0, 33.0, 34.0, 34.0, 34.0};

	size_t n1 = 11;
	size_t n2 = 13;

	double p_value = NAN;
	double t_stat = nsl_stats_welch_t(sample1, n1, sample2, n2, nsl_stats_tail_type_two, &p_value);

	double expected_t = -1.5379;
	double expected_p = 0.1414;

	const double epsilon = 1e-4;

	printf("Computed t-statistic: %f\n", t_stat);
	printf("Computed p-value: %f\n", p_value);

	QVERIFY(std::abs(t_stat - expected_t) < epsilon);
	QVERIFY(std::abs(p_value - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testWilcoxon() {
	const double sample1[] = {1.1,	2.3,  3.5,	4.2,  5.8,	6.1,  7.9,	8.4,  9.6,	10.3, 11.7, 12.5, 13.9,
							  14.2, 15.6, 16.8, 17.1, 18.5, 19.3, 20.7, 21.2, 22.8, 23.4, 24.9, 25.1};
	const double sample2[] = {1.5,	2.1,  3.8,	4.0,  5.9,	6.7,  7.2,	8.8,  9.1,	10.5, 11.9, 12.3, 13.5,
							  14.8, 15.2, 16.9, 17.5, 18.0, 19.7, 20.1, 21.5, 22.3, 23.0, 24.5, 25.8};

	size_t n = 25;

	const double epsilon = 1e-3;

	double p_value = NAN;
	double w_stat = nsl_stats_wilcoxon_w(sample1, sample2, n, nsl_stats_tail_type_two, &p_value);

	double expected_w = 152.5;
	double expected_p = 0.79808;

	printf("Computed W-statistic: %f\n", w_stat);
	printf("Computed p-value: %f\n", p_value);

	QVERIFY(std::abs(w_stat - expected_w) < epsilon);
	QVERIFY(std::abs(p_value - expected_p) < epsilon);

	p_value = NAN;
	w_stat = nsl_stats_wilcoxon_w(sample1, sample2, n, nsl_stats_tail_type_negative, &p_value);

	expected_w = 172.5;
	expected_p = 0.61132;

	printf("Computed W-statistic: %f\n", w_stat);
	printf("Computed p-value: %f\n", p_value);

	QVERIFY(std::abs(w_stat - expected_w) < epsilon);
	QVERIFY(std::abs(p_value - expected_p) < epsilon);

	p_value = NAN;
	w_stat = nsl_stats_wilcoxon_w(sample1, sample2, n, nsl_stats_tail_type_positive, &p_value);

	expected_w = 172.5;
	expected_p = 0.39903;

	printf("Computed W-statistic: %f\n", w_stat);
	printf("Computed p-value: %f\n", p_value);

	QVERIFY(std::abs(w_stat - expected_w) < epsilon);
	QVERIFY(std::abs(p_value - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testFriedman() {
	const double group1[] = {34.0, 33.0, 41.0, 39.0, 44.0, 37.0, 39.0};
	const double group2[] = {45.0, 36.0, 35.0, 43.0, 42.0, 42.0, 46.0};
	const double group3[] = {36.0, 31.0, 44.0, 42.0, 41.0, 45.0, 40.0};
	const size_t n_samples = 7;
	const size_t n_groups = 3;

	double* groups[] = {const_cast<double*>(group1), const_cast<double*>(group2), const_cast<double*>(group3)};

	double p = NAN;
	double Q = nsl_stats_friedman_q(groups, n_samples, n_groups, &p);
	printf("Computed Q-statistic: %f", Q);

	printf("Computed p-value: %f\n", p);

	double expected_Q = 2.5714;
	double expected_p = 0.276;

	const double epsilon = 1e-3;

	QVERIFY(std::abs(Q - expected_Q) < epsilon);
	QVERIFY(std::abs(p - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testChisqIndependence() {
	const double col1[] = {5.0, 5.0};
	const double col2[] = {7.0, 5.0};
	const size_t n_rows = 2;
	const size_t n_cols = 2;

	double* table[] = {const_cast<double*>(col1), const_cast<double*>(col2)};

	double p = NAN;
	double x2 = nsl_stats_chisq_ind_x2(table, n_rows, n_cols, &p);
	printf("Computed x2-statistic: %f", x2);

	printf("Computed p-value: %f\n", p);

	double expected_x2 = 0.1528;
	double expected_p = 0.696;

	const double epsilon = 1e-3;

	QVERIFY(std::abs(x2 - expected_x2) < epsilon);
	QVERIFY(std::abs(p - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testChisqGoodnessOfFit() {
	const int observed[] = {50, 110, 40};
	const int expected[] = {60, 100, 40};
	const size_t n = 3;

	double p = NAN;
	double x2 = nsl_stats_chisq_gof_x2(const_cast<int*>(observed), const_cast<int*>(expected), n, 0, &p);
	printf("Computed x2-statistic: %f", x2);

	printf("Computed p-value: %f\n", p);

	double expected_x2 = 2.667;
	double expected_p = 0.2636;

	const double epsilon = 1e-3;

	QVERIFY(std::abs(x2 - expected_x2) < epsilon);
	QVERIFY(std::abs(p - expected_p) < epsilon);
}

QTEST_MAIN(NSLStatisticalTestTest)
