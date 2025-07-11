/*
	File                 : WorksheetTest.h
	Project              : LabPlot
	Description          : Tests for Worksheets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETTEST_H
#define WORKSHEETTEST_H

#include "../../CommonTest.h"

class WorksheetTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void cursorCurveColor();
	void cursorNotAllPlotsVisible();
	void exportReplaceExtension();
	void zValueAfterAddMoveRemove();
};

#endif // WORKSHEETTEST_H
