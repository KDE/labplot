/*
	File                 : AxisTest.h
	Project              : LabPlot
	Description          : Tests for Axis methods
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AXISTEST_H
#define AXISTEST_H

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
};

#endif // AXISTEST_H
