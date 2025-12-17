/*
	File                 : XYFunctionCurveTest.h
	Project              : LabPlot
	Description          : Tests for XYFunctionCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYFUNCTIONCURVETEST_H
#define XYFUNCTIONCURVETEST_H

#include "../../CommonTest.h"

class XYFunctionCurveTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void setCurves();
	void removeCurves();
	void removeColumnFromCurve();
	void removeCurveRenameAutomaticAdd();
	void saveLoad();
	void importData();
	void importDataComplexDependency();
};

#endif // XYFUNCTIONCURVETEST_H
