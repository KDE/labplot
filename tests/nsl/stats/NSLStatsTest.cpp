/*
    File                 : NSLStatsTest.cpp
    Project              : LabPlot
    Description          : NSL Tests for statistical functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach (stefan.gerlach@uni.kn)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLStatsTest.h"

extern "C" {
#include "backend/nsl/nsl_stats.h"
}

//##############################################################################
//#################  Quantile test
//##############################################################################
	
const int N = 10;
const int NQ = 13;

void NSLStatsTest::testQuantile() {
	const double data_sorted[] = {1, 1, 1, 3, 4, 7, 9, 11, 13, 13};
	double data_unsorted[] = {3, 7, 11, 1, 13, 1, 9, 1, 13, 4};
	const double quantile[] = {.0, .1, .2, .25, .3, .4, .5, .6, .7, .75, .8, .9, 1.};
	const double result[NSL_STATS_QUANTILE_TYPE_COUNT][NQ+1] = {
		{1., 1., 1., 1., 1., 3., 4., 7., 9., 11., 11., 13., 13., 4.},
		{1., 1., 1., 2., 2., 3.5, 5.5, 8., 10., 12., 12., 13., 13., 5.5},
		{1., 1., 1., 1., 1., 3., 4., 7., 9., 11., 11., 13., 13., 4.},
		{1., 1., 1., 1., 1., 3., 4., 7., 9., 10., 11., 13., 13., 4.},
		{1., 1., 1., 1., 2., 3.5, 5.5, 8., 10., 11., 12., 13., 13., 5.5},
		{1., 1., 1., 1., 1.6, 3.4, 5.5, 8.2, 10.4, 11.5, 12.6, 13., 13., 5.5},
		{1., 1., 1., 1.5, 2.4, 3.6, 5.5, 7.8, 9.6, 10.5, 11.4, 13., 13., 5.5},
		{1., 1., 1., 1., 28./15., 52./15., 5.5, 121./15., 152./15., 335./30., 12.2, 13., 13., 5.5},
		{1., 1., 1., 1., 1.9, 3.475, 5.5, 8.05, 10.1, 11.125, 12.15, 13., 13., 5.5}};

	int type, i;
	for (type = 1; type <= NSL_STATS_QUANTILE_TYPE_COUNT; ++type) {
		printf("quantile type %d\n", type);
		for (i = 0; i < NQ; ++i) {
			double value = nsl_stats_quantile_sorted(data_sorted, 1, N, quantile[i], (nsl_stats_quantile_type)type);
			//printf("%d %d: %g %g\n", i, j, value, result[i-1][j]);
			QCOMPARE(value, result[type-1][i]);
		}
		QCOMPARE(nsl_stats_median_sorted(data_sorted, 1, N, (nsl_stats_quantile_type)type), result[type-1][NQ]);
	}
	for (type = 1; type <= NSL_STATS_QUANTILE_TYPE_COUNT; ++type) {
		printf("quantile type %d\n", type);
		for (i = 0; i < NQ; ++i) {
			double value = nsl_stats_quantile(data_unsorted, 1, N, quantile[i], (nsl_stats_quantile_type)type);
			//printf("%d %d: %g %g\n", i, j, value, result[i-1][j]);
			QCOMPARE(value, result[type-1][i]);
		}
		QCOMPARE(nsl_stats_median_sorted(data_sorted, 1, N, (nsl_stats_quantile_type)type), result[type-1][NQ]);
	}
}

//##############################################################################
//#################  performance
//##############################################################################

QTEST_MAIN(NSLStatsTest)
