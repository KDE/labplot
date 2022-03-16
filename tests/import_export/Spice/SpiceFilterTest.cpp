/*
    File                 : AsciiFilterTest.cpp
    Project              : LabPlot
    Description          : Tests for the ascii filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpiceFilterTest.h"
#include <qglobal.h>
#include "src/backend/datasources/filters/SpiceFilter.h"
#include "src/backend/spreadsheet/Spreadsheet.h"

#include <data/ngspice/dc_ascii.raw.h>
#include <data/ngspice/dc_binary.raw.h>
#include <data/ngspice/ac_ascii.raw.h>
#include <data/ngspice/ac_binary.raw.h>
#include <data/ltspice/AC/LowPassFilter_AC.raw.h>
#include <data/ltspice/transient/LowPassFilter.raw.h>
#include <data/ltspice/LowPassFilter_transient_doubleFlag.raw.h>

#include <QFile>

const QString ngspicePath = "data/ngspice"; // relative path
const QString ltspicePath = "data/ltspice"; // relative path

#define NGSpiceFile QFINDTESTDATA(ngspicePath + "/" + filename) // filename comes from the namespace
#define LTSpiceFile QFINDTESTDATA(ltspicePath + "/" + filename) // filename comes from the namespace
#define NGSpiceRefDataFile QFINDTESTDATA(ngspicePath + "/" + filename + ".refdata") // filename comes from the namespace
#define LTSpiceRefDataFile QFINDTESTDATA(ltspicePath + "/" + filename + ".refdata") // filename comes from the namespace


#define READ_REFDATA(filename) \
	auto filepath = QFINDTESTDATA(filename); \
	QFile f(filepath); \
	QCOMPARE(f.open(QIODevice::ReadOnly), true); \
	QVector<QStringList> refData; \
	while (!f.atEnd()) { \
		QString line = f.readLine().simplified(); \
		refData.append(line.split(",")); \
	} \
	QVERIFY(refData.count() > 0);

// Compare that the columns in the spreadsheet have the correct name and columnMode
#define COMPARE_COLUMN_NAMES_MODE(spreadsheet, columnNames, refColumnCount) \
	QCOMPARE(spreadsheet.columnCount(), refColumnCount); \
	for (int i=0; i < refColumnCount; i++) { \
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Double); \
		QCOMPARE(spreadsheet.column(i)->name(), columnNames.at(i)); \
	}

// Compare all data in the spreadsheet with the reference data
#define COMPARE_ROW_VALUES_START_END_ROW(spreadsheet, refData, refDataRowCount, refColumnCount, startRow, endRow); \
	QCOMPARE(spreadsheet.columnCount(), refColumnCount); \
	QCOMPARE(spreadsheet.rowCount(), endRow - startRow + 1); \
	for (int row = startRow - 1; row < endRow; row++) { \
		for (int col = 0; col < refColumnCount; col++) { \
			QCOMPARE(spreadsheet.column(col)->valueAt(row - startRow + 1), refData.at(row).at(col).toDouble()); \
		} \
	}

// Compare all data in the spreadsheet with the reference data
#define COMPARE_ROW_VALUES(spreadsheet, refData, refDataRowCount, refColumnCount); \
	QCOMPARE(spreadsheet.columnCount(), refColumnCount); \
	QCOMPARE(spreadsheet.rowCount(), refDataRowCount); \
	for (int row = 0; row < refData.count(); row++) { \
		for (int col = 0; col < refColumnCount; col++) { \
			QCOMPARE(spreadsheet.column(col)->valueAt(row), refData.at(row).at(col).toDouble()); \
		} \
	}

// Float version
// Compare all data in the spreadsheet with the reference data
#define COMPARE_ROW_VALUES_FLOAT(spreadsheet, refData, refDataRowCount, refColumnCount); \
	QCOMPARE(spreadsheet.columnCount(), refColumnCount); \
	QCOMPARE(spreadsheet.rowCount(), refDataRowCount); \
	for (int row = 0; row < refData.count(); row++) { \
		for (int col = 0; col < refColumnCount; col++) { \
			QVERIFY(qFuzzyCompare(static_cast<float>(spreadsheet.column(col)->valueAt(row)), refData.at(row).at(col).toFloat())); \
		} \
	}

void SpiceFilterTest::initTestCase() {
	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

void SpiceFilterTest::NgSpiceAsciiFileToBinaryFilter() {
	using namespace dc_ascii;

	const QString ngFile = NGSpiceFile;
	QCOMPARE(SpiceFilter::isSpiceBinaryFile(ngFile), false);
}

void SpiceFilterTest::NgSpiceBinaryFileToAsciiFilter() {
	using namespace dc_binary;

	const QString ngFile = NGSpiceFile;
	QCOMPARE(SpiceFilter::isSpiceAsciiFile(ngFile), false);
}

void SpiceFilterTest::NgSpiceDCAscii() {
	using namespace dc_ascii;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	QCOMPARE(SpiceFilter::isSpiceAsciiFile(ngFile), true);
	SpiceFilter filter(SpiceFilter::Type::Ascii);
	auto res = filter.preview(ngFile, refData.count());

	QCOMPARE(res.length(), refData.length());
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::NgSpiceDCBinary() {
	using namespace dc_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	QCOMPARE(SpiceFilter::isSpiceBinaryFile(ngFile), true);
	SpiceFilter filter(SpiceFilter::Type::Binary);
	auto res = filter.preview(ngFile, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::NgSpiceACAscii() {
	using namespace ac_ascii;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString ngFile = NGSpiceFile;
	QCOMPARE(SpiceFilter::isSpiceAsciiFile(ngFile), true);
	const int refColumnCount = refData.at(0).count();

	SpiceFilter filter(SpiceFilter::Type::Ascii);
	auto res = filter.preview(ngFile, refData.count());

	QCOMPARE(res.length(), refData.length());
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::NgSpiceACBinary() {
	using namespace ac_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	QCOMPARE(SpiceFilter::isSpiceBinaryFile(ngFile), true);
	SpiceFilter filter(SpiceFilter::Type::Binary);
	auto res = filter.preview(ngFile, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES(sheet, refData, refDataRowCount, refColumnCount);
}

// ######################################################################################################
// Startrow not zero
// ######################################################################################################
void SpiceFilterTest::NgSpiceDCAsciiStartRowNotZero() {
	using namespace dc_ascii;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	QCOMPARE(SpiceFilter::isSpiceAsciiFile(ngFile), true);
	SpiceFilter filter(SpiceFilter::Type::Ascii);
	const int startRow = 2;
	const int endRow = 4;
	filter.setStartRow(startRow);
	filter.setEndRow(endRow);
	auto res = filter.preview(ngFile, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = startRow - 1; i < startRow - 1 + numberPreviewData; i++)
		QCOMPARE(res.at(i - startRow + 1), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES_START_END_ROW(sheet, refData, refDataRowCount, refColumnCount, startRow, endRow);
}

void SpiceFilterTest::NgSpiceDCBinaryStartRowNotZero() {
	using namespace dc_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	QCOMPARE(SpiceFilter::isSpiceBinaryFile(ngFile), true);
	SpiceFilter filter(SpiceFilter::Type::Binary);
	const int startRow = 31;
	const int endRow = 433;
	filter.setStartRow(startRow);
	filter.setEndRow(endRow);
	auto res = filter.preview(ngFile, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = startRow - 1; i < startRow - 1 + numberPreviewData; i++)
		QCOMPARE(res.at(i - startRow + 1), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES_START_END_ROW(sheet, refData, refDataRowCount, refColumnCount, startRow, endRow);
}

void SpiceFilterTest::NgSpiceACAsciiStartRowNotZero() {
	using namespace ac_ascii;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString ngFile = NGSpiceFile;
	QCOMPARE(SpiceFilter::isSpiceAsciiFile(ngFile), true);
	const int refColumnCount = refData.at(0).count();

	SpiceFilter filter(SpiceFilter::Type::Ascii);
	const int startRow = 2;
	const int endRow = 4;
	filter.setStartRow(startRow);
	filter.setEndRow(endRow);
	auto res = filter.preview(ngFile, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = startRow - 1; i < startRow - 1 + numberPreviewData; i++)
		QCOMPARE(res.at(i - startRow + 1), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES_START_END_ROW(sheet, refData, refDataRowCount, refColumnCount, startRow, endRow);
}

void SpiceFilterTest::NgSpiceACBinaryStartRowNotZero() {
	using namespace ac_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	QCOMPARE(SpiceFilter::isSpiceBinaryFile(ngFile), true);
	SpiceFilter filter(SpiceFilter::Type::Binary);
	const int startRow = 2;
	const int endRow = 198;
	filter.setStartRow(startRow);
	filter.setEndRow(endRow);
	auto res = filter.preview(ngFile, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = startRow - 1; i < startRow - 1 + numberPreviewData; i++)
		QCOMPARE(res.at(i - startRow + 1), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES_START_END_ROW(sheet, refData, refDataRowCount, refColumnCount, startRow, endRow);
}

//########################################################################################################
//
//########################################################################################################

void SpiceFilterTest::NgSpiceDCBinaryBulkReadNumberLines() {
	using namespace dc_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	QCOMPARE(SpiceFilter::isSpiceBinaryFile(ngFile), true);
	SpiceFilter filter(SpiceFilter::Type::Ascii);
	const int startRow = 31;
	const int endRow = 433;
	filter.setReaderBulkLineCount(100);
	filter.setStartRow(startRow);
	filter.setEndRow(endRow);
	auto res = filter.preview(ngFile, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i= startRow - 1; i < startRow - 1 + numberPreviewData; i++) {
		QCOMPARE(res.at(i - startRow + 1), refData.at(i));
	}

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES_START_END_ROW(sheet, refData, refDataRowCount, refColumnCount, startRow, endRow);
}

void SpiceFilterTest::NgSpiceACBinaryBulkReadNumberLines() {
	using namespace ac_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	QCOMPARE(SpiceFilter::isSpiceBinaryFile(ngFile), true);
	SpiceFilter filter(SpiceFilter::Type::Binary);
	filter.setReaderBulkLineCount(100);
	const int startRow = 2;
	const int endRow = 198;
	filter.setStartRow(startRow);
	filter.setEndRow(endRow);
	auto res = filter.preview(ngFile, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = startRow - 1; i < startRow - 1 + numberPreviewData; i++)
		QCOMPARE(res.at(i - startRow + 1), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES_START_END_ROW(sheet, refData, refDataRowCount, refColumnCount, startRow, endRow);
}

// ###########################################################################################################
// ####  LTSpice  ############################################################################################
// ###########################################################################################################

void SpiceFilterTest::LtSpiceACBinary() {
	using namespace LowPassFilter_AC;

	READ_REFDATA(LTSpiceRefDataFile);
	const QString file = LTSpiceFile;
	const int refColumnCount = refData.at(0).count();

	QCOMPARE(SpiceFilter::isSpiceBinaryFile(file), true);
	SpiceFilter filter(SpiceFilter::Type::Binary);
	auto res = filter.preview(file, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(file);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(file, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::LtSpiceTranBinary() {
	using namespace LowPassFilter_Transient;

	READ_REFDATA(LTSpiceRefDataFile);
	const QString file = LTSpiceFile;
	const int refColumnCount = refData.at(0).count();

	QCOMPARE(SpiceFilter::isSpiceBinaryFile(file), true);
	SpiceFilter filter(SpiceFilter::Type::Binary);
	auto res = filter.preview(file, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++) {
		for (int j = 0; j < columnNames.length(); j++)
			QVERIFY(qFuzzyCompare(res.at(i).at(j).toFloat(), refData.at(i).at(j).toFloat()));
	}

	QString resFileInfoString = filter.fileInfoString(file);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(file, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	// Because the read data are floats, a float comparsion must be done, because
	// comparing float and double will not work properly
	COMPARE_ROW_VALUES_FLOAT(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::LtSpiceTranDoubleBinary() {
	using namespace LowPassFilter_transient_doubleFlag;

	READ_REFDATA(LTSpiceRefDataFile);
	const QString file = LTSpiceFile;
	const int refColumnCount = refData.at(0).count();

	QCOMPARE(SpiceFilter::isSpiceBinaryFile(file), true);
	SpiceFilter filter(SpiceFilter::Type::Binary);
	auto res = filter.preview(file, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(file);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet("Test", false);
	filter.readDataFromFile(file, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES(sheet, refData, refDataRowCount, refColumnCount);
}

QTEST_MAIN(SpiceFilterTest)
