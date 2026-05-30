/*
	File                 : ParquetFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the Parquet/Arrow IPC/ORC import filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ParquetFilterTest.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/ParquetFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <cmath>

/*!
 * import basic Parquet file: 4 columns (int32, double, string, timestamp), 5 rows, with nulls
 */
void ParquetFilterTest::testParquetBasicImport() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ParquetFilter filter(AbstractFileFilter::FileType::Parquet);

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testdata.parquet"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// 4 columns, 5 rows
	QCOMPARE(spreadsheet.columnCount(), 4);
	QCOMPARE(spreadsheet.rowCount(), 5);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QStringLiteral("id"));
	QCOMPARE(spreadsheet.column(1)->name(), QStringLiteral("value"));
	QCOMPARE(spreadsheet.column(2)->name(), QStringLiteral("name"));
	QCOMPARE(spreadsheet.column(3)->name(), QStringLiteral("ts"));

	// column types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::DateTime);

	// id column: 1,2,3,4,5
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->integerAt(3), 4);
	QCOMPARE(spreadsheet.column(0)->integerAt(4), 5);

	// value column: 1.1, 2.2, null, 4.4, 5.5
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.1);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 2.2);
	QVERIFY(std::isnan(spreadsheet.column(1)->valueAt(2))); // null → NaN
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 4.4);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 5.5);

	// name column: Alice, Bob, null, Dave, Eve
	QCOMPARE(spreadsheet.column(2)->textAt(0), QStringLiteral("Alice"));
	QCOMPARE(spreadsheet.column(2)->textAt(1), QStringLiteral("Bob"));
	QCOMPARE(spreadsheet.column(2)->textAt(2), QString()); // null → empty string
	QCOMPARE(spreadsheet.column(2)->textAt(3), QStringLiteral("Dave"));
	QCOMPARE(spreadsheet.column(2)->textAt(4), QStringLiteral("Eve"));

	// timestamp column: 2025-01-01, 2025-06-15 12:30, 2025-12-31 23:59:59, null, 2024-03-14 09:26:53
	QCOMPARE(spreadsheet.column(3)->dateTimeAt(0), QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0), Qt::UTC));
	QCOMPARE(spreadsheet.column(3)->dateTimeAt(1), QDateTime(QDate(2025, 6, 15), QTime(12, 30, 0), Qt::UTC));
	QCOMPARE(spreadsheet.column(3)->dateTimeAt(2), QDateTime(QDate(2025, 12, 31), QTime(23, 59, 59), Qt::UTC));
	QCOMPARE(spreadsheet.column(3)->dateTimeAt(3), QDateTime()); // null → empty
	QCOMPARE(spreadsheet.column(3)->dateTimeAt(4), QDateTime(QDate(2024, 3, 14), QTime(9, 26, 53), Qt::UTC));
}

/*!
 * import Parquet file with extended numeric types: int8, int16, int32, int64, float, double, string, bool
 */
void ParquetFilterTest::testParquetTypes() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ParquetFilter filter(AbstractFileFilter::FileType::Parquet);

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testdata_types.parquet"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 8);
	QCOMPARE(spreadsheet.rowCount(), 3);

	// int8, int16, int32 → Integer
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// int64 → BigInt
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::BigInt);

	// float, double → Double
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(5)->columnMode(), AbstractColumn::ColumnMode::Double);

	// string → Text
	QCOMPARE(spreadsheet.column(6)->columnMode(), AbstractColumn::ColumnMode::Text);

	// bool → Integer
	QCOMPARE(spreadsheet.column(7)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// spot-check values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);   // int8
	QCOMPARE(spreadsheet.column(0)->integerAt(1), -2);
	QCOMPARE(spreadsheet.column(2)->integerAt(2), 30000); // int32
	QCOMPARE(spreadsheet.column(3)->bigIntAt(0), (qint64)100000000000LL); // int64
	QCOMPARE(spreadsheet.column(6)->textAt(0), QStringLiteral("hello"));
	QCOMPARE(spreadsheet.column(7)->integerAt(0), 1); // true
	QCOMPARE(spreadsheet.column(7)->integerAt(1), 0); // false
}

/*!
 * import Parquet file where all values are null
 */
void ParquetFilterTest::testParquetNulls() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ParquetFilter filter(AbstractFileFilter::FileType::Parquet);

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testdata_nulls.parquet"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 3);

	// all int values are 0 (null → 0)
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 0);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 0);

	// all double values are NaN
	QVERIFY(std::isnan(spreadsheet.column(1)->valueAt(0)));
	QVERIFY(std::isnan(spreadsheet.column(1)->valueAt(1)));
	QVERIFY(std::isnan(spreadsheet.column(1)->valueAt(2)));

	// all text values are empty
	QCOMPARE(spreadsheet.column(2)->textAt(0), QString());
	QCOMPARE(spreadsheet.column(2)->textAt(1), QString());
	QCOMPARE(spreadsheet.column(2)->textAt(2), QString());
}

/*!
 * import empty Parquet file (schema only, 0 rows)
 */
void ParquetFilterTest::testParquetEmpty() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ParquetFilter filter(AbstractFileFilter::FileType::Parquet);

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testdata_empty.parquet"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 0);
}

/*!
 * import with start/end row: rows 2-4 from a 5-row file
 */
void ParquetFilterTest::testParquetRowRange() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ParquetFilter filter(AbstractFileFilter::FileType::Parquet);

	filter.setStartRow(2);
	filter.setEndRow(4);

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testdata.parquet"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// rows 2-4 → 3 rows
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 4);

	// id column should be 2, 3, 4
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 3);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 4);
}

/*!
 * import with start/end column: columns 2-3 from a 4-column file
 */
void ParquetFilterTest::testParquetColumnRange() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ParquetFilter filter(AbstractFileFilter::FileType::Parquet);

	filter.setStartColumn(2);
	filter.setEndColumn(3);

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testdata.parquet"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// columns 2-3 → 2 columns (value, name)
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 5);

	// first imported column should be "value" (Double)
	QCOMPARE(spreadsheet.column(0)->name(), QStringLiteral("value"));
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);

	// second imported column should be "name" (Text)
	QCOMPARE(spreadsheet.column(1)->name(), QStringLiteral("name"));
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Text);
}

/*!
 * test preview: returns first N rows as string lists
 */
void ParquetFilterTest::testParquetPreview() {
	ParquetFilter filter(AbstractFileFilter::FileType::Parquet);

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testdata.parquet"));
	auto preview = filter.preview(fileName, 3);

	// 3 rows of preview data + 1 header row
	QCOMPARE(preview.size(), 4);

	// header row: column names
	QCOMPARE(preview[0].size(), 4);
	QCOMPARE(preview[0][0], QStringLiteral("id"));
	QCOMPARE(preview[0][1], QStringLiteral("value"));
	QCOMPARE(preview[0][2], QStringLiteral("name"));
	QCOMPARE(preview[0][3], QStringLiteral("ts"));

	// first data row
	QCOMPARE(preview[1][0], QStringLiteral("1"));
	QCOMPARE(preview[1][2], QStringLiteral("Alice"));
}

// ============================================================================
// Arrow IPC format tests
// ============================================================================

/*!
 * import Arrow IPC (Feather) file: same data as basic Parquet test
 */
void ParquetFilterTest::testArrowIPCBasicImport() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ParquetFilter filter(AbstractFileFilter::FileType::ArrowIPC);

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testdata.arrow"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 4);
	QCOMPARE(spreadsheet.rowCount(), 5);

	// same data as Parquet basic import
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(4), 5);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.1);
	QVERIFY(std::isnan(spreadsheet.column(1)->valueAt(2)));
	QCOMPARE(spreadsheet.column(2)->textAt(0), QStringLiteral("Alice"));
	QCOMPARE(spreadsheet.column(3)->dateTimeAt(0), QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0), Qt::UTC));
}

// ============================================================================
// ORC format tests
// ============================================================================

/*!
 * import ORC file: same data as basic Parquet test
 */
void ParquetFilterTest::testORCBasicImport() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ParquetFilter filter(AbstractFileFilter::FileType::ORC);

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testdata.orc"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 4);
	QCOMPARE(spreadsheet.rowCount(), 5);

	// same data as Parquet basic import
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(4), 5);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.1);
	QVERIFY(std::isnan(spreadsheet.column(1)->valueAt(2)));
	QCOMPARE(spreadsheet.column(2)->textAt(0), QStringLiteral("Alice"));
	QCOMPARE(spreadsheet.column(3)->dateTimeAt(0), QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0), Qt::UTC));
}

QTEST_MAIN(ParquetFilterTest)
