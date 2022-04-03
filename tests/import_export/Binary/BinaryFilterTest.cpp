/*
    File                 : BinaryFilterTest.cpp
    Project              : LabPlot
    Description          : Tests for the binary filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BinaryFilterTest.h"
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

void BinaryFilterTest::importInt8() {
	Spreadsheet spreadsheet("test", false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/int8.bin"));
	filter.setDataType(BinaryFilter::DataType::INT8);
	//filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	//spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 1000);

	//TODO: Fix
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	//QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	//QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 9);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 19);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 29);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 38);
	QCOMPARE(spreadsheet.column(0)->valueAt(998), 99);
	QCOMPARE(spreadsheet.column(1)->valueAt(998), -59);
	QCOMPARE(spreadsheet.column(0)->valueAt(999), 100);
	QCOMPARE(spreadsheet.column(1)->valueAt(999), -50);
}

void BinaryFilterTest::importFloatBE() {
	Spreadsheet spreadsheet("test", false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/float.bin"));
	filter.setDataType(BinaryFilter::DataType::REAL32);
	filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	//spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 1000);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	//DEBUG(Q_FUNC_INFO << ", value = " << spreadsheet.column(0)->valueAt(0))
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 5.27598034705257e-11);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1.35065096884546e-08);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 3.83760891509155e-07);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 3.45766648024437e-06);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 3.07619702653028e-05);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 9.82427882263437e-05);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 0.000295705918688327);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 0.000885162618942559);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 0.00276252860203385);
	QCOMPARE(spreadsheet.column(0)->valueAt(998), 0.);
	QCOMPARE(spreadsheet.column(1)->valueAt(998), 0.);
	QCOMPARE(spreadsheet.column(0)->valueAt(999), 0.);
	QCOMPARE(spreadsheet.column(1)->valueAt(999), 0.);
}

void BinaryFilterTest::importDoubleBE() {
	Spreadsheet spreadsheet("test", false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/double.bin"));
	filter.setDataType(BinaryFilter::DataType::REAL64);
	filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	//spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 1000);
	
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0.1001001001001);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 9.9932916564023);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0.2002002002002);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 19.8865340135936);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0.3003003003003);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 29.5806794305369);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 0.4004004004004);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 38.978673554296);
	QCOMPARE(spreadsheet.column(0)->valueAt(998), 99.8998998998999);
	QCOMPARE(spreadsheet.column(1)->valueAt(998), -59.000490423342);
	QCOMPARE(spreadsheet.column(0)->valueAt(999), 100);
	QCOMPARE(spreadsheet.column(1)->valueAt(999), -50.6365641109759);
}

QTEST_MAIN(BinaryFilterTest)
