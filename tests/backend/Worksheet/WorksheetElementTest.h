/*
	File                 : WorksheetTest.h
	Project              : LabPlot
	Description          : Tests for Worksheets and positioning them on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETELEMENTTEST_H
#define WORKSHEETELEMENTTEST_H

#include "../../CommonTest.h"

class WorksheetElementTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void customPointSetPositionLogical();
	void customPointMouseMove();
	void customPointKeyPressMoveRight();
	void customPointKeyPressMoveLeft();
	void customPointKeyPressMoveUp();
	void customPointKeyPressMoveDown();
	void customPointEnableDisableCoordBinding();

    void referenceRangeXMouseMove();
};

#endif // WORKSHEETELEMENTTEST_H
