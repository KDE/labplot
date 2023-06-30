/*
	File                 : ExcelFilterTest.h
	Project              : LabPlot
	Description          : Tests for the Excel filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef EXCELFILTERTEST_H
#define EXCELFILTERTEST_H

#include "../../CommonTest.h"
#include <QtTest>

class ExcelFilterTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void importFile2Cols();
	void importFile3Cols();
	void importFileEmptyCells();
};
#endif
