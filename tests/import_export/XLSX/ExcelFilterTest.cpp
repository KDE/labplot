/*
	File                 : ExcelFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the Excel filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ExcelFilterTest.h"
#include "backend/datasources/filters/ExcelFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

void ExcelFilterTest::importFile2Cols() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/2col.xlsx"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ExcelFilter filter;
	filter.setCurrentSheet(QStringLiteral("Sheet1"));
	filter.setCurrentRange(QStringLiteral("A1:B5"));
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 5);

	// WARN(spreadsheet.column(0)->valueAt(0))
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

void ExcelFilterTest::importFile3Cols() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/3col.xlsx"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ExcelFilter filter;
	filter.setCurrentSheet(QStringLiteral("Sheet1"));
	filter.setCurrentRange(QStringLiteral("A1:C5"));
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 5);

	// WARN(spreadsheet.column(0)->valueAt(0))
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1.1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 2.2);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 3.3);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 4.4);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 5.5);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 25);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 16);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 9);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 4);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 1);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 20);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 300);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 4000);
	QCOMPARE(spreadsheet.column(2)->valueAt(4), 50000);
}

void ExcelFilterTest::importFileEmptyCells() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/datatypes-empty.xlsx"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ExcelFilter filter;
	filter.setCurrentSheet(QStringLiteral("Sheet1"));
	filter.setCurrentRange(QStringLiteral("A1:F5"));
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 6);
	QCOMPARE(spreadsheet.rowCount(), 5);

	WARN(spreadsheet.column(0)->valueAt(0))
	WARN(spreadsheet.column(0)->valueAt(1))
	WARN(spreadsheet.column(0)->valueAt(2))
	WARN(spreadsheet.column(0)->valueAt(3))
	WARN(spreadsheet.column(0)->valueAt(4))
	WARN(spreadsheet.column(1)->valueAt(0))
	WARN(spreadsheet.column(2)->valueAt(0))
	WARN(spreadsheet.column(3)->valueAt(0))
	WARN(spreadsheet.column(4)->valueAt(0))
	WARN(spreadsheet.column(5)->valueAt(0))
	//	QCOMPARE(spreadsheet.column(0)->valueAt(0), 45107.7371295949);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 10.5);
	//	QCOMPARE(spreadsheet.column(0)->valueAt(2), 43747);
	//	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0.422974537037037);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 40000);
	//	QCOMPARE(spreadsheet.column(1)->valueAt(0), 45107.7371180556);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 10.5);
	//	QCOMPARE(spreadsheet.column(3)->valueAt(0), 43747);
	//	QCOMPARE(spreadsheet.column(4)->valueAt(0), 0.422974537037037);
	QCOMPARE(spreadsheet.column(5)->valueAt(0), 40000);

	for (int col = 1; col < 6; col++)
		for (int row = 1; row < 5; row++)
			QCOMPARE(spreadsheet.column(col)->valueAt(row), 0);
}

QTEST_MAIN(ExcelFilterTest)
