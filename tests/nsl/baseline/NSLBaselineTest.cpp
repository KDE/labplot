/*
	File                 : NSLBaselineTest.cpp
	Project              : LabPlot
	Description          : NSL Tests for baseline functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLBaselineTest.h"
#include "backend/nsl/nsl_baseline.h"

#include <fstream>

// ##############################################################################
// #################  constant base lines
// ##############################################################################

void NSLBaselineTest::testBaselineMinimum() {
	double data[] = {4., 5., 2.};
	const double result[] = {2., 3., 0.};
	const size_t N = 3;

	nsl_baseline_remove_minimum(data, N);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}
void NSLBaselineTest::testBaselineMinimum2() {
	double data[] = {-4., -5., -2.};
	const double result[] = {1., 0., 3.};
	const size_t N = 3;

	nsl_baseline_remove_minimum(data, N);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}
void NSLBaselineTest::testBaselineMaximum() {
	double data[] = {4., 5., 2.};
	const double result[] = {-1., 0., -3.};
	const size_t N = 3;

	nsl_baseline_remove_maximum(data, N);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}
void NSLBaselineTest::testBaselineMaximum2() {
	double data[] = {-4., -5., -2.};
	const double result[] = {-2., -3., 0.};
	const size_t N = 3;

	nsl_baseline_remove_maximum(data, N);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}

void NSLBaselineTest::testBaselineMean() {
	double data[] = {4., 5., 2.};
	const double result[] = {1 / 3., 4 / 3., -5 / 3.};
	const size_t N = 3;

	nsl_baseline_remove_mean(data, N);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}
void NSLBaselineTest::testBaselineMean2() {
	double data[] = {-4., -5., -2.};
	const double result[] = {-1 / 3., -4 / 3., 5 / 3.};
	const size_t N = 3;

	nsl_baseline_remove_mean(data, N);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}

void NSLBaselineTest::testBaselineMedian() {
	double data[] = {4., 5., 2.};
	const double result[] = {0., 1., -2.};
	const size_t N = 3;

	nsl_baseline_remove_median(data, N);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}
void NSLBaselineTest::testBaselineMedian2() {
	double data[] = {-4., -5., -2.};
	const double result[] = {0., -1., 2.};
	const size_t N = 3;

	nsl_baseline_remove_median(data, N);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(data[i], result[i]);
}

// ##############################################################################
// #################  non-constant base lines
// ##############################################################################

void NSLBaselineTest::testBaselineEndpoints() {
	double xdata[] = {1., 2., 4.};
	double ydata[] = {4., 5., 2.};
	const double result[] = {0., 13. / 3., 0.};
	const size_t N = 3;

	nsl_baseline_remove_endpoints(xdata, ydata, N);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(ydata[i], result[i]);
}

void NSLBaselineTest::testBaselineLinReg() {
	double xdata[] = {1., 2., 4.};
	double ydata[] = {4., 5., 2.};
	const double result[] = {-0.7142857142857144, 1.0714285714285712, -0.35714285714285765};
	const size_t N = 3;

	nsl_baseline_remove_linreg(xdata, ydata, N);

	for (size_t i = 0; i < N; ++i)
		QCOMPARE(ydata[i], result[i]);
}

void NSLBaselineTest::testBaselineARPLS() {
	double data[] = {1, 2, 3, 2, 4, 1, 3, 2, 3, 2};
	double result[] = {-0.605556598278328,
					   0.29922426449953,
					   1.20406568260443,
					   0.108998357316374,
					   2.0139914015879,
					   -1.0809969643924,
					   0.823990535656255,
					   -0.270980722318351,
					   0.634078132872751,
					   -0.460816930930479};

	const size_t N = 10;

	double tol = nsl_baseline_remove_arpls(data, N, 1.e-3, 1.e4, 10);
	WARN("TOL = " << tol)

	FuzzyCompare(tol, 0.0043202087307554, 5.e-9); // GSL value
	for (size_t i = 0; i < N; ++i)
		FuzzyCompare(data[i], result[i], 1.e-10);
}

void NSLBaselineTest::testBaselineARPLSSpectrum() {
	std::ifstream d(QFINDTESTDATA(QLatin1String("data/spectrum.dat")).toStdString());
	std::ifstream r(QFINDTESTDATA(QLatin1String("data/spectrum_arpls.dat")).toStdString());
	const size_t N = 1000;

	double data[N], result[N];
	for (size_t i = 0; i < N; i++) {
		d >> data[i];
		r >> result[i];
	}

	double tol = nsl_baseline_remove_arpls(data, N, 1.e-2, 1.e4, 10);
	WARN("TOL = " << tol)

	FuzzyCompare(tol, 0.108167623062361, 1.e-9); // GSL value
	for (size_t i = 0; i < N; ++i)
		FuzzyCompare(data[i], result[i], 2.e-5);
}

void NSLBaselineTest::testBaselineARPLS_XRD() {
	std::ifstream d(QFINDTESTDATA(QLatin1String("data/XRD.dat")).toStdString());
	std::ifstream r(QFINDTESTDATA(QLatin1String("data/XRD_arpls.dat")).toStdString());
	const size_t N = 1764;

	double data[N], result[N];
	for (size_t i = 0; i < N; i++) {
		d >> data[i];
		r >> result[i];
	}

	double tol = nsl_baseline_remove_arpls(data, N, 1.e-3, 1.e6, 20);
	WARN("TOL = " << tol)

	FuzzyCompare(tol, 0.0068956252520988, 1.e-6); // GSL value
	// std::ofstream o("out.dat");
	for (size_t i = 0; i < N; ++i) {
		// o << data[i] << std::endl;
		FuzzyCompare(data[i], result[i], 2.e-5);
	}
}

QTEST_MAIN(NSLBaselineTest)
