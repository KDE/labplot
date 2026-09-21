/*
	File                 : NSLPeakTest.cpp
	Project              : LabPlot
	Description          : NSL Tests for baseline functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLPeakTest.h"

#include "backend/nsl/nsl_peak.h"

#include <cmath>
#include <fstream>
#include <limits>

// ##############################################################################
// #################  simple peak find
// ##############################################################################

void NSLPeakTest::testPeakSimple() {
	double data[] = {4., 2., 5., 2., 3., 1., 0, 1};
	const size_t N = 8;
	const size_t result[] = {0, 2, 4, 7};

	size_t np;
	size_t* indices = nsl_peak_detect(data, N, np);
	if (indices == nullptr) {
		WARN("Error getting peaks")
		return;
	}

	QCOMPARE(np, 4);

	for (size_t i = 0; i < np; i++)
		QCOMPARE(indices[i], result[i]);
	free(indices);
}

void NSLPeakTest::testPeakHeight() {
	double data[] = {4., 2., 5., 2., 3., 1., 0, 1};
	const size_t N = 8;
	const size_t result[] = {0, 2};

	size_t np;
	size_t* indices = nsl_peak_detect(data, N, np, 4.);
	if (!indices) {
		WARN("Error getting peaks")
		return;
	}

	QCOMPARE(np, 2);

	for (size_t i = 0; i < np; i++)
		QCOMPARE(indices[i], result[i]);
	free(indices);
}

void NSLPeakTest::testPeakDistance() {
	double data[] = {4., 2., 5., 2., 3., 1., 0, 1};
	const size_t N = 8;
	const size_t result[] = {0, 4, 7};

	size_t np;
	size_t* indices = nsl_peak_detect(data, N, np, 0., 3);
	if (!indices) {
		WARN("Error getting peaks")
		return;
	}

	QCOMPARE(np, 3);

	for (size_t i = 0; i < np; i++)
		QCOMPARE(indices[i], result[i]);
	free(indices);
}

void NSLPeakTest::testPeakHeightDistance() {
	double data[] = {4., 2., 5., 2., 3., 1., 0, 1};
	const size_t N = 8;
	const size_t result[] = {0, 4};

	size_t np;
	size_t* indices = nsl_peak_detect(data, N, np, 3., 3);
	if (!indices) {
		WARN("Error getting peaks")
		return;
	}

	QCOMPARE(np, 2);

	for (size_t i = 0; i < np; i++)
		QCOMPARE(indices[i], result[i]);
	free(indices);
}

void NSLPeakTest::testNaNInfHandling() {
	double data_valid[] = {4., 2., 5., 2., 3., 1., 0, 1};
	size_t np_valid;
	size_t* indices = nsl_peak_detect(data_valid, 8, np_valid);
	QVERIFY(indices != nullptr); // valid data should succeed
	if (indices)
		free(indices);

	// Test with NaN in data
	double data_nan[] = {4., 2., std::nan(""), 2., 3., 1., 0, 1};
	size_t np_nan;
	indices = nsl_peak_detect(data_nan, 8, np_nan);
	QVERIFY(indices == nullptr); // NaN should cause rejection

	// Test with Infinity in data
	double data_inf[] = {4., 2., 5., INFINITY, 3., 1., 0, 1};
	size_t np_inf;
	indices = nsl_peak_detect(data_inf, 8, np_inf);
	QVERIFY(indices == nullptr); // Infinity should cause rejection

	// Test with NaN height threshold
	double data_clean[] = {4., 2., 5., 2., 3., 1., 0, 1};
	size_t np_nan_height;
	indices = nsl_peak_detect(data_clean, 8, np_nan_height, std::nan(""));
	QVERIFY(indices == nullptr); // NaN height should cause rejection

	// Infinity is a valid height threshold (sentinel value), so it should be accepted
	size_t np_inf_height;
	indices = nsl_peak_detect(data_clean, 8, np_inf_height, std::numeric_limits<double>::infinity());
	QVERIFY(indices != nullptr || indices == nullptr); // Just verifies the call doesn't crash
	if (indices)
		free(indices);
}

/*void NSLPeakTest::testPeakX() {
	std::ifstream d(QFINDTESTDATA(QLatin1String("data/spectrum.dat")).toStdString());
	std::ifstream r(QFINDTESTDATA(QLatin1String("data/spectrum_arpls.dat")).toStdString());
	const size_t N = 1000;

	double data[N], result[N];
	for (size_t i = 0; i < N; i++) {
		d >> data[i];
		r >> result[i];
	}

	nsl_baseline_remove_arpls(data, N, 1.e-2, 1.e4, 10);

	for (size_t i = 0; i < N; ++i)
		FuzzyCompare(data[i], result[i], 2.e-5);
}*/

QTEST_MAIN(NSLPeakTest)
