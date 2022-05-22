/*
	File                 : ExcelFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the Excel filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ExcelFilterTest.h"
#include "backend/datasources/filters/ExcelFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

void ExcelFilterTest::importFile1() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/2col.xlsx"));

	Spreadsheet spreadsheet("test", false);
	ExcelFilter filter;
	filter.setCurrentSheet("Sheet1");
	filter.setCurrentRange("A1:B5");
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 5);

	//WARN(spreadsheet.column(0)->valueAt(0))
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 4);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 5);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 25);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 16);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 9);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 4);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 1);
}

QTEST_MAIN(ExcelFilterTest)
