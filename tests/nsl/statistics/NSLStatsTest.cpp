/*
	File                 : NSLStatsTest.cpp
	Project              : LabPlot
	Description          : NSL Tests for statistical functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLStatsTest.h"
#include "backend/nsl/nsl_stats.h"

// ##############################################################################
// #################  Quantile test
// ##############################################################################

const int N = 10;
const int NQ = 13;

void NSLStatsTest::testQuantile() {
	const double data_sorted[] = {1, 1, 1, 3, 4, 7, 9, 11, 13, 13};
	double data_unsorted[] = {3, 7, 11, 1, 13, 1, 9, 1, 13, 4};
	const double quantile[] = {.0, .1, .2, .25, .3, .4, .5, .6, .7, .75, .8, .9, 1.};
	const double result[NSL_STATS_QUANTILE_TYPE_COUNT][NQ + 1] = {
		{1., 1., 1., 1., 1., 3., 4., 7., 9., 11., 11., 13., 13., 4.},
		{1., 1., 1., 2., 2., 3.5, 5.5, 8., 10., 12., 12., 13., 13., 5.5},
		{1., 1., 1., 1., 1., 3., 4., 7., 9., 11., 11., 13., 13., 4.},
		{1., 1., 1., 1., 1., 3., 4., 7., 9., 10., 11., 13., 13., 4.},
		{1., 1., 1., 1., 2., 3.5, 5.5, 8., 10., 11., 12., 13., 13., 5.5},
		{1., 1., 1., 1., 1.6, 3.4, 5.5, 8.2, 10.4, 11.5, 12.6, 13., 13., 5.5},
		{1., 1., 1., 1.5, 2.4, 3.6, 5.5, 7.8, 9.6, 10.5, 11.4, 13., 13., 5.5},
		{1., 1., 1., 1., 28. / 15., 52. / 15., 5.5, 121. / 15., 152. / 15., 335. / 30., 12.2, 13., 13., 5.5},
		{1., 1., 1., 1., 1.9, 3.475, 5.5, 8.05, 10.1, 11.125, 12.15, 13., 13., 5.5}};

	int type, i;
	for (type = 1; type <= NSL_STATS_QUANTILE_TYPE_COUNT; ++type) {
		printf("quantile type %d\n", type);
		for (i = 0; i < NQ; ++i) {
			double value = nsl_stats_quantile_sorted(data_sorted, 1, N, quantile[i], (nsl_stats_quantile_type)type);
			// printf("%d %d: %g %g\n", i, j, value, result[i-1][j]);
			QCOMPARE(value, result[type - 1][i]);
		}
		QCOMPARE(nsl_stats_median_sorted(data_sorted, 1, N, (nsl_stats_quantile_type)type), result[type - 1][NQ]);
	}
	for (type = 1; type <= NSL_STATS_QUANTILE_TYPE_COUNT; ++type) {
		printf("quantile type %d\n", type);
		for (i = 0; i < NQ; ++i) {
			double value = nsl_stats_quantile(data_unsorted, 1, N, quantile[i], (nsl_stats_quantile_type)type);
			// printf("%d %d: %g %g\n", i, j, value, result[i-1][j]);
			QCOMPARE(value, result[type - 1][i]);
		}
		QCOMPARE(nsl_stats_median_sorted(data_sorted, 1, N, (nsl_stats_quantile_type)type), result[type - 1][NQ]);
	}
}

void NSLStatsTest::testNaNInfHandling() {
	double min_data[] = {NAN, 3.0, INFINITY, -2.0, 1.0, -INFINITY, 4.0};
	double max_data[] = {NAN, -INFINITY, 3.0, -2.0, 1.0, INFINITY, 4.0};
	double median_data[] = {NAN, 3.0, INFINITY, 1.0, 2.0, -INFINITY, 4.0};

	QCOMPARE(nsl_stats_minimum(min_data, 7, nullptr), -2.0);
	QCOMPARE(nsl_stats_maximum(max_data, 7, nullptr), 4.0);
	QCOMPARE(nsl_stats_median(median_data, 1, 7, nsl_stats_quantile_type7), 2.5);
	QCOMPARE(nsl_stats_quantile(median_data, 1, 7, 0.5, nsl_stats_quantile_type7), 2.5);

	double all_invalid[] = {NAN, INFINITY, -INFINITY};
	QVERIFY(std::isnan(nsl_stats_minimum(all_invalid, 3, nullptr)));
	QVERIFY(std::isnan(nsl_stats_maximum(all_invalid, 3, nullptr)));
	QVERIFY(std::isnan(nsl_stats_median(all_invalid, 1, 3, nsl_stats_quantile_type7)));
	QVERIFY(std::isnan(nsl_stats_quantile(all_invalid, 1, 3, 0.5, nsl_stats_quantile_type7)));
}

void NSLStatsTest::testRemainingStatistics() {
	const double sorted[] = {1, 2, 3, 4, 5};
	QCOMPARE(nsl_stats_median_from_sorted_data(sorted, 1, 5), 3.);
	QCOMPARE(nsl_stats_quantile_from_sorted_data(sorted, 1, 5, 0.25), 2.);

	const double data[] = {3, -2, 8, 8};
	size_t index = 99;
	QCOMPARE(nsl_stats_minimum(data, 4, &index), -2.);
	QCOMPARE(index, size_t(1));
	QCOMPARE(nsl_stats_maximum(data, 4, &index), 8.);
	QCOMPARE(index, size_t(2));
	QVERIFY(std::isnan(nsl_stats_minimum(data, 0, &index)));
	QCOMPARE(index, size_t(0));

	QCOMPARE(nsl_stats_rsquare(2., 10.), 0.8);
	QVERIFY(std::abs(nsl_stats_rsquareAdj(0.8, 2, 8, 2) - 0.775) < 1.e-12);
	QCOMPARE(nsl_stats_tdist_t(2., 0.5), 4.);
	QCOMPARE(nsl_stats_tdist_p(0., 10.), 1.);
	const double tCritical = nsl_stats_tdist_z(0.05, 100.);
	QVERIFY(tCritical > 1.9 && tCritical < 2.1);
	QVERIFY(std::abs(nsl_stats_tdist_margin(0.05, 100., 0.5) - 0.5 * tCritical) < 1.e-12);
	QCOMPARE(nsl_stats_chisq_p(0., 4.), 1.);
	QVERIFY(nsl_stats_chisq_low(0.05, 4.) < nsl_stats_chisq_high(0.05, 4.));
	QCOMPARE(nsl_stats_fdist_F(0.5, 2, 10), 10.);
	QCOMPARE(nsl_stats_fdist_p(0., 2, 10.), 1.);

	QVERIFY(std::isfinite(nsl_stats_logLik(4., 10)));
	QVERIFY(std::isfinite(nsl_stats_aic(4., 10, 2, 1)));
	QVERIFY(std::isfinite(nsl_stats_aic(4., 10, 2, 2)));
	QVERIFY(std::isfinite(nsl_stats_aic(4., 10, 2, 3)));
	QVERIFY(std::isfinite(nsl_stats_aicc(4., 10, 2, 1)));
	QVERIFY(std::isfinite(nsl_stats_aicc(4., 10, 2, 2)));
	QVERIFY(std::isfinite(nsl_stats_bic(4., 10, 2, 1)));
	QVERIFY(std::isfinite(nsl_stats_bic(4., 10, 2, 2)));
}

// ##############################################################################
// #################  performance
// ##############################################################################

QTEST_MAIN(NSLStatsTest)
