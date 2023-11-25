/*
	File                 : WorksheetElementTest.h
	Project              : LabPlot
	Description          : Tests for WorksheetElements in the graphical representation sense
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETELEMENTTEST_H
#define WORKSHEETELEMENTTEST_H

#include "../../CommonTest.h"

class AxisTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void axisLine();
	void majorTicksAutoNumberEnableDisable();
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

	void columnLabelValues();
	void columnLabelValuesMaxValues();

	void customTextLabels();

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
	void customColumnDateTime();

	void autoScale();
	void autoScale2();
};

#endif // WORKSHEETELEMENTTEST_H
