/*
	File                 : SpreadsheetGenerateDataTest.cpp
	Project              : LabPlot
	Description          : Tests for the generation of data in spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetGenerateDataTest.h"
#include "kdefrontend/spreadsheet/EquidistantValuesDialog.h"
#include "backend/spreadsheet/Spreadsheet.h"

void SpreadsheetGenerateDataTest::initTestCase() {
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
	QLocale::setDefault(QLocale(QLocale::C));
}

//**********************************************************
//**************** Equidistant values **********************
//**********************************************************

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

void SpreadsheetGenerateDataTest::testFixedNumberDateTime() {

}

void SpreadsheetGenerateDataTest::testFixedNumberDoubleDateTime() {

}

// fixed increment between the values
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

// column mode conversion
void SpreadsheetGenerateDataTest::testFixedNumberIntToBigInt() {
	QSKIP("doesn't work");
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

void SpreadsheetGenerateDataTest::testFixedNumberBigIntToDouble() {

}

QTEST_MAIN(SpreadsheetGenerateDataTest)
