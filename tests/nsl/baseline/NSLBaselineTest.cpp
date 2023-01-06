/*
	File                 : NSLBaselineTest.cpp
	Project              : LabPlot
	Description          : NSL Tests for baseline functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLBaselineTest.h"

extern "C" {
#include "backend/nsl/nsl_baseline.h"
}

//##############################################################################
//#################  Min/Max, etc. test
//##############################################################################

void NSLBaselineTest::testBaselineMinimum() {
	double data[] = {4., 5., 2.};
	const double result[] = {2., 3., 0.};
	const size_t N = 3;

	nsl_baseline_remove_minimum(data, 3);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}
void NSLBaselineTest::testBaselineMinimum2() {
	double data[] = {-4., -5., -2.};
	const double result[] = {1., 0., 3.};
	const size_t N = 3;

	nsl_baseline_remove_minimum(data, 3);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}
void NSLBaselineTest::testBaselineMaximum() {
	double data[] = {4., 5., 2.};
	const double result[] = {-1., 0., -3.};
	const size_t N = 3;

	nsl_baseline_remove_maximum(data, 3);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}
void NSLBaselineTest::testBaselineMaximum2() {
	double data[] = {-4., -5., -2.};
	const double result[] = {-2., -3., 0.};
	const size_t N = 3;

	nsl_baseline_remove_maximum(data, 3);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}

void NSLBaselineTest::testBaselineMean() {
	double data[] = {4., 5., 2.};
	const double result[] = {1 / 3., 4 / 3., -5 / 3.};
	const size_t N = 3;

	nsl_baseline_remove_mean(data, 3);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}
void NSLBaselineTest::testBaselineMean2() {
	double data[] = {-4., -5., -2.};
	const double result[] = {-1 / 3., -4 / 3., 5 / 3.};
	const size_t N = 3;

	nsl_baseline_remove_mean(data, 3);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}

void NSLBaselineTest::testBaselineMedian() {
	double data[] = {4., 5., 2.};
	const double result[] = {0., 1., -2.};
	const size_t N = 3;

	nsl_baseline_remove_median(data, 3);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}
void NSLBaselineTest::testBaselineMedian2() {
	double data[] = {-4., -5., -2.};
	const double result[] = {0., -1., 2.};
	const size_t N = 3;

	nsl_baseline_remove_median(data, 3);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}

QTEST_MAIN(NSLBaselineTest)
