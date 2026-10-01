/*
	File                 : NSLInterpolationTest.cpp
	Project              : LabPlot
	Description          : Tests for NSL rational interpolation
	--------------------------------------------------------------------
		SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLInterpolationTest.h"

#include <cmath>

extern "C" {
#include "backend/nsl/nsl_interp.h"
}

void NSLInterpolationTest::testRationalInterpolation() {
	const double x[] = {0, 1};
	const double y[] = {1, 3};
	double value = 0.;
	double error = 0.;
	QCOMPARE(nsl_interp_ratint(x, y, 2, 1., &value, &error), 1);
	QCOMPARE(value, 3.);
	QCOMPARE(error, 0.);
	QCOMPARE(nsl_interp_ratint(x, y, 2, 0.25, &value, &error), 0);
	QVERIFY(std::abs(value - 1.2) < 1.e-12);
	QVERIFY(std::abs(error - 0.2) < 1.e-12);
}

QTEST_MAIN(NSLInterpolationTest)