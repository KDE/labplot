/*
	File                 : NSLMathTest.cpp
	Project              : LabPlot
	Description          : NSL tests for math functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLMathTest.h"
#include <cmath>
#include <limits>

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

void NSLMathTest::testNaNInfHandling() {
	double nan = std::numeric_limits<double>::quiet_NaN();
	double inf = std::numeric_limits<double>::infinity();

	// Test approximately_equal with NaN
	QVERIFY(!nsl_math_approximately_equal(nan, 1.0));
	QVERIFY(!nsl_math_approximately_equal(1.0, nan));
	QVERIFY(!nsl_math_approximately_equal_eps(nan, 1.0, 1.e-7));

	// Test approximately_equal with Inf
	QVERIFY(!nsl_math_approximately_equal(inf, 1.0));
	QVERIFY(!nsl_math_approximately_equal_eps(inf, inf, 1.e-7));

	// Test essentially_equal with NaN
	QVERIFY(!nsl_math_essentially_equal(nan, 1.0));
	QVERIFY(!nsl_math_essentially_equal(1.0, nan));
	QVERIFY(!nsl_math_essentially_equal_eps(nan, 1.0, 1.e-7));

	// Test essentially_equal with Inf
	QVERIFY(!nsl_math_essentially_equal(inf, 1.0));
	QVERIFY(!nsl_math_essentially_equal_eps(inf, inf, 1.e-7));

	// Test definitely_greater_than with NaN
	QVERIFY(!nsl_math_definitely_greater_than(nan, 1.0));
	QVERIFY(!nsl_math_definitely_greater_than(1.0, nan));
	QVERIFY(!nsl_math_definitely_greater_than_eps(nan, 1.0, 1.e-7));

	// Test definitely_greater_than with Inf
	QVERIFY(!nsl_math_definitely_greater_than(inf, inf));
	QVERIFY(!nsl_math_definitely_greater_than_eps(inf, 1.0, 1.e-7));

	// Test definitely_less_than with NaN
	QVERIFY(!nsl_math_definitely_less_than(nan, 1.0));
	QVERIFY(!nsl_math_definitely_less_than(1.0, nan));
	QVERIFY(!nsl_math_definitely_less_than_eps(nan, 1.0, 1.e-7));

	// Test definitely_less_than with Inf
	QVERIFY(!nsl_math_definitely_less_than(1.0, inf));
	QVERIFY(!nsl_math_definitely_less_than_eps(inf, 1.0, 1.e-7));

	// Test frexp10 with NaN
	int exponent = 0;
	double result = nsl_math_frexp10(nan, &exponent);
	QVERIFY(std::isnan(result));

	// Test frexp10 with Inf
	result = nsl_math_frexp10(inf, &exponent);
	QVERIFY(std::isnan(result));

	// Test decimal_places with NaN
	int places = nsl_math_decimal_places(nan);
	QCOMPARE(places, 0);

	// Test decimal_places with Inf
	places = nsl_math_decimal_places(inf);
	QCOMPARE(places, 0);

	// Test rounded_decimals with NaN
	places = nsl_math_rounded_decimals(nan);
	QCOMPARE(places, 0);

	// Test rounded_decimals with Inf
	places = nsl_math_rounded_decimals(inf);
	QCOMPARE(places, 0);

	// Test rounded_decimals_max with NaN
	places = nsl_math_rounded_decimals_max(nan, 5);
	QCOMPARE(places, 0);

	// Test *_places functions with NaN
	result = nsl_math_round_places(nan, 2);
	QVERIFY(std::isnan(result));
	result = nsl_math_floor_places(nan, 2);
	QVERIFY(std::isnan(result));
	result = nsl_math_ceil_places(nan, 2);
	QVERIFY(std::isnan(result));
	result = nsl_math_trunc_places(nan, 2);
	QVERIFY(std::isnan(result));

	// Test *_places functions with Inf
	result = nsl_math_round_places(inf, 2);
	QVERIFY(std::isnan(result));
	result = nsl_math_floor_places(inf, 2);
	QVERIFY(std::isnan(result));

	// Test valid inputs still work
	QVERIFY(nsl_math_approximately_equal(1.0, 1.0));
	QVERIFY(nsl_math_definitely_greater_than(2.0, 1.0));
	QVERIFY(nsl_math_definitely_less_than(1.0, 2.0));
	result = nsl_math_frexp10(100.0, &exponent);
	QCOMPARE(exponent, 2);
}

// ##############################################################################
// #################  performance
// ##############################################################################

QTEST_MAIN(NSLMathTest)
