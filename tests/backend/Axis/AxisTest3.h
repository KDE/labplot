/*
	File                 : AxisTest3.h
	Project              : LabPlot
	Description          : More tests for Axis methods
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AXISTEST3_H
#define AXISTEST3_H

#include "../../CommonTest.h"

class AxisTest3 : public CommonTest {
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

	void autoScaleLog10();
	void autoScaleLog102();
};

#endif // AXISTEST3_H
