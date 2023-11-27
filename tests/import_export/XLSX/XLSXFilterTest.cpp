/*
	File                 : XLSXFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the XLSX filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XLSXFilterTest.h"
#include "backend/datasources/filters/XLSXFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

void XLSXFilterTest::importFile2Cols() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/2col.xlsx"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	XLSXFilter filter;
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

void XLSXFilterTest::importFile3Cols() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/3col.xlsx"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	XLSXFilter filter;
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

void XLSXFilterTest::importFile3ColsStartEndRow() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/3col.xlsx"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	XLSXFilter filter;
	filter.setCurrentSheet(QStringLiteral("Sheet1"));
	filter.setCurrentRange(QStringLiteral("A1:C5"));
	// set start/end row and check result
	filter.setStartRow(2);
	filter.setEndRow(3);
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 2);

	// WARN(spreadsheet.column(0)->valueAt(0))
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 2.2);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 3.3);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 16);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 9);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 20);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 300);

	// set end row too high
	filter.setCurrentRange(QStringLiteral("A1:C5"));
	filter.setStartRow(2);
	filter.setEndRow(8);
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 4);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 2.2);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 3.3);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 4.4);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 5.5);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 16);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 9);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 4);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 1);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 20);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 300);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 4000);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 50000);

	// set start row too high
	filter.setCurrentRange(QStringLiteral("A1:C5"));
	filter.setStartRow(8);
	filter.setEndRow(3);
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 3);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1.1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 2.2);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 3.3);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 25);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 16);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 9);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 20);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 300);
}

void XLSXFilterTest::importFile3ColsStartEndColumn() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/3col.xlsx"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	XLSXFilter filter;
	filter.setCurrentSheet(QStringLiteral("Sheet1"));
	filter.setCurrentRange(QStringLiteral("A1:C5"));
	// set start/end row and check result
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 5);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 25);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 16);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 9);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 4);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 20);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 300);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 4000);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 50000);

	// set end column too high
	filter.setCurrentRange(QStringLiteral("A1:C5"));
	filter.setStartColumn(2);
	filter.setEndColumn(7);
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 5);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 25);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 16);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 9);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 4);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 20);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 300);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 4000);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 50000);

	// set start column too high
	filter.setCurrentRange(QStringLiteral("A1:C5"));
	filter.setStartColumn(6);
	filter.setEndColumn(2);
	filter.readDataFromFile(fileName, &spreadsheet);

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
}

void XLSXFilterTest::importFileEmptyCells() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/datatypes-empty.xlsx"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	XLSXFilter filter;
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

void XLSXFilterTest::importFileDatetime() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/datatypes-excel.xlsx"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	XLSXFilter filter;
	filter.setCurrentSheet(QStringLiteral("Sheet1"));
	filter.setCurrentRange(QStringLiteral("A1:E4"));
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 5);
	QCOMPARE(spreadsheet.rowCount(), 4);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 4);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 3);
	QCOMPARE(spreadsheet.column(1)->dateTimeAt(0).toString(), QStringLiteral("Sun Jan 1 00:00:00 2023"));
	QCOMPARE(spreadsheet.column(1)->dateTimeAt(1).toString(), QStringLiteral("Thu Mar 2 00:00:00 2023"));
	QCOMPARE(spreadsheet.column(1)->dateTimeAt(2).toString(), QStringLiteral("Sun Jun 4 00:00:00 2023"));
	QCOMPARE(spreadsheet.column(1)->dateTimeAt(3).toString(), QStringLiteral("Mon Aug 7 00:00:00 2023"));
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 1.1);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 2.3);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 4.2);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 7.4);
	QCOMPARE(spreadsheet.column(3)->dateTimeAt(0).toString(), QStringLiteral("Sat Nov 11 01:02:03 2023"));
	QCOMPARE(spreadsheet.column(3)->dateTimeAt(1).toString(), QStringLiteral("Thu Mar 27 03:17:24 2014"));
	QCOMPARE(spreadsheet.column(3)->dateTimeAt(2).toString(), QStringLiteral("Sun Sep 19 12:12:12 1999"));
	QCOMPARE(spreadsheet.column(3)->dateTimeAt(3).toString(), QStringLiteral("Mon Aug 8 23:23:23 1988"));
	QCOMPARE(spreadsheet.column(4)->valueAt(0), 2.5);
	QCOMPARE(spreadsheet.column(4)->valueAt(1), 3.14);
	QCOMPARE(spreadsheet.column(4)->valueAt(2), 0.22);
	QCOMPARE(spreadsheet.column(4)->valueAt(3), 0.01);
}

QTEST_MAIN(XLSXFilterTest)
