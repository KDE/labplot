/*
	File                 : XLSXFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the XLSX filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XLSXFilterTest.h"
#include "backend/core/column/Column.h"
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

void XLSXFilterTest::importFormula() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/formula.xlsx"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	XLSXFilter filter;
	filter.setCurrentSheet(QStringLiteral("Sheet1"));
	filter.setCurrentRange(QStringLiteral("A2:A13"));
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 12);

	spreadsheet.column(0)->setColumnMode(AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->integerAt(3), 4);
	QCOMPARE(spreadsheet.column(0)->integerAt(4), 5);
	QCOMPARE(spreadsheet.column(0)->integerAt(5), 6);
	QCOMPARE(spreadsheet.column(0)->integerAt(6), 7);
	QCOMPARE(spreadsheet.column(0)->integerAt(7), 8);
	QCOMPARE(spreadsheet.column(0)->integerAt(8), 9);
	QCOMPARE(spreadsheet.column(0)->integerAt(9), 10);
	QCOMPARE(spreadsheet.column(0)->integerAt(10), 11);
	QCOMPARE(spreadsheet.column(0)->integerAt(11), 12);

	filter.setCurrentRange(QStringLiteral("D2:D13"));
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 12);

	spreadsheet.column(0)->setColumnMode(AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(0)->textAt(0), QStringLiteral("January"));
	QCOMPARE(spreadsheet.column(0)->textAt(1), QStringLiteral("February"));
	QCOMPARE(spreadsheet.column(0)->textAt(2), QStringLiteral("March"));
	QCOMPARE(spreadsheet.column(0)->textAt(3), QStringLiteral("April"));
	QCOMPARE(spreadsheet.column(0)->textAt(4), QStringLiteral("May"));
	QCOMPARE(spreadsheet.column(0)->textAt(5), QStringLiteral("June"));
	QCOMPARE(spreadsheet.column(0)->textAt(6), QStringLiteral("July"));
	QCOMPARE(spreadsheet.column(0)->textAt(7), QStringLiteral("August"));
	QCOMPARE(spreadsheet.column(0)->textAt(8), QStringLiteral("September"));
	QCOMPARE(spreadsheet.column(0)->textAt(9), QStringLiteral("October"));
	QCOMPARE(spreadsheet.column(0)->textAt(10), QStringLiteral("November"));
	QCOMPARE(spreadsheet.column(0)->textAt(11), QStringLiteral("December"));

	filter.setCurrentRange(QStringLiteral("F2:F13"));
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 12);

	// DATE() function was used to generate this column but excel date and time functions all store their calculated values as integers
	// https://support.microsoft.com/en-us/office/date-and-time-functions-reference-fd1b5961-c1ae-4677-be58-074152f97b81
	spreadsheet.column(0)->setColumnMode(AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 45668);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 45669);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 45670);
	QCOMPARE(spreadsheet.column(0)->integerAt(3), 45671);
	QCOMPARE(spreadsheet.column(0)->integerAt(4), 45672);
	QCOMPARE(spreadsheet.column(0)->integerAt(5), 45673);
	QCOMPARE(spreadsheet.column(0)->integerAt(6), 45674);
	QCOMPARE(spreadsheet.column(0)->integerAt(7), 45675);
	QCOMPARE(spreadsheet.column(0)->integerAt(8), 45676);
	QCOMPARE(spreadsheet.column(0)->integerAt(9), 45677);
	QCOMPARE(spreadsheet.column(0)->integerAt(10), 45678);
	QCOMPARE(spreadsheet.column(0)->integerAt(11), 45679);
}

QTEST_MAIN(XLSXFilterTest)
