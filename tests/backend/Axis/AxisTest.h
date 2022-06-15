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
	void majorTicksAutoNumberEnableDisable();
	void minorTicksAutoNumberEnableDisable();
};

#endif // WORKSHEETELEMENTTEST_H
