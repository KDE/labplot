/*
	File                 : NSLPCMTest.cpp
	Project              : LabPlot
	Description          : Tests for NSL process-control factors
	--------------------------------------------------------------------
		SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLPCMTest.h"

#include <cmath>

extern "C" {
#include "backend/nsl/nsl_pcm.h"
}

void NSLPCMTest::testFactors() {
	const double d2 = nsl_pcm_d2(2);
	const double d3 = nsl_pcm_d3(2);
	QVERIFY(std::abs(d2 - 1.128379167095197) < 1.e-12);
	QVERIFY(std::abs(nsl_pcm_D3(2) - (1. - 3. * d3 / d2)) < 1.e-12);
	QVERIFY(std::abs(nsl_pcm_D4(2) - (1. + 3. * d3 / d2)) < 1.e-12);

	const double factors[] = {nsl_pcm_D5(5),
							  nsl_pcm_D6(5),
							  nsl_pcm_A2(5),
							  nsl_pcm_A3(5),
							  nsl_pcm_A4(5),
							  nsl_pcm_B3(5),
							  nsl_pcm_B4(5),
							  nsl_pcm_B5(5),
							  nsl_pcm_B6(5),
							  nsl_pcm_d3(5),
							  nsl_pcm_d4(5),
							  nsl_pcm_c4(5)};
	for (double value : factors)
		QVERIFY(std::isfinite(value));
	QCOMPARE(nsl_pcm_d2(101), 0.);
	QCOMPARE(nsl_pcm_D3(101), 0.);
}

QTEST_MAIN(NSLPCMTest)