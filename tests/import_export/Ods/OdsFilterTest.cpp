/*
	File                 : OdsFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the Ods filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "OdsFilterTest.h"
#include "backend/datasources/filters/OdsFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

void OdsFilterTest::importFile3SheetsRangesFormula() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/ranges-formula.ods"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	OdsFilter filter;
	filter.setSelectedSheetNames(QStringList() << QStringLiteral("Sheet1"));
	// filter.setCurrentRange(QStringLiteral("A1:B5"));
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 5);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 4);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 5);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 2.1);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 3.21);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 4.321);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 5.4321);

	// TODO: check other sheets

	// WARN(spreadsheet.column(0)->valueAt(0))
	// QCOMPARE(spreadsheet.column(0)->valueAt(0), 1);
}

QTEST_MAIN(OdsFilterTest)
