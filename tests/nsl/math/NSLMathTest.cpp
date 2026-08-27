/*
	File                 : NSLMathTest.cpp
	Project              : LabPlot
	Description          : NSL tests for math functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLMathTest.h"

extern "C" {
#include "backend/nsl/nsl_math.h"
}

void NSLMathTest::mathMultiple() {
	// (2.5, 2.)	-> 2.,2.,4.,2.
	double value = 2.5, multiple = 2.;
	QCOMPARE(nsl_math_round_multiple(value, multiple), 2.);
	QCOMPARE(nsl_math_floor_multiple(value, multiple), 2.);
	QCOMPARE(nsl_math_ceil_multiple(value, multiple), 4.);
	QCOMPARE(nsl_math_trunc_multiple(value, multiple), 2.);

	// (4.5, 3)	-> 6.,3.,6.,3.
	value = 4.5, multiple = 3.;
	QCOMPARE(nsl_math_round_multiple(value, multiple), 6.);
	QCOMPARE(nsl_math_floor_multiple(value, multiple), 3.);
	QCOMPARE(nsl_math_ceil_multiple(value, multiple), 6.);
	QCOMPARE(nsl_math_trunc_multiple(value, multiple), 3.);

	// (-0.25, 1)	-> 0.,0.,-1.,0.
	value = -0.25, multiple = 1.;
	QCOMPARE(nsl_math_round_multiple(value, multiple), 0.);
	QCOMPARE(nsl_math_floor_multiple(value, multiple), -1.);
	QCOMPARE(nsl_math_ceil_multiple(value, multiple), 0.);
	QCOMPARE(nsl_math_trunc_multiple(value, multiple), 0.);
}

void NSLMathTest::approximatelyEqual() {
	QVERIFY(nsl_math_approximately_equal_eps(0., 2.8e-17, 1.e-12));
	QVERIFY(nsl_math_approximately_equal_eps(100., 100.00005, 1.e-6));
	QVERIFY(!nsl_math_approximately_equal_eps(0., 2.e-6, 1.e-7));
}

// ##############################################################################
// #################  performance
// ##############################################################################

QTEST_MAIN(NSLMathTest)
