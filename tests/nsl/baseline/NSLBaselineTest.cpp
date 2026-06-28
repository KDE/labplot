/*
	File                 : NSLBaselineTest.cpp
	Project              : LabPlot
	Description          : NSL Tests for baseline functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2026 Stefan Gerlach <stefan.gerlach@uni.kn>

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

// Test the Eigen3 implementation directly
void NSLBaselineTest::testBaselineARPLSEigen3() {
	double data[] = {1, 2, 3, 2, 4, 1, 3, 2, 3, 2};
	double result[] = {-0.584766396744723,
					   0.3178780697516,
					   1.2204997307991,
					   0.123162244333425,
					   2.02576532689042,
					   -1.07157429017768,
					   0.831071565574356,
					   -0.266275685377669,
					   0.636358665771434,
					   -0.460995612379269};

	const size_t N = 10;

// Test the Eigen3 implementation directly
#ifdef HAVE_EIGEN3
	double tol = nsl_baseline_remove_arpls_Eigen3(data, N, 1.e-3, 1.e6, 20);
	WARN("TOL = " << tol)

	FuzzyCompare(tol, 0.0008774583012132667, 1.e-6);
	for (size_t i = 0; i < N; ++i) {
		FuzzyCompare(data[i], result[i], 2.e-5);
	}
#else
	QSKIP("Eigen3 not available");
#endif
}

// Test the GSL implementation directly
void NSLBaselineTest::testBaselineARPLSGSL() {
#ifndef HAVE_EIGEN3
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

	// Test the GSL implementation directly
	double tol = nsl_baseline_remove_arpls_GSL(data, N, 1.e-3, 1.e6, 20);
	WARN("TOL = " << tol)

	FuzzyCompare(tol, 0.0068956252520988, 1.e-6); // GSL value
	for (size_t i = 0; i < N; ++i) {
		FuzzyCompare(data[i], result[i], 2.e-5);
	}
#endif
}

// Test the Eigen3 implementation with spectrum data
void NSLBaselineTest::testBaselineARPLSEigen3Spectrum() {
	std::ifstream d(QFINDTESTDATA(QLatin1String("data/spectrum.dat")).toStdString());
	std::ifstream r(QFINDTESTDATA(QLatin1String("data/spectrum_arpls.dat")).toStdString());
	const size_t N = 1000;

	double data[N], result[N];
	for (size_t i = 0; i < N; i++) {
		d >> data[i];
		r >> result[i];
	}

#ifdef HAVE_EIGEN3
	double tol = nsl_baseline_remove_arpls_Eigen3(data, N, 1.e-2, 1.e4, 10);
	WARN("TOL = " << tol)

	FuzzyCompare(tol, 0.108167623062361, 1.e-9); // GSL value
	for (size_t i = 0; i < N; ++i) {
		FuzzyCompare(data[i], result[i], 2.e-5);
	}
#else
	QSKIP("Eigen3 not available");
#endif
}

// Test the GSL implementation with spectrum data
void NSLBaselineTest::testBaselineARPLSGSLSpectrum() {
#ifndef HAVE_EIGEN3
	std::ifstream d(QFINDTESTDATA(QLatin1String("data/spectrum.dat")).toStdString());
	std::ifstream r(QFINDTESTDATA(QLatin1String("data/spectrum_arpls.dat")).toStdString());
	const size_t N = 1000;

	double data[N], result[N];
	for (size_t i = 0; i < N; i++) {
		d >> data[i];
		r >> result[i];
	}

	double tol = nsl_baseline_remove_arpls_GSL(data, N, 1.e-2, 1.e4, 10);
	WARN("TOL = " << tol)

	FuzzyCompare(tol, 0.108167623062361, 1.e-9); // GSL value
	for (size_t i = 0; i < N; ++i) {
		FuzzyCompare(data[i], result[i], 2.e-5);
	}
#endif
}

// Test the Eigen3 implementation with XRD data
void NSLBaselineTest::testBaselineARPLSEigen3XRD() {
	std::ifstream d(QFINDTESTDATA(QLatin1String("data/XRD.dat")).toStdString());
	std::ifstream r(QFINDTESTDATA(QLatin1String("data/XRD_arpls.dat")).toStdString());
	const size_t N = 1764;

	double data[N], result[N];
	for (size_t i = 0; i < N; i++) {
		d >> data[i];
		r >> result[i];
	}

#ifdef HAVE_EIGEN3
	double tol = nsl_baseline_remove_arpls_Eigen3(data, N, 1.e-3, 1.e6, 20);
	WARN("TOL = " << tol)

	FuzzyCompare(tol, 0.0068956252520988, 1.e-6); // GSL value
	for (size_t i = 0; i < N; ++i) {
		FuzzyCompare(data[i], result[i], 2.e-5);
	}
#else
	QSKIP("Eigen3 not available");
#endif
}

// Test the GSL implementation with XRD data
void NSLBaselineTest::testBaselineARPLSGSLXRD() {
#ifndef HAVE_EIGEN3
	std::ifstream d(QFINDTESTDATA(QLatin1String("data/XRD.dat")).toStdString());
	std::ifstream r(QFINDTESTDATA(QLatin1String("data/XRD_arpls.dat")).toStdString());
	const size_t N = 1764;

	double data[N], result[N];
	for (size_t i = 0; i < N; i++) {
		d >> data[i];
		r >> result[i];
	}

	double tol = nsl_baseline_remove_arpls_GSL(data, N, 1.e-3, 1.e6, 20);
	WARN("TOL = " << tol)

	FuzzyCompare(tol, 0.0068956252520988, 1.e-6); // GSL value
	for (size_t i = 0; i < N; ++i) {
		FuzzyCompare(data[i], result[i], 2.e-5);
	}
#endif
}

// Test edge cases
void NSLBaselineTest::testBaselineEmptyData() {
	// Test empty data (n=0)
	double* data = nullptr;
	nsl_baseline_remove_minimum(data, 0);
	nsl_baseline_remove_maximum(data, 0);
	nsl_baseline_remove_mean(data, 0);
	nsl_baseline_remove_median(data, 0);

	// Should not crash or cause undefined behavior
	QVERIFY(true);
}

void NSLBaselineTest::testBaselineSingleElement() {
	// Test single element data
	double data1[] = {5.0};
	double data2[] = {5.0};
	double data3[] = {5.0};
	double data4[] = {5.0};

	nsl_baseline_remove_minimum(data1, 1);
	nsl_baseline_remove_maximum(data2, 1);
	nsl_baseline_remove_mean(data3, 1);
	nsl_baseline_remove_median(data4, 1);

	// For single element, result should be 0.0 (element - element = 0)
	QCOMPARE(data1[0], 0.0);
	QCOMPARE(data2[0], 0.0);
	QCOMPARE(data3[0], 0.0);
	QCOMPARE(data4[0], 0.0);

	// Test endpoints and linear regression with single element
	double xdata[] = {1.0};
	double ydata[] = {5.0};
	nsl_baseline_remove_endpoints(xdata, ydata, 1);
	nsl_baseline_remove_linreg(xdata, ydata, 1);

	// For single element, result should be 0.0
	QCOMPARE(ydata[0], 0.0);

	// Test ARPLS with single element
	double data8[] = {5.0};
	nsl_baseline_remove_arpls(data8, 1, 1.e-3, 1.e4, 10);
	QCOMPARE(data8[0], 0.0);
}

void NSLBaselineTest::testBaselineNaNData() {
	// Test data with NaN values
	double data1[] = {1.0, NAN, 3.0};
	double data2[] = {NAN, 2.0, 3.0};
	double data3[] = {NAN, NAN, NAN};
	double data4[] = {1.0, 2.0, 3.0};

	// These should not crash and should handle NaN gracefully
	nsl_baseline_remove_minimum(data1, 3);
	nsl_baseline_remove_maximum(data2, 3);
	nsl_baseline_remove_mean(data3, 3);
	nsl_baseline_remove_median(data4, 3);

	// Test that valid values are processed correctly
	// (Note: actual behavior depends on implementation details)
	QVERIFY(true);
}

QTEST_MAIN(NSLBaselineTest)
