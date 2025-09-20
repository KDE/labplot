/*
	File                 : MCAPFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the MCAP I/O-filter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MCAPFilterTest.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/McapFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KLocalizedString>
#include <QTemporaryFile>

void MCAPFilterTest::testArrayImport() {
	// This mcap file has one topic with name: integer_topic with 10 entries
	// Its encoded in json schema, so each entry looks like this: {"value": n } with n from 0 to 9
	// mcap info basic.mcap
	// library: python mcap 1.1.1
	// profile:
	// messages: 10
	// duration: 9h0m0s
	// start: 0.000000000
	// end: 32400.000000000
	// compression: {none,lz4 or zstd}
	// 	: [1/1 chunks] [575.00 B/575.00 B (0.00%)] [0.00 B/sec]
	// channels:
	//   	(1) integer_topic  10 msgs (0.00 Hz)   : sample [jsonschema]
	// attachments: 0
	// metadata: 0

	QVector<QString> compression_types = {QLatin1String("data/basic_NONE.mcap")};
#ifdef HAVE_LZ4
	compression_types.append(QLatin1String("data/basic_LZ4.mcap"));
#endif
#ifdef HAVE_ZSTD
	compression_types.append(QLatin1String("data/basic_ZSTD.mcap"));
#endif

	for (const QString& file : compression_types) {
		Spreadsheet spreadsheet(QStringLiteral("test"), false);
		McapFilter filter;

		const QString& fileName = QFINDTESTDATA(file);

		AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
		filter.setCreateIndexEnabled(true);
		filter.setDataRowType(QJsonValue::Object);
		filter.setDateTimeFormat(QLatin1String("yyyy-MM-dd"));
		filter.readDataFromFile(fileName, &spreadsheet, mode);

		QCOMPARE(spreadsheet.columnCount(), 5);
		QCOMPARE(spreadsheet.rowCount(), 10);
		QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer); // Index
		QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::DateTime); // LogTime
		QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::DateTime); // PublishTime
		QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Integer); // Sequence
		QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Double); // Value

		QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
		QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);
		QCOMPARE(spreadsheet.column(2)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

		QCOMPARE(spreadsheet.column(0)->name(), i18n("index"));
		QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("logTime"));
		QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("publishTime"));
		QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("sequence"));
		QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("value"));

		// Check Sequence
		QCOMPARE(spreadsheet.column(3)->valueAt(0), 0);
		QCOMPARE(spreadsheet.column(3)->valueAt(1), 1);
		QCOMPARE(spreadsheet.column(3)->valueAt(2), 2);

		// Check Value
		QCOMPARE(spreadsheet.column(4)->valueAt(0), 0);
		QCOMPARE(spreadsheet.column(4)->valueAt(1), 1);
		QCOMPARE(spreadsheet.column(4)->valueAt(2), 2);

		// Check index
		QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
		QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
		QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

		// Check Logging Times
		QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
		QCOMPARE(spreadsheet.column(1)->valueAt(1), 1 * 3600000);
		QCOMPARE(spreadsheet.column(1)->valueAt(2), 2 * 3600000);

		// Check Logging Times
		QCOMPARE(spreadsheet.column(2)->valueAt(0), 0);
		QCOMPARE(spreadsheet.column(2)->valueAt(1), 1 * 3600000);
		QCOMPARE(spreadsheet.column(2)->valueAt(2), 2 * 3600000);
	}
}

void MCAPFilterTest::testExport() {
	QElapsedTimer timer_import;
	timer_import.start();

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	McapFilter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/basic_NONE.mcap"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setCreateIndexEnabled(true);
	filter.setDataRowType(QJsonValue::Object);
	filter.setDateTimeFormat(QLatin1String("yyyy-MM-dd"));
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 5);
	QCOMPARE(spreadsheet.rowCount(), 10);
	qDebug() << "The importing took" << timer_import.elapsed() << "milliseconds";

	QElapsedTimer timer_export;
	timer_export.start();

	QTemporaryFile tmpFile;
	if (tmpFile.open()) {
		filter.write(tmpFile.fileName(), &spreadsheet);
		QCOMPARE(QFile::exists(tmpFile.fileName()), true);
	}

	qDebug() << "The exporting took" << timer_export.elapsed() << "milliseconds";
}

void MCAPFilterTest::testImportWithoutValidTopics() {
	McapFilter filter;

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/basic_NONE_unsupported_encoding.mcap"));
	QCOMPARE(filter.getValidTopics(fileName).size(), 0);

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(filter.lastError(), i18n("No JSON encoded topics found."));
}

void MCAPFilterTest::testImportWrongFile() {
	McapFilter filter;

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/empty_file.mcap"));
	QCOMPARE(filter.getValidTopics(fileName).size(), 0);
	QCOMPARE(filter.lastError(), i18n("Failed to read the file. Reason: %1", QLatin1String("file too small")));
}

QTEST_MAIN(MCAPFilterTest)
