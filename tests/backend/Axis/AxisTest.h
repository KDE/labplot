/*
	File                 : AxisTest.h
	Project              : LabPlot
	Description          : Tests for Axis methods
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2022-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AXISTEST_H
#define AXISTEST_H

#include "../../CommonTest.h"

class AxisTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void axisLine();
	void minorTicksAutoNumberEnableDisable();
	void majorTicksStartValue();
	void TestSetCoordinateSystem();
	void TestSetRange();
	void TestAddingHorizontalAxis();
	void TestAddingVerticalAxis();
	void tickLabelRepresentationAutomatic();
	void tickLabelRepresentationManual();
	void setAxisColor(); // Set color of all elements
	void setTitleColor();
	void setMajorTickColor();
	void setMinorTickColor();
	void setLineColor();
	void setTickLabelColor();

	void automaticTicNumberUpdateDockMajorTicks();
	void automaticTicNumberUpdateDockMinorTicks();
	void tickSpacingUpdateDockMajorTicks();

	void testComputeMajorTickStart();

	void columnLabelValues();
	void columnLabelValuesMaxValues();
	void columnLabelValuesMoreTicksThanLabels();

	void customTextLabels();
	void customTextLabelsMoreTicksThanLabels();

	void dateTime();
	void dateTimeSpacing();
	void dateTimeSpacingOffsetNonZero();
	void dateTimeSpacingStartValueNonZero();

	void numeric();
	void numericSpacing();
	void numericSpacingOffsetNonZero();
	void numericSpacingStartValueNonZero();

	void customColumnNumeric();
	void customColumnNumericMaxValues();
	void customColumnNumericMaxValuesLimitedRange();
	void customColumnNumericMaxValuesLimitedRangeNotCompleteRange();
	void customColumnNonMonotonicColumnValues();
	void customColumnDateTime();

	void autoScaleLog10();
	void autoScaleLog102();
	void autoScaleLog102Vertical();

	void colorBar();

	void tickDrawingPosition(); // Check that ticks are positioned correctly without colorbar
	void tickDrawingPositionColorBar(); // Check that ticks are positioned correctly with colorbar
};

#endif // AXISTEST_H
