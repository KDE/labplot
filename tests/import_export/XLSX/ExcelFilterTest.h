/*
	File                 : ExcelFilterTest.h
	Project              : LabPlot
	Description          : Tests for the Excel filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef EXCELFILTERTEST_H
#define EXCELFILTERTEST_H

#include "../../CommonTest.h"
#include <QtTest>

class ExcelFilterTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void importFile1();
	void importFile2();
};
#endif
