/*
	File                 : NSLSFBasicTest.cpp
	Project              : LabPlot
	Description          : NSL Tests for basic special functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLSFBasicTest.h"

#include "backend/nsl/nsl_sf_basic.h"

// ##############################################################################
// #################  RNG tests
// ##############################################################################

void NSLSFBasicTest::testran_triangular() {
	// out of bounds
	QCOMPARE(nsl_sf_ran_triangular(1., 1., 1.), 0.);
	QCOMPARE(nsl_sf_ran_triangular(2., 1., 1.), 0.);
	QCOMPARE(nsl_sf_ran_triangular(1., 2., 0.), 0.);
	QCOMPARE(nsl_sf_ran_triangular(1., 2., 3.), 0.);

	QBENCHMARK {
		for (unsigned int i = 0; i < 1e4; i++)
			nsl_sf_ran_triangular(0., 1., 0.5);
	}
}

// ##############################################################################
// #################  log2() tests
// ##############################################################################
void NSLSFBasicTest::testlog2_int_C99() {
	QBENCHMARK {
		for (unsigned int i = 1; i < 1e7; i++)
			Q_UNUSED((int)log2(i));
	}
}

void NSLSFBasicTest::testlog2_int() {
	for (unsigned int i = 1; i < 1e5; i++) {
		int result = nsl_sf_log2_int(i);
		QCOMPARE(result, (int)log2(i));
	}

	QBENCHMARK {
		for (unsigned int i = 1; i < 1e7; i++)
			nsl_sf_log2_int(i);
	}
}
void NSLSFBasicTest::testlog2_longlong() {
#ifndef _MSC_VER /* not implemented yet */
	for (unsigned long long i = 1; i < 1e5; i++) {
		int result = nsl_sf_log2_longlong(i);
		QCOMPARE(result, (int)log2(i));
	}

	QBENCHMARK {
		for (unsigned long long i = 1; i < 1e7; i++)
			nsl_sf_log2_longlong(i);
	}
#endif
}

void NSLSFBasicTest::testlog2_int2() {
	for (int i = 1; i < 1e5; i++) {
		int result = nsl_sf_log2_int2(i);
		QCOMPARE(result, (int)log2(i));
	}

	QBENCHMARK {
		for (int i = 1; i < 1e7; i++)
			nsl_sf_log2_int2(i);
	}
}

void NSLSFBasicTest::testlog2_int3() {
	for (unsigned int i = 1; i < 1e5; i++) {
		int result = nsl_sf_log2_int3(i);
		QCOMPARE(result, (int)log2(i));
	}

	QBENCHMARK {
		for (unsigned int i = 1; i < 1e7; i++)
			nsl_sf_log2_int3(i);
	}
}

void NSLSFBasicTest::testlog2p1_int() {
	for (int i = 1; i < 1e5; i++) {
		int result = nsl_sf_log2p1_int(i);
		QCOMPARE(result, (int)log2(i) + 1);
	}

	QBENCHMARK {
		for (int i = 1; i < 1e7; i++)
			nsl_sf_log2p1_int(i);
	}
}

QTEST_MAIN(NSLSFBasicTest)
