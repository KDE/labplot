/*
	File                 : NSLStatisticalTestTest.cpp
	Project              : LabPlot
	Description          : NSL Tests for statistical functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>

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

	mannwhitney_test_result result = nsl_stats_mannwhitney_u(sample1, n1, sample2, n2, nsl_stats_tail_type_two);
	printf("Mann-Whitney U: %f\n", result.U);
	printf("Mann-Whitney p-value: %f\n", result.p);
	double expected_U = 0.0;
	double expected_p = 0.0808556;
	const double epsilon = 1e-4;
	QVERIFY(std::abs(result.U - expected_U) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}
void NSLStatisticalTestTest::testAnovaOneWay() {
	const double group1[] = {1.0, 2.0, 3.0};
	const double group2[] = {2.0, 3.0, 4.0};
	const double group3[] = {5.0, 6.0, 7.0};
	size_t sizes[] = {3, 3, 3};
	const size_t n_groups = 3;

	const double* groups[] = {const_cast<double*>(group1), const_cast<double*>(group2), const_cast<double*>(group3)};

	anova_oneway_test_result result = nsl_stats_anova_oneway_f(groups, sizes, n_groups);
	printf("Computed F-statistic: %f", result.F);

	printf("Computed p-value: %f\n", result.p);

	double expected_F = 13.0;
	double expected_p = 0.0065;

	const double epsilon = 1e-4;

	QVERIFY(std::abs(result.F - expected_F) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}
void NSLStatisticalTestTest::testKruskalWallis() {
	const double group1[] = {1.0, 2.0, 3.0};
	const double group2[] = {2.0, 3.0, 4.0};
	const double group3[] = {5.0, 6.0, 7.0};
	size_t sizes[] = {3, 3, 3};
	const size_t n_groups = 3;

	const double* groups[] = {const_cast<double*>(group1), const_cast<double*>(group2), const_cast<double*>(group3)};

	kruskal_wallis_test_result result = nsl_stats_kruskal_wallis_h(groups, sizes, n_groups);
	printf("Computed h-statistic: %f\n", result.H);
	printf("Computed p-value: %f\n", result.p);

	double expected_H = 6.0564;
	double expected_p = 0.0484;

	const double epsilon = 1e-4;

	QVERIFY(std::abs(result.H - expected_H) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
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

	log_rank_test_result result = nsl_stats_log_rank_h(time1_combined, status1_combined, group1_indices1, size1_1, group2_indices1, size2_1);

	printf("Computed H-value: %f\n", result.H);
	printf("Computed p-value: %f\n", result.p);

	double expected_H1 = 1.009510;
	double expected_p1 = 0.31502028;

	double epsilon = 1e-4;
	QVERIFY(std::abs(result.H - expected_H1) < epsilon);
	QVERIFY(std::abs(result.p - expected_p1) < epsilon);

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

	result = nsl_stats_log_rank_h(time2_combined, status2_combined, group1_indices2, size1_2, group2_indices2, size2_2);

	double expected_H2 = 0.586871;
	double expected_p2 = 0.44362;
	printf("Computed H-value: %f\n", result.H);
	printf("Computed p-value: %f\n", result.p);

	QVERIFY(std::abs(result.H - expected_H2) < epsilon);
	QVERIFY(std::abs(result.p - expected_p2) < epsilon);
}
void NSLStatisticalTestTest::testIndependentT() {
	const double sample1[] = {12.5, 13.3, 14.2, 12.7, 13.9};
	const double sample2[] = {15.1, 14.8, 15.5, 15.2, 14.9};

	size_t n1 = 5;
	size_t n2 = 5;

	independent_t_test_result result = nsl_stats_independent_t(sample1, n1, sample2, n2, nsl_stats_tail_type_two);

	double expected_t = -5.0671;
	double expected_p = 0.0010;

	const double epsilon = 1e-4;

	printf("Computed t-statistic: %f\n", result.t);
	printf("Computed p-value: %f\n", result.p);

	QVERIFY(std::abs(result.t - expected_t) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testOneSampleT() {
	const double sample[] = {15.2, 14.8, 15.5, 14.9, 15.1};
	size_t n = 5;
	double hypothesized_mean = 15.0;

	one_sample_t_test_result result = nsl_stats_one_sample_t(sample, n, hypothesized_mean, nsl_stats_tail_type_two);

	double expected_t = 0.8165;
	double expected_p = 0.4601;

	const double epsilon = 1e-4;
	printf("Computed t-statistic: %f\n", result.t);
	printf("Computed p-value: %f\n", result.p);
	QVERIFY(std::abs(result.t - expected_t) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testAnovaOneWayRepeated() {
	const double group1[] = {7.0, 2.0, 5.0, 6.0, 4.0, 7.0, 4.0, 5.0};
	const double group2[] = {9.0, 3.0, 7.0, 6.0, 7.0, 8.0, 6.0, 3.0};
	const double group3[] = {8.0, 3.0, 6.0, 4.0, 5.0, 4.0, 4.0, 7.0};
	const size_t n_samples = 8;
	const size_t n_groups = 3;

	const double* groups[] = {const_cast<double*>(group1), const_cast<double*>(group2), const_cast<double*>(group3)};

	anova_oneway_repeated_test_result result = nsl_stats_anova_oneway_repeated_f(groups, n_samples, n_groups);
	printf("Computed F-statistic: %f", result.F);

	printf("Computed p-value: %f\n", result.p);

	double expected_F = 1.68647;
	double expected_p = 0.22069;

	const double epsilon = 1e-3;

	QVERIFY(std::abs(result.F - expected_F) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testWelchT() {
	const double sample1[] = {14.0, 15.0, 15.0, 15.0, 16.0, 18.0, 22.0, 23.0, 24.0, 25.0, 25.0};
	const double sample2[] = {10.0, 12.0, 14.0, 15.0, 18.0, 22.0, 24.0, 27.0, 31.0, 33.0, 34.0, 34.0, 34.0};

	size_t n1 = 11;
	size_t n2 = 13;

	welch_t_test_result result = nsl_stats_welch_t(sample1, n1, sample2, n2, nsl_stats_tail_type_two);

	double expected_t = -1.5379;
	double expected_p = 0.1414;

	const double epsilon = 1e-4;

	printf("Computed t-statistic: %f\n", result.t);
	printf("Computed p-value: %f\n", result.p);

	QVERIFY(std::abs(result.t - expected_t) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testWilcoxon() {
	const double sample1[] = {1.1,	2.3,  3.5,	4.2,  5.8,	6.1,  7.9,	8.4,  9.6,	10.3, 11.7, 12.5, 13.9,
							  14.2, 15.6, 16.8, 17.1, 18.5, 19.3, 20.7, 21.2, 22.8, 23.4, 24.9, 25.1};
	const double sample2[] = {1.5,	2.1,  3.8,	4.0,  5.9,	6.7,  7.2,	8.8,  9.1,	10.5, 11.9, 12.3, 13.5,
							  14.8, 15.2, 16.9, 17.5, 18.0, 19.7, 20.1, 21.5, 22.3, 23.0, 24.5, 25.8};

	size_t n = 25;

	const double epsilon = 1e-3;

	wilcoxon_test_result result = nsl_stats_wilcoxon_w(sample1, sample2, n, nsl_stats_tail_type_two);

	double expected_w = 152.5;
	double expected_p = 0.79808;

	printf("Computed W-statistic: %f\n", result.W);
	printf("Computed p-value: %f\n", result.p);

	QVERIFY(std::abs(result.W - expected_w) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);

	result = nsl_stats_wilcoxon_w(sample1, sample2, n, nsl_stats_tail_type_negative);

	expected_w = 172.5;
	expected_p = 0.61132;

	printf("Computed W-statistic: %f\n", result.W);
	printf("Computed p-value: %f\n", result.p);

	QVERIFY(std::abs(result.W - expected_w) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);

	result = nsl_stats_wilcoxon_w(sample1, sample2, n, nsl_stats_tail_type_positive);

	expected_w = 172.5;
	expected_p = 0.39903;

	printf("Computed W-statistic: %f\n", result.W);
	printf("Computed p-value: %f\n", result.p);

	QVERIFY(std::abs(result.W - expected_w) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testFriedman() {
	const double group1[] = {34.0, 33.0, 41.0, 39.0, 44.0, 37.0, 39.0};
	const double group2[] = {45.0, 36.0, 35.0, 43.0, 42.0, 42.0, 46.0};
	const double group3[] = {36.0, 31.0, 44.0, 42.0, 41.0, 45.0, 40.0};
	const size_t n_samples = 7;
	const size_t n_groups = 3;

	const double* groups[] = {const_cast<double*>(group1), const_cast<double*>(group2), const_cast<double*>(group3)};

	friedman_test_result result = nsl_stats_friedman_q(groups, n_samples, n_groups);
	printf("Computed Q-statistic: %f", result.Q);

	printf("Computed p-value: %f\n", result.p);

	double expected_Q = 2.5714;
	double expected_p = 0.276;

	const double epsilon = 1e-3;

	QVERIFY(std::abs(result.Q - expected_Q) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testChisqIndependence() {
	const long long col1[] = {5, 5};
	const long long col2[] = {7, 5};
	const size_t n_rows = 2;
	const size_t n_cols = 2;

	const long long* table[] = {const_cast<long long*>(col1), const_cast<long long*>(col2)};

	chisq_ind_test_result result = nsl_stats_chisq_ind_x2(table, n_rows, n_cols);
	printf("Computed x2-statistic: %f", result.x2);

	printf("Computed p-value: %f\n", result.p);

	double expected_x2 = 0.1528;
	double expected_p = 0.696;

	const double epsilon = 1e-3;

	QVERIFY(std::abs(result.x2 - expected_x2) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

void NSLStatisticalTestTest::testChisqGoodnessOfFit() {
	const long long observed[] = {50, 110, 40};
	const double expected[] = {60.0, 100.0, 40.0};
	const size_t n = 3;

	chisq_gof_test_result result = nsl_stats_chisq_gof_x2(observed, expected, n, 0);
	printf("Computed x2-statistic: %f", result.x2);

	printf("Computed p-value: %f\n", result.p);

	double expected_x2 = 2.667;
	double expected_p = 0.2636;

	const double epsilon = 1e-3;

	QVERIFY(std::abs(result.x2 - expected_x2) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

/*!
 * \brief Test Mann-Kendall trend test with two-tailed hypothesis on monotonically increasing data.
 * The two-tailed test checks for any trend (upward or downward) without specifying direction.
 */
void NSLStatisticalTestTest::testMannKendall01() {
	const double sample[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
	size_t n = 10;

	mann_kendall_test_result result = nsl_stats_mann_kendall(sample, n, nsl_stats_tail_type_two);

	// printf("Mann-Kendall Test 01 (Two-tailed, positive trend):\n");
	// printf("  S-statistic: %f\n", result.S);
	// printf("  Tau: %f\n", result.tau);
	// printf("  Z-score: %f\n", result.z);
	// printf("  p-value: %f\n", result.p);

	// Expected values for monotonically increasing data:
	// S = 45 (all pairs are increasing)
	// tau = S/n_pairs = 45/45 = 1.0
	double expected_S = 45.0;
	double expected_tau = 1.0;
	double expected_z = 3.9355;
	double expected_p = 0.0001;

	const double epsilon = 1e-3;

	QVERIFY(std::abs(result.S - expected_S) < epsilon);
	QVERIFY(std::abs(result.tau - expected_tau) < epsilon);
	QVERIFY(std::abs(result.z - expected_z) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

/*!
 * \brief Test Mann-Kendall trend test with two-tailed hypothesis on monotonically decreasing data.
 * The two-tailed test checks for any trend (upward or downward) without specifying direction.
 */
void NSLStatisticalTestTest::testMannKendall02() {
	const double sample[] = {10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
	size_t n = 10;

	mann_kendall_test_result result = nsl_stats_mann_kendall(sample, n, nsl_stats_tail_type_two);

	// printf("Mann-Kendall Test 02 (Two-tailed, negative trend):\n");
	// printf("  S-statistic: %f\n", result.S);
	// printf("  Tau: %f\n", result.tau);
	// printf("  Z-score: %f\n", result.z);
	// printf("  p-value: %f\n", result.p);

	// Expected values for monotonically decreasing data:
	// S = -45 (all pairs are decreasing)
	// tau = -1.0
	double expected_S = -45.0;
	double expected_tau = -1.0;
	double expected_z = -3.9355;
	double expected_p = 0.0001;

	const double epsilon = 1e-3;

	QVERIFY(std::abs(result.S - expected_S) < epsilon);
	QVERIFY(std::abs(result.tau - expected_tau) < epsilon);
	QVERIFY(std::abs(result.z - expected_z) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

/*!
 * \brief Test Mann-Kendall trend test with positive (right-tailed) hypothesis.
 *
 * Tests the Mann-Kendall test using monotonically increasing data with a positive tail test.
 * The positive tail test specifically checks for an upward trend (H₁: trend > 0).
 */
void NSLStatisticalTestTest::testMannKendall03() {
	const double sample[] = {1.2, 1.5, 1.8, 2.1, 2.3, 2.8, 3.0, 3.5, 3.9, 4.2};
	size_t n = 10;

	mann_kendall_test_result result = nsl_stats_mann_kendall(sample, n, nsl_stats_tail_type_positive);

	// printf("Mann-Kendall Test 03 (Positive tail):\n");
	// printf("  S-statistic: %f\n", result.S);
	// printf("  Tau: %f\n", result.tau);
	// printf("  Z-score: %f\n", result.z);
	// printf("  p-value: %f\n", result.p);

	// Positive trend should have small p-value for positive tail test
	double expected_S = 45.0;
	double expected_tau = 1.0;
	double expected_z = 3.9355;
	double expected_p = 0.00004; // P(Z > z)

	const double epsilon = 1e-3;

	QVERIFY(std::abs(result.S - expected_S) < epsilon);
	QVERIFY(std::abs(result.tau - expected_tau) < epsilon);
	QVERIFY(std::abs(result.z - expected_z) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

/*!
 * \brief Test Mann-Kendall trend test with negative (left-tailed) hypothesis.
 *
 * Tests the Mann-Kendall test using monotonically decreasing data with a negative tail test.
 * The negative tail test specifically checks for a downward trend (H₁: trend < 0).
 */
void NSLStatisticalTestTest::testMannKendall04() {
	const double sample[] = {4.2, 3.9, 3.5, 3.0, 2.8, 2.3, 2.1, 1.8, 1.5, 1.2};
	size_t n = 10;

	mann_kendall_test_result result = nsl_stats_mann_kendall(sample, n, nsl_stats_tail_type_negative);

	// printf("Mann-Kendall Test 04 (Negative tail):\n");
	// printf("  S-statistic: %f\n", result.S);
	// printf("  Tau: %f\n", result.tau);
	// printf("  Z-score: %f\n", result.z);
	// printf("  p-value: %f\n", result.p);

	// Negative trend should have small p-value for negative tail test
	double expected_S = -45.0;
	double expected_tau = -1.0;
	double expected_z = -3.9355;
	double expected_p = 0.00004; // P(Z < z)

	const double epsilon = 1e-3;

	QVERIFY(std::abs(result.S - expected_S) < epsilon);
	QVERIFY(std::abs(result.tau - expected_tau) < epsilon);
	QVERIFY(std::abs(result.z - expected_z) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

/*!
 * \brief Test Mann-Kendall trend test with no clear trend (mixed data).
 *
 * Tests the Mann-Kendall test using data without a clear monotonic trend.
 * The data contains both increases and decreases in a non-systematic pattern.
 * This test validates that the Mann-Kendall test correctly identifies the absence
 * of a trend when the data does not follow a monotonic pattern.
 */
void NSLStatisticalTestTest::testMannKendall05() {
	const double sample[] = {3.0, 2.0, 5.0, 4.0, 1.0, 7.0, 6.0, 9.0, 8.0, 10.0};
	size_t n = 10;

	mann_kendall_test_result result = nsl_stats_mann_kendall(sample, n, nsl_stats_tail_type_two);

	// printf("Mann-Kendall Test 05 (No clear trend):\n");
	// printf("  S-statistic: %f\n", result.S);
	// printf("  Tau: %f\n", result.tau);
	// printf("  Z-score: %f\n", result.z);
	// printf("  p-value: %f\n", result.p);

	// With mixed data, we expect:
	// - S to be relatively small
	// - tau to be moderate
	// - p-value to indicate a weak positive trend
	double expected_S = 29.0;
	double expected_tau = 0.6444;
	double expected_z = 2.5044;
	double expected_p = 0.0123;

	const double epsilon = 1e-3;

	QVERIFY(std::abs(result.S - expected_S) < epsilon);
	QVERIFY(std::abs(result.tau - expected_tau) < epsilon);
	QVERIFY(std::abs(result.z - expected_z) < epsilon);
	QVERIFY(std::abs(result.p - expected_p) < epsilon);
}

/*!
 * \brief Test Wald-Wolfowitz runs test with a random sequence.
 *
 * Tests the Wald-Wolfowitz runs test using data that appears random.
 * The sequence alternates moderately around the median without obvious clustering.
 * This test validates that the Wald-Wolfowitz runs test correctly identifies randomness
 * when the data does not show significant clustering or excessive alternation.
 */
void NSLStatisticalTestTest::testWaldWolfowitzRuns01() {
	// Example data that should appear reasonably random
	// Data with moderate number of runs around expected
	const double sample[] = {5.0, 7.0, 3.0, 8.0, 9.0, 2.0, 6.0, 4.0, 10.0, 1.0};
	size_t n = 10;

	wald_wolfowitz_runs_test_result result = nsl_stats_wald_wolfowitz_runs(sample, n, nsl_stats_tail_type_two);

	// Expected values for this dataset:
	// Sorted: 1,2,3,4,5,6,7,8,9,10
	// Median = (5+6)/2 = 5.5
	// Above median (>5.5): 7, 8, 9, 6, 10 = 5 values
	// Below median (<5.5): 5, 3, 2, 4, 1 = 5 values
	// Sequence: B A B A A B A B A B (B=below, A=above)
	// Number of runs: 9
	// n1=5, n2=5
	// Expected runs: (2*5*5)/(5+5) + 1 = 6.0
	// Variance: (2*5*5*(2*5*5-10))/(100*9) = 2.222
	// z-score = (9 - 0.5 - 6.0) / sqrt(2.222) ≈ 1.68
	// p-value ≈ 0.09 (reasonably random)

	const double epsilon = 1e-3;

	QCOMPARE(result.runs, 9);
	QCOMPARE(result.n, 10);
	QVERIFY(result.z > 0.0 && result.z < 2.5); // z should be positive but moderate
	QVERIFY(result.p > 0.05); // p-value should indicate randomness
}

/*!
 * \brief Test Wald-Wolfowitz runs test with clustered data (too few runs).
 *
 * Tests the Wald-Wolfowitz runs test using data that shows clustering.
 * The sequence has consecutive low values followed by consecutive high values,
 * resulting in too few runs compared to random expectation.
 * This test validates that the test correctly detects non-randomness due to clustering.
 */
void NSLStatisticalTestTest::testWaldWolfowitzRuns02() {
	// Data showing clear clustering (low values then high values)
	const double sample[] = {10.0, 11.0, 12.0, 13.0, 14.0, 20.0, 21.0, 22.0, 23.0, 24.0};
	size_t n = 10;

	wald_wolfowitz_runs_test_result result = nsl_stats_wald_wolfowitz_runs(sample, n, nsl_stats_tail_type_negative);

	// Expected values for this dataset:
	// Median = 17.0
	// Below median (<17): 10, 11, 12, 13, 14 = 5 values
	// Above median (>17): 20, 21, 22, 23, 24 = 5 values
	// Sequence: B B B B B A A A A A (B=below, A=above)
	// Number of runs: 2 (significantly fewer than expected)
	// n1=5, n2=5
	// Expected runs: (2*5*5)/(5+5) + 1 = 6.0
	// Variance: (2*5*5*(2*5*5-10))/(100*9) = 2.222
	// z-score = (2 + 0.5 - 6.0) / sqrt(2.222) ≈ -2.35 (negative z indicates clustering)
	// p-value for left-tailed test should be small

	const double epsilon = 1e-1;

	QCOMPARE(result.runs, 2);
	QCOMPARE(result.n, 10);
	QVERIFY(result.z < -2.0); // z should be significantly negative
	QVERIFY(result.p < 0.05); // p-value should be small (not random, clustered)
}

/*!
 * \brief Test Wald-Wolfowitz runs test with alternating data (too many runs).
 *
 * Tests the Wald-Wolfowitz runs test using data that alternates excessively.
 * The sequence switches between high and low values frequently,
 * resulting in too many runs compared to random expectation.
 * This test validates that the test correctly detects non-randomness due to excessive alternation.
 */
void NSLStatisticalTestTest::testWaldWolfowitzRuns03() {
	// Data showing excessive alternation
	const double sample[] = {10.0, 20.0, 11.0, 21.0, 12.0, 22.0, 13.0, 23.0, 14.0, 24.0};
	size_t n = 10;

	wald_wolfowitz_runs_test_result result = nsl_stats_wald_wolfowitz_runs(sample, n, nsl_stats_tail_type_positive);

	// Expected values for this dataset:
	// Median = 17.0
	// Below median (<17): 10, 11, 12, 13, 14 = 5 values
	// Above median (>17): 20, 21, 22, 23, 24 = 5 values
	// Sequence: B A B A B A B A B A (alternating)
	// Number of runs: 10 (maximum possible, excessive alternation)
	// n1=5, n2=5
	// Expected runs: (2*5*5)/(5+5) + 1 = 6.0
	// Variance: (2*5*5*(2*5*5-10))/(100*9) = 2.222
	// z-score = (10 - 0.5 - 6.0) / sqrt(2.222) ≈ 2.35 (positive z indicates alternation)
	// p-value for right-tailed test should be small

	const double epsilon = 1e-1;

	QCOMPARE(result.runs, 10);
	QCOMPARE(result.n, 10);
	QVERIFY(result.z > 2.0); // z should be significantly positive
	QVERIFY(result.p < 0.05); // p-value should be small (not random, alternating)
}

QTEST_MAIN(NSLStatisticalTestTest)
