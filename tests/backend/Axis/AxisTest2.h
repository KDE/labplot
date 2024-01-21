/*
	File                 : AxisTest2.h
	Project              : LabPlot
	Description          : Second tests for Axis methods
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AXISTEST2_H
#define AXISTEST2_H

#include "../../CommonTest.h"

class AxisTest2 : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
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

	void autoScale();
	void autoScale2();
};

#endif // AXISTEST2_H
