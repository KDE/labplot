/*
	File                 : PiecewiseLinearFitTest.h
	Project              : LabPlot
	Description          : Tests for piecewise linear regression
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PIECEWISELINEARFITTEST_H
#define PIECEWISELINEARFITTEST_H

#include "../../CommonTest.h"

class PiecewiseLinearFitTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	// Basic functionality
	void testTwoSegments();
	void testThreeSegments();
	void testNoChangepoint();

	// Algorithm comparison
	void testBinarySegmentationVsPELT();

	// Edge cases
	void testMinimumData();
	void testConstantData();
	void testNaNHandling();

	// Penalty parameter
	void testLowPenalty();
	void testHighPenalty();

	// Connection types
	void testContinuousFit();
	void testDiscontinuousFit();

	// Statistical validation
	void testRsquare();
	void testSSE();
};

#endif // PIECEWISELINEARFITTEST_H
