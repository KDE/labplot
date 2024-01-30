/*
	File                 : MCAPFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the MCAP I/O-filter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MCAPFilterTest.h"
#include "backend/datasources/filters/McapFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KLocalizedString>

void MCAPFilterTest::initTestCase() {
	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	// TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

void MCAPFilterTest::testArrayImport() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	McapFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/basic.mcap"));

	// This mcap file has one topic with name: integer_topic with 10 entries
	// Its encoded in json schema, so each entry looks like this: {"value": n } with n from 0 to 9
	// mcap info basic.mcap
	// library: python mcap 1.1.1
	// profile: 
	// messages: 10
	// duration: 176.674µs
	// start: 2024-01-18T18:57:47.21980086-08:00 (1705633067.219800860)
	// end: 2024-01-18T18:57:47.219977534-08:00 (1705633067.219977534)
	// compression:
	// 	: [1/1 chunks] [575.00 B/575.00 B (0.00%)] [3.10 MiB/sec] 
	// channels:
	//   	(1) integer_topic  10 msgs (56601.42 Hz)   : sample [jsonschema]  
	// attachments: 0
	// metadata: 0


	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setCreateIndexEnabled(true);
	filter.setDataRowType(QJsonValue::Double);
	filter.setDateTimeFormat(QLatin1String("yyyy-MM-dd"));
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 4);
	QCOMPARE(spreadsheet.rowCount(), 10);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer); // Index
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::DateTime); // LogTime
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::DateTime); // PublishTime
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Integer); // Sequence


	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
	QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->name(), i18n("index"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("logTime"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("publishTime"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("sequence"));


	// Check index
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	// Check Publish Times
	QCOMPARE(spreadsheet.column(1)->dateTimeAt(0), QDateTime::fromMSecsSinceEpoch(0));
	QCOMPARE(spreadsheet.column(1)->dateTimeAt(1), QDateTime::fromMSecsSinceEpoch(1 * 3600000));
	QCOMPARE(spreadsheet.column(1)->dateTimeAt(2), QDateTime::fromMSecsSinceEpoch(2 * 3600000));

	// Check Logging Times
	QCOMPARE(spreadsheet.column(2)->dateTimeAt(0), QDateTime::fromMSecsSinceEpoch(0));
	QCOMPARE(spreadsheet.column(2)->dateTimeAt(1), QDateTime::fromMSecsSinceEpoch(1 * 3600000));
	QCOMPARE(spreadsheet.column(2)->dateTimeAt(2), QDateTime::fromMSecsSinceEpoch(2 * 3600000));

	// Check Sequence
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 1);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 2);


}

QTEST_MAIN(MCAPFilterTest)


