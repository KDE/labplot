/*
	File                 : AxisTest2.h
	Project              : LabPlot
	Description          : More tests for Axis methods
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
};

#endif // AXISTEST2_H
