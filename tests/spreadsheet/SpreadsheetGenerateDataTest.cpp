/*
	File                 : SpreadsheetGenerateDataTest.cpp
	Project              : LabPlot
	Description          : Tests for the generation of data in spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetGenerateDataTest.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/spreadsheet/EquidistantValuesDialog.h"

void SpreadsheetGenerateDataTest::initTestCase() {
	CommonTest::initTestCase();

	QLocale::setDefault(QLocale(QLocale::C));
}

// **********************************************************
// **************** Equidistant values **********************
// **********************************************************

// **********************************************************
// ********************* Fixed number ***********************
// **********************************************************

/*!
 * generate equidistant double values, total number of values is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedNumberDouble() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::Double);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumber);
	dlg.setNumber(5);
	dlg.setFromValue(1);
	dlg.setToValue(5);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->valueAt(0), 1.);
	QCOMPARE(column->valueAt(1), 2.);
	QCOMPARE(column->valueAt(2), 3.);
	QCOMPARE(column->valueAt(3), 4.);
	QCOMPARE(column->valueAt(4), 5.);
}

/*!
 * generate equidistant integer values, total number of values is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedNumberInt() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::Integer);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumber);
	dlg.setNumber(5);
	dlg.setFromValue(1);
	dlg.setToValue(5);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->integerAt(0), 1);
	QCOMPARE(column->integerAt(1), 2);
	QCOMPARE(column->integerAt(2), 3);
	QCOMPARE(column->integerAt(3), 4);
	QCOMPARE(column->integerAt(4), 5);
}

/*!
 * generate equidistant big int values, total number of values is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedNumberBigInt() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::BigInt);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumber);
	dlg.setNumber(5);
	dlg.setFromValue(1);
	dlg.setToValue(5);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->bigIntAt(0), 1);
	QCOMPARE(column->bigIntAt(1), 2);
	QCOMPARE(column->bigIntAt(2), 3);
	QCOMPARE(column->bigIntAt(3), 4);
	QCOMPARE(column->bigIntAt(4), 5);
}

/*!
 * generate equidistant DateTime values, total number of values is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedNumberDateTime() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::DateTime);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumber);
	dlg.setNumber(5);

	auto dateTime = QDateTime::fromString(QStringLiteral("2023-05-01T00:00:00Z"), Qt::ISODate);
	dlg.setFromDateTime(dateTime.toMSecsSinceEpoch());
	dlg.setToDateTime(dateTime.addSecs(4).toMSecsSinceEpoch());
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->dateTimeAt(0), QDateTime::fromString(QStringLiteral("2023-05-01T00:00:00Z"), Qt::ISODate));
	QCOMPARE(column->dateTimeAt(1), QDateTime::fromString(QStringLiteral("2023-05-01T00:00:01Z"), Qt::ISODate));
	QCOMPARE(column->dateTimeAt(2), QDateTime::fromString(QStringLiteral("2023-05-01T00:00:02Z"), Qt::ISODate));
	QCOMPARE(column->dateTimeAt(3), QDateTime::fromString(QStringLiteral("2023-05-01T00:00:03Z"), Qt::ISODate));
	QCOMPARE(column->dateTimeAt(4), QDateTime::fromString(QStringLiteral("2023-05-01T00:00:04Z"), Qt::ISODate));
}

/*!
 * two columns (double and datetime) provided, generate equidistant values for both, total number of values is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedNumberDoubleDateTime() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(1);
	auto* column1 = sheet.column(0);
	column1->setColumnMode(AbstractColumn::ColumnMode::Double);
	auto* column2 = sheet.column(1);
	column2->setColumnMode(AbstractColumn::ColumnMode::DateTime);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column1, column2});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumber);
	dlg.setNumber(5);
	dlg.setFromValue(1);
	dlg.setToValue(5);
	auto dateTime = QDateTime::fromString(QStringLiteral("2023-05-01T00:00:00Z"), Qt::ISODate);
	dlg.setFromDateTime(dateTime.toMSecsSinceEpoch());
	dlg.setToDateTime(dateTime.addSecs(4).toMSecsSinceEpoch());
	dlg.generate();

	// checks for the numeric column
	QCOMPARE(column1->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column1->rowCount(), 5);
	QCOMPARE(column1->valueAt(0), 1.);
	QCOMPARE(column1->valueAt(1), 2.);
	QCOMPARE(column1->valueAt(2), 3.);
	QCOMPARE(column1->valueAt(3), 4.);
	QCOMPARE(column1->valueAt(4), 5.);

	// checks for the DateTime column
	QCOMPARE(column2->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(column2->rowCount(), 5);
	QCOMPARE(column2->dateTimeAt(0), QDateTime::fromString(QStringLiteral("2023-05-01T00:00:00Z"), Qt::ISODate));
	QCOMPARE(column2->dateTimeAt(1), QDateTime::fromString(QStringLiteral("2023-05-01T00:00:01Z"), Qt::ISODate));
	QCOMPARE(column2->dateTimeAt(2), QDateTime::fromString(QStringLiteral("2023-05-01T00:00:02Z"), Qt::ISODate));
	QCOMPARE(column2->dateTimeAt(3), QDateTime::fromString(QStringLiteral("2023-05-01T00:00:03Z"), Qt::ISODate));
	QCOMPARE(column2->dateTimeAt(4), QDateTime::fromString(QStringLiteral("2023-05-01T00:00:04Z"), Qt::ISODate));
}

// **********************************************************
// ******************** Fixed increment *********************
// **********************************************************

/*!
 * generate equidistant double values, the increment is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedIncrementDouble() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::Double);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedIncrement);
	dlg.setIncrement(0.2);
	dlg.setFromValue(1);
	dlg.setToValue(1.8);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->valueAt(0), 1.);
	QCOMPARE(column->valueAt(1), 1.2);
	QCOMPARE(column->valueAt(2), 1.4);
	QCOMPARE(column->valueAt(3), 1.6);
	QCOMPARE(column->valueAt(4), 1.8);
}

/*!
 * generate equidistant integer values, the increment is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedIncrementInt() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::Integer);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedIncrement);
	dlg.setIncrement(1);
	dlg.setFromValue(1);
	dlg.setToValue(5);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->integerAt(0), 1);
	QCOMPARE(column->integerAt(1), 2);
	QCOMPARE(column->integerAt(2), 3);
	QCOMPARE(column->integerAt(3), 4);
	QCOMPARE(column->integerAt(4), 5);
}

/*!
 * generate equidistant big int values, the increment is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedIncrementBigInt() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::BigInt);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedIncrement);
	dlg.setIncrement(1);
	dlg.setFromValue(1);
	dlg.setToValue(5);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->bigIntAt(0), 1);
	QCOMPARE(column->bigIntAt(1), 2);
	QCOMPARE(column->bigIntAt(2), 3);
	QCOMPARE(column->bigIntAt(3), 4);
	QCOMPARE(column->bigIntAt(4), 5);
}

/*!
 * generate equidistant DateTime values, the increment is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedIncrementDateTime() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::DateTime);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedIncrement);
	dlg.setIncrementDateTimeUnit(EquidistantValuesDialog::DateTimeUnit::Year);
	auto dateTime = QDateTime::fromString(QStringLiteral("2023-05-01T00:00:00Z"), Qt::ISODate);
	dlg.setFromDateTime(dateTime.toMSecsSinceEpoch());
	dlg.setToDateTime(dateTime.addYears(4).toMSecsSinceEpoch());
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->dateTimeAt(0).date().year(), 2023);
	QCOMPARE(column->dateTimeAt(1).date().year(), 2024);
	QCOMPARE(column->dateTimeAt(2).date().year(), 2025);
	QCOMPARE(column->dateTimeAt(3).date().year(), 2026);
	QCOMPARE(column->dateTimeAt(4).date().year(), 2027);
}

// **********************************************************
// **************** Fixed number and increment **************
// **********************************************************

/*!
 * generate equidistant double values, total number of values and the increment are fixed.
 */
void SpreadsheetGenerateDataTest::testFixedNumberIncrementDouble() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::Double);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumberIncrement);
	dlg.setNumber(5);
	dlg.setIncrement(10);
	dlg.setFromValue(0.);
	// dlg.setToValue(5);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->valueAt(0), 0.);
	QCOMPARE(column->valueAt(1), 10.);
	QCOMPARE(column->valueAt(2), 20.);
	QCOMPARE(column->valueAt(3), 30.);
	QCOMPARE(column->valueAt(4), 40.);
}

/*!
 * generate equidistant integer values, the increment is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedNumberIncrementInt() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::Integer);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumberIncrement);
	dlg.setNumber(5);
	dlg.setIncrement(1);
	dlg.setFromValue(1);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->integerAt(0), 1);
	QCOMPARE(column->integerAt(1), 2);
	QCOMPARE(column->integerAt(2), 3);
	QCOMPARE(column->integerAt(3), 4);
	QCOMPARE(column->integerAt(4), 5);
}

/*!
 * generate equidistant big int values, the increment is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedNumberIncrementBigInt() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::BigInt);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumberIncrement);
	dlg.setNumber(5);
	dlg.setIncrement(1);
	dlg.setFromValue(1);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->bigIntAt(0), 1);
	QCOMPARE(column->bigIntAt(1), 2);
	QCOMPARE(column->bigIntAt(2), 3);
	QCOMPARE(column->bigIntAt(3), 4);
	QCOMPARE(column->bigIntAt(4), 5);
}

/*!
 * generate equidistant DateTime values, the increment is fixed.
 */
void SpreadsheetGenerateDataTest::testFixedNumberIncrementDateTime() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::DateTime);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumberIncrement);
	dlg.setNumber(5);
	dlg.setIncrementDateTimeUnit(EquidistantValuesDialog::DateTimeUnit::Year);
	auto dateTime = QDateTime::fromString(QStringLiteral("2023-05-01T00:00:00Z"), Qt::ISODate);
	dlg.setFromDateTime(dateTime.toMSecsSinceEpoch());
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->dateTimeAt(0).date().year(), 2023);
	QCOMPARE(column->dateTimeAt(1).date().year(), 2024);
	QCOMPARE(column->dateTimeAt(2).date().year(), 2025);
	QCOMPARE(column->dateTimeAt(3).date().year(), 2026);
	QCOMPARE(column->dateTimeAt(4).date().year(), 2027);
}

// **********************************************************
// ***************** Column mode conversion *****************
// **********************************************************

/*!
 * generate equidistant big int values, total number of values is fixed, the initial int column mode needs to be adjusted.
 */
void SpreadsheetGenerateDataTest::testFixedNumberIntToBigInt() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::Integer);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumber);
	dlg.setNumber(5);
	dlg.setFromValue(2147483647.);
	dlg.setToValue(2147483651.);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->bigIntAt(0), 2147483647);
	QCOMPARE(column->bigIntAt(1), 2147483648);
	QCOMPARE(column->bigIntAt(2), 2147483649);
	QCOMPARE(column->bigIntAt(3), 2147483650);
	QCOMPARE(column->bigIntAt(4), 2147483651);
}

/*!
 * generate equidistant double values, total number of values is fixed, the initial int column mode needs to be adjusted.
 */
void SpreadsheetGenerateDataTest::testFixedNumberIntToDouble() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::Integer);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumber);
	dlg.setNumber(5);
	dlg.setFromValue(1);
	dlg.setToValue(1.8);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->valueAt(0), 1.);
	QCOMPARE(column->valueAt(1), 1.2);
	QCOMPARE(column->valueAt(2), 1.4);
	QCOMPARE(column->valueAt(3), 1.6);
	QCOMPARE(column->valueAt(4), 1.8);
}

/*!
 * generate equidistant double values, total number of values is fixed, the initial big int column mode needs to be adjusted.
 */
void SpreadsheetGenerateDataTest::testFixedNumberBigIntToDouble() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(1);
	auto* column = sheet.column(0);
	column->setColumnMode(AbstractColumn::ColumnMode::BigInt);

	EquidistantValuesDialog dlg(&sheet);
	dlg.setColumns(QVector<Column*>{column});
	dlg.setType(EquidistantValuesDialog::Type::FixedNumber);
	dlg.setNumber(5);
	dlg.setFromValue(1);
	dlg.setToValue(1.8);
	dlg.generate();

	// checks
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(column->rowCount(), 5);
	QCOMPARE(column->valueAt(0), 1.);
	QCOMPARE(column->valueAt(1), 1.2);
	QCOMPARE(column->valueAt(2), 1.4);
	QCOMPARE(column->valueAt(3), 1.6);
	QCOMPARE(column->valueAt(4), 1.8);
}

QTEST_MAIN(SpreadsheetGenerateDataTest)
