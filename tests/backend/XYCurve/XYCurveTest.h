/*
	File                 : XYCurveTest.h
	Project              : LabPlot
	Description          : Tests for XYCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYCURVETEST_H
#define XYCURVETEST_H

#include "../../CommonTest.h"

class XYCurveTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:

	void addUniqueLineTest01();

	// lineSkipGaps = false
	void updateLinesNoGapDirectConnection();
	void updateLinesNoGapStartHorizontal();
	void updateLinesNoGapStartVertical();
	void updateLinesNoGapMidPointHorizontal();
	void updateLinesNoGapMidPointVertical();
	void updateLinesNoGapSegments2();
	void updateLinesNoGapSegments3();

	// lineSkipGaps = false
	void updateLinesNoGapDirectConnectionLastVertical();
	void updateLinesNoGapStartHorizontalLastVertical();
	void updateLinesNoGapStartVerticalLastVertical();
	void updateLinesNoGapMidPointHorizontalLastVertical();
	void updateLinesNoGapMidPointVerticalLastVertical();
	void updateLinesNoGapSegments2LastVertical();
	void updateLinesNoGapSegments3LastVertical();

	// lineSkipGaps = true
	void updateLinesWithGapLineSkipDirectConnection();
	void updateLinesWithGapLineSkipDirectConnection2();
	void updateLinesWithGapLineSkipStartHorizontal();
	void updateLinesWithGapLineSkipStartVertical();
	void updateLinesWithGapLineSkipMidPointHorizontal();
	void updateLinesWithGapLineSkipMidPointVertical();
	void updateLinesWithGapLineSkipSegments2();
	void updateLinesWithGapLineSkipSegments3();

	// lineSkipGaps = false
	void updateLinesWithGapDirectConnection();
	void updateLinesWithGapStartHorizontal();
	void updateLinesWithGapStartVertical();
	void updateLinesWithGapMidPointHorizontal();
	void updateLinesWithGapMidPointVertical();
	void updateLinesWithGapSegments2();
	void updateLinesWithGapSegments3();
};

#endif // XYCURVETEST_H
