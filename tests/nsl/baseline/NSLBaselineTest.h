/*
	File                 : NSLBaselineTest.h
	Project              : LabPlot
	Description          : NSL Tests for baseline functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLBASELINETEST_H
#define NSLBASELINETEST_H

#include "../NSLTest.h"

class NSLBaselineTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void testBaselineMinimum();
	void testBaselineMinimum2();
	void testBaselineMaximum();
	void testBaselineMaximum2();
	void testBaselineMean();
	void testBaselineMean2();
	void testBaselineMedian();
	void testBaselineMedian2();

	void testBaselineEndpoints();
	void testBaselineLinReg();

	void testBaselineARPLS();
	void testBaselineARPLSSpectrum();
	void testBaselineARPLS_XRD();
	void testBaselineARPLSEigen3();
	void testBaselineARPLSEigen3Spectrum();
	void testBaselineARPLSEigen3XRD();
	void testBaselineARPLSGSL();
	void testBaselineARPLSGSLSpectrum();
	void testBaselineARPLSGSLXRD();
	// performance
	// void testPerformance();

	void testBaselineEmptyData();
	void testBaselineSingleElement();
	void testBaselineNaNData();
};
#endif
