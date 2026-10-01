/*
	File                 : NSLSFAdvancedTest.h
	Project              : LabPlot
	Description          : Tests for additional NSL special functions
	--------------------------------------------------------------------
		SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLSFADVANCEDTEST_H
#define NSLSFADVANCEDTEST_H

#include "../NSLTest.h"

class NSLSFAdvancedTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void testKernelFunctions();
	void testPolynomialFunctions();
	void testDistributionMetadata();
	void testTriangularRandomDistribution();
};

#endif