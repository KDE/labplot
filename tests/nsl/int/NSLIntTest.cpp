/*
	File                 : NSLIntTest.cpp
	Project              : LabPlot
	Description          : NSL Tests for numerical integration
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLIntTest.h"
#include <limits>
#include <cmath>

extern "C" {
#include "backend/nsl/nsl_int.h"
}

// ##############################################################################
// #################  rule integral/area tests
// ##############################################################################

const int N = 5;
double rxdata[] = {1, 2, 3, 5, 7};

void NSLIntTest::testRectangle_integral() {
	double ydata[] = {2, 2, 2, -2, -2};

	int status = nsl_int_rectangle(rxdata, ydata, N, 0);
	QCOMPARE(status, 0);
	QCOMPARE(ydata[N - 1], 4.);
}

void NSLIntTest::testRectangle_area() {
	double ydata[] = {2, 2, 2, -2, -2};

	int status = nsl_int_rectangle(rxdata, ydata, N, 1);
	QCOMPARE(status, 0);
	QCOMPARE(ydata[N - 1], 12.);
}

void NSLIntTest::testTrapezoid_integral() {
	double ydata[] = {1, 2, 3, -1, -3};

	int status = nsl_int_trapezoid(rxdata, ydata, N, 0);
	QCOMPARE(status, 0);
	QCOMPARE(ydata[N - 1], 2.);
}

void NSLIntTest::testTrapezoid_area() {
	double ydata[] = {1, 2, 3, -1, -3};

	int status = nsl_int_trapezoid(rxdata, ydata, N, 1);
	QCOMPARE(status, 0);
	QCOMPARE(ydata[N - 1], 10.5);
}

void NSLIntTest::test3Point_integral() {
	double ydata[] = {1, 2, 3, -1, -3};

	int np = (int)nsl_int_simpson(rxdata, ydata, N, 0);
	QCOMPARE(np, 3);
	QCOMPARE(ydata[np - 1], 4 / 3.);
}

void NSLIntTest::test4Point_integral() {
	double xdata2[] = {1, 2, 3, 5, 7, 8, 9};
	double ydata[] = {2, 2, 2, 2, 2, 2, 2, 2};
	const int n = 7;

	int np = (int)nsl_int_simpson_3_8(xdata2, ydata, n, 0);
	QCOMPARE(np, 3);
	QCOMPARE(ydata[np - 1], 16.);
}

// ##############################################################################
// #################  performance
// ##############################################################################

void NSLIntTest::testPerformanceRectangle() {
	const int n = 1e6;
	QScopedArrayPointer<double> xdata(new double[n]);
	QScopedArrayPointer<double> ydata(new double[n]);

	for (int i = 0; i < n; i++)
		xdata[i] = (double)i;

	QBENCHMARK {
		for (int i = 0; i < n; i++)
			ydata[i] = 1.;
		int status = nsl_int_rectangle(xdata.data(), ydata.data(), n, 0);
		QCOMPARE(status, 0);
	}
	QCOMPARE(ydata[n - 1], (double)(n - 1));
}

void NSLIntTest::testPerformanceTrapezoid() {
	const int n = 1e6;
	QScopedArrayPointer<double> xdata(new double[n]);
	QScopedArrayPointer<double> ydata(new double[n]);

	for (int i = 0; i < n; i++)
		xdata[i] = (double)i;

	QBENCHMARK {
		for (int i = 0; i < n; i++)
			ydata[i] = 1.;
		int status = nsl_int_trapezoid(xdata.data(), ydata.data(), n, 0);
		QCOMPARE(status, 0);
	}
	QCOMPARE(ydata[n - 1], (double)(n - 1));
}

void NSLIntTest::testPerformance3Point() {
	const int n = 1e6;
	QScopedArrayPointer<double> xdata(new double[n]);
	QScopedArrayPointer<double> ydata(new double[n]);

	for (int i = 0; i < n; i++)
		xdata[i] = (double)i;

	int np = 1;
	QBENCHMARK {
		for (int i = 0; i < n; i++)
			ydata[i] = 1.;
		np = (int)nsl_int_simpson(xdata.data(), ydata.data(), n, 0);
		QCOMPARE(np, n / 2 + 1);
	}
	QCOMPARE(ydata[np - 1], (double)(n - 1));
}

void NSLIntTest::testPerformance4Point() {
	const int n = 1e6;
	QScopedArrayPointer<double> xdata(new double[n]);
	QScopedArrayPointer<double> ydata(new double[n]);

	for (int i = 0; i < n; i++)
		xdata[i] = (double)i;

	int np = 1;
	QBENCHMARK {
		for (int i = 0; i < n; i++)
			ydata[i] = 1.;
		np = (int)nsl_int_simpson_3_8(xdata.data(), ydata.data(), n, 0);
		QCOMPARE(np, n / 3 + 1);
	}

	// TODO:
	// QCOMPARE(ydata[np - 1], (double)(n - 1));
	printf("%.15g %.15g\n", ydata[np - 1], (double)(n - 1));
}

void NSLIntTest::testNaNInfHandling() {
	const int n = 5;
	QScopedArrayPointer<double> xdata(new double[n]);
	QScopedArrayPointer<double> ydata(new double[n]);
	double nan = std::numeric_limits<double>::quiet_NaN();
	double inf = std::numeric_limits<double>::infinity();

	// Test rectangle with finite data first
	for (int i = 0; i < n; i++) {
		xdata[i] = (double)i;
		ydata[i] = 1.0;
	}
	int result = nsl_int_rectangle(xdata.data(), ydata.data(), n, 0);
	QCOMPARE(result, 0);

	// Test rectangle with NaN in x
	for (int i = 0; i < n; i++) {
		xdata[i] = (double)i;
		ydata[i] = 1.0;
	}
	xdata[2] = nan;
	result = nsl_int_rectangle(xdata.data(), ydata.data(), n, 0);
	QCOMPARE(result, -1);

	// Test rectangle with Inf in y
	for (int i = 0; i < n; i++) {
		xdata[i] = (double)i;
		ydata[i] = 1.0;
	}
	ydata[3] = inf;
	result = nsl_int_rectangle(xdata.data(), ydata.data(), n, 0);
	QCOMPARE(result, -1);

	// Test trapezoid with NaN
	for (int i = 0; i < n; i++) {
		xdata[i] = (double)i;
		ydata[i] = 1.0;
	}
	ydata[1] = nan;
	result = nsl_int_trapezoid(xdata.data(), ydata.data(), n, 0);
	QCOMPARE(result, -1);

	// Test trapezoid with Inf
	for (int i = 0; i < n; i++) {
		xdata[i] = (double)i;
		ydata[i] = 1.0;
	}
	xdata[0] = inf;
	result = nsl_int_trapezoid(xdata.data(), ydata.data(), n, 0);
	QCOMPARE(result, -1);

	// Test Simpson with NaN
	for (int i = 0; i < n; i++) {
		xdata[i] = (double)i;
		ydata[i] = 1.0;
	}
	ydata[2] = nan;
	size_t sresult = nsl_int_simpson(xdata.data(), ydata.data(), n, 0);
	QCOMPARE(sresult, 0uL);

	// Test Simpson 3/8 with Inf
	for (int i = 0; i < n; i++) {
		xdata[i] = (double)i;
		ydata[i] = 1.0;
	}
	xdata[3] = inf;
	sresult = nsl_int_simpson_3_8(xdata.data(), ydata.data(), n, 0);
	QCOMPARE(sresult, 0uL);

	// Test NULL pointers
	result = nsl_int_rectangle(NULL, ydata.data(), n, 0);
	QCOMPARE(result, -1);
	result = nsl_int_trapezoid(xdata.data(), NULL, n, 0);
	QCOMPARE(result, -1);
}

QTEST_MAIN(NSLIntTest)
