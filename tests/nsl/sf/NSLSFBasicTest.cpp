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
// #################  RNG distribution tests
// ##############################################################################

void NSLSFBasicTest::test_triangular() {
	// out of bounds
	QCOMPARE(nsl_sf_triangular(1., 1., 1., 1.), 0.);
	QCOMPARE(nsl_sf_triangular(1., 2., 1., 1.), 0.);
	QCOMPARE(nsl_sf_triangular(1.5, 1., 2., 0.), 0.);
	QCOMPARE(nsl_sf_triangular(1.5, 1., 2., 3.), 0.);
	QCOMPARE(nsl_sf_triangular(0., 1., 2., 1.5), 0.);
	QCOMPARE(nsl_sf_triangular(3., 1., 2., 1.5), 0.);

	// values
	QCOMPARE(nsl_sf_triangular(1.5, 1., 3., 2.), 0.5);
	QCOMPARE(nsl_sf_triangular(2.5, 1., 3., 2.), 0.5);

	const int nrPoints = 1e6;
	QBENCHMARK {
		for (int i = 0; i < nrPoints; i++)
			nsl_sf_triangular(3. * i / (double)nrPoints, 1., 2., 1.5);
	}
}
void NSLSFBasicTest::test_triangular_P() {
	// out of bounds
	QCOMPARE(nsl_sf_triangular_P(1., 1., 1., 1.), 0.);
	QCOMPARE(nsl_sf_triangular_P(1., 2., 1., 1.), 0.);
	QCOMPARE(nsl_sf_triangular_P(0., 1., 2., 1.5), 0.);
	QCOMPARE(nsl_sf_triangular_P(3., 1., 2., 1.5), 1.);

	// values
	QCOMPARE(nsl_sf_triangular_P(1.5, 1., 3., 2.), 0.125);
	QCOMPARE(nsl_sf_triangular_P(2.5, 1., 3., 2.), 0.875);

	const int nrPoints = 1e6;
	QBENCHMARK {
		for (int i = 0; i < nrPoints; i++)
			nsl_sf_triangular_P(3. * i / (double)nrPoints, 1., 2., 1.5);
	}
}
void NSLSFBasicTest::test_triangular_Q() {
	// out of bounds
	QCOMPARE(nsl_sf_triangular_Q(1., 1., 1., 1.), 1.);
	QCOMPARE(nsl_sf_triangular_Q(1., 2., 1., 1.), 1.);
	QCOMPARE(nsl_sf_triangular_Q(0., 1., 2., 1.5), 1.);
	QCOMPARE(nsl_sf_triangular_Q(3., 1., 2., 1.5), 0.);

	// values
	QCOMPARE(nsl_sf_triangular_Q(1.5, 1., 3., 2.), 0.875);
	QCOMPARE(nsl_sf_triangular_Q(2.5, 1., 3., 2.), 0.125);

	const int nrPoints = 1e6;
	QBENCHMARK {
		for (int i = 0; i < nrPoints; i++)
			nsl_sf_triangular_Q(3. * i / (double)nrPoints, 1., 2., 1.5);
	}
}
void NSLSFBasicTest::test_triangular_Quantile() {
	// out of bounds
	QCOMPARE(nsl_sf_triangular_Quantile(0., 1., 1., 1.), 1.);
	QCOMPARE(nsl_sf_triangular_Quantile(1., 1., 2., 1.), 2.);

	WARN(nsl_sf_triangular_Quantile(0.25, 1., 3., 2.))
	WARN(nsl_sf_triangular_Quantile(0.25, 1., 4., 3.))
	WARN(nsl_sf_triangular_Quantile(0.5, 1., 3., 2.))
	WARN(nsl_sf_triangular_Quantile(0.75, 1., 3., 2.))

	// values
	QCOMPARE(nsl_sf_triangular_Quantile(0.25, 1., 3., 2.), 1.707106781186547);
	QCOMPARE(nsl_sf_triangular_Quantile(0.25, 1., 4., 3.), 2.224744871391589);
	QCOMPARE(nsl_sf_triangular_Quantile(0.5, 1., 3., 2.), 2.);
	QCOMPARE(nsl_sf_triangular_Quantile(0.75, 1., 3., 2.), 2.292893218813453);

	const int nrPoints = 1e6;
	QBENCHMARK {
		for (int i = 0; i < nrPoints; i++)
			nsl_sf_triangular_Quantile(i / (double)nrPoints, 1., 2., 1.5);
	}
}
void NSLSFBasicTest::testran_triangular() {
	// out of bounds
	QCOMPARE(nsl_sf_ran_triangular(1., 1., 1.), 0.);
	QCOMPARE(nsl_sf_ran_triangular(2., 1., 1.), 0.);
	QCOMPARE(nsl_sf_ran_triangular(1., 2., 0.), 0.);
	QCOMPARE(nsl_sf_ran_triangular(1., 2., 3.), 0.);

	const int nrPoints = 1e3;
	QBENCHMARK {
		for (int i = 0; i < nrPoints; i++)
			nsl_sf_ran_triangular(0., 1., 0.5);
	}
}

// ##############################################################################
// #################  log2() tests
// ##############################################################################

void NSLSFBasicTest::testlog2_int_C99() {
	QBENCHMARK {
		for (unsigned int i = 1; i < 4e9; i++)
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
