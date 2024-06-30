/*
	File                 : AsciiFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the ascii filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpiceFilterTest.h"
#include "src/backend/datasources/filters/SpiceFilter.h"
#include "src/backend/spreadsheet/Spreadsheet.h"

#include <data/ltspice/AC/LowPassFilter_AC.raw.h>
#include <data/ltspice/DCTransfer/DCTransfer.raw.h>
#include <data/ltspice/FFT/FFT.raw.h>
#include <data/ltspice/Windows/Wakeup.raw.h>
#include <data/ltspice/transient/LowPassFilter.raw.h>
#include <data/ltspice/transientDouble/LowPassFilter_transient_doubleFlag.raw.h>
#include <data/ngspice/ac_ascii.raw.h>
#include <data/ngspice/ac_binary.raw.h>
#include <data/ngspice/dc_ascii.raw.h>
#include <data/ngspice/dc_binary.raw.h>

#include <QFile>

const QString ngspicePath = QStringLiteral("data/ngspice"); // relative path
const QString ltspicePath = QStringLiteral("data/ltspice"); // relative path

#define NGSpiceFile QFINDTESTDATA(ngspicePath + QStringLiteral("/") + filename) // filename comes from the namespace
#define LTSpiceFile QFINDTESTDATA(ltspicePath + QStringLiteral("/") + filename) // filename comes from the namespace
#define NGSpiceRefDataFile QFINDTESTDATA(ngspicePath + QStringLiteral("/") + filename + QStringLiteral(".refdata")) // filename comes from the namespace
#define LTSpiceRefDataFile QFINDTESTDATA(ltspicePath + QStringLiteral("/") + filename + QStringLiteral(".refdata")) // filename comes from the namespace

#define READ_REFDATA(filename)                                                                                                                                 \
	auto filepath = QFINDTESTDATA(filename);                                                                                                                   \
	QFile f(filepath);                                                                                                                                         \
	QCOMPARE(f.open(QIODevice::ReadOnly), true);                                                                                                               \
	QVector<QStringList> refData;                                                                                                                              \
	while (!f.atEnd()) {                                                                                                                                       \
		QString line = QLatin1String(f.readLine().simplified());                                                                                               \
		refData.append(line.split(QLatin1Char(',')));                                                                                                          \
	}                                                                                                                                                          \
	QVERIFY(refData.count() > 0);

// Compare that the columns in the spreadsheet have the correct name and columnMode
#define COMPARE_COLUMN_NAMES_MODE(spreadsheet, columnNames, refColumnCount)                                                                                    \
	QCOMPARE(spreadsheet.columnCount(), refColumnCount);                                                                                                       \
	for (int i = 0; i < refColumnCount; i++) {                                                                                                                 \
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Double);                                                                     \
		QCOMPARE(spreadsheet.column(i)->name(), columnNames.at(i));                                                                                            \
	}

// Compare all data in the spreadsheet with the reference data
#define COMPARE_ROW_VALUES_START_END_ROW(spreadsheet, refData, refDataRowCount, refColumnCount, startRow, endRow)                                              \
	;                                                                                                                                                          \
	QCOMPARE(spreadsheet.columnCount(), refColumnCount);                                                                                                       \
	QCOMPARE(spreadsheet.rowCount(), endRow - startRow + 1);                                                                                                   \
	for (int row = startRow - 1; row < endRow; row++) {                                                                                                        \
		for (int col = 0; col < refColumnCount; col++) {                                                                                                       \
			QCOMPARE(spreadsheet.column(col)->valueAt(row - startRow + 1), refData.at(row).at(col).toDouble());                                                \
		}                                                                                                                                                      \
	}

// Compare all data in the spreadsheet with the reference data
#define COMPARE_ROW_VALUES(spreadsheet, refData, refDataRowCount, refColumnCount)                                                                              \
	;                                                                                                                                                          \
	QCOMPARE(spreadsheet.columnCount(), refColumnCount);                                                                                                       \
	QCOMPARE(spreadsheet.rowCount(), refDataRowCount);                                                                                                         \
	for (int row = 0; row < refData.count(); row++) {                                                                                                          \
		for (int col = 0; col < refColumnCount; col++) {                                                                                                       \
			QCOMPARE(spreadsheet.column(col)->valueAt(row), refData.at(row).at(col).toDouble());                                                               \
		}                                                                                                                                                      \
	}

// Float version
// Compare all data in the spreadsheet with the reference data
#define COMPARE_ROW_VALUES_FLOAT(spreadsheet, refData, refDataRowCount, refColumnCount)                                                                        \
	;                                                                                                                                                          \
	QCOMPARE(spreadsheet.columnCount(), refColumnCount);                                                                                                       \
	QCOMPARE(spreadsheet.rowCount(), refDataRowCount);                                                                                                         \
	for (int row = 0; row < refData.count(); row++) {                                                                                                          \
		for (int col = 0; col < refColumnCount; col++) {                                                                                                       \
			QVERIFY(qFuzzyCompare(static_cast<float>(spreadsheet.column(col)->valueAt(row)), refData.at(row).at(col).toFloat()));                              \
		}                                                                                                                                                      \
	}

void SpiceFilterTest::NgSpiceAsciiFileToBinaryFilter() {
	using namespace dc_ascii;

	const QString& ngFile = NGSpiceFile;
	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, false);
}

void SpiceFilterTest::NgSpiceBinaryFileToAsciiFilter() {
	using namespace dc_binary;

	const QString& ngFile = NGSpiceFile;
	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, true);
}

void SpiceFilterTest::NgSpiceDCAscii() {
	using namespace dc_ascii;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString& ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, false);

	SpiceFilter filter;
	auto res = filter.preview(ngFile, refData.count());

	QCOMPARE(res.length(), refData.length());
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::NgSpiceDCBinary() {
	using namespace dc_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString& ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
	auto res = filter.preview(ngFile, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::NgSpiceACAscii() {
	using namespace ac_ascii;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString& ngFile = NGSpiceFile;
	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, false);

	const int refColumnCount = refData.at(0).count();

	SpiceFilter filter;
	auto res = filter.preview(ngFile, refData.count());

	QCOMPARE(res.length(), refData.length());
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::NgSpiceACBinary() {
	using namespace ac_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString& ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
	auto res = filter.preview(ngFile, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet(QStringLiteral("Test"), false);
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
	const QString& ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, false);

	SpiceFilter filter;
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

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES_START_END_ROW(sheet, refData, refDataRowCount, refColumnCount, startRow, endRow);
}

void SpiceFilterTest::NgSpiceDCBinaryStartRowNotZero() {
	using namespace dc_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString& ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
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

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES_START_END_ROW(sheet, refData, refDataRowCount, refColumnCount, startRow, endRow);
}

void SpiceFilterTest::NgSpiceACAsciiStartRowNotZero() {
	using namespace ac_ascii;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString& ngFile = NGSpiceFile;
	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, false);

	const int refColumnCount = refData.at(0).count();

	SpiceFilter filter;
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

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES_START_END_ROW(sheet, refData, refDataRowCount, refColumnCount, startRow, endRow);
}

void SpiceFilterTest::NgSpiceACBinaryStartRowNotZero() {
	using namespace ac_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString& ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
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

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES_START_END_ROW(sheet, refData, refDataRowCount, refColumnCount, startRow, endRow);
}

// ########################################################################################################
//
// ########################################################################################################

void SpiceFilterTest::NgSpiceDCBinaryBulkReadNumberLines() {
	using namespace dc_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString& ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
	const int startRow = 31;
	const int endRow = 433;
	filter.setReaderBulkLineCount(100);
	filter.setStartRow(startRow);
	filter.setEndRow(endRow);
	auto res = filter.preview(ngFile, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = startRow - 1; i < startRow - 1 + numberPreviewData; i++) {
		QCOMPARE(res.at(i - startRow + 1), refData.at(i));
	}

	QString resFileInfoString = filter.fileInfoString(ngFile);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(ngFile, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES_START_END_ROW(sheet, refData, refDataRowCount, refColumnCount, startRow, endRow);
}

void SpiceFilterTest::NgSpiceACBinaryBulkReadNumberLines() {
	using namespace ac_binary;

	READ_REFDATA(NGSpiceRefDataFile);
	const QString& ngFile = NGSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(ngFile, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
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

	Spreadsheet sheet(QStringLiteral("Test"), false);
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
	const QString& file = LTSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(file, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
	auto res = filter.preview(file, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(file);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(file, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::LtSpiceTranBinary() {
	using namespace LowPassFilter_Transient;

	READ_REFDATA(LTSpiceRefDataFile);
	const QString& file = LTSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(file, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
	auto res = filter.preview(file, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++) {
		for (int j = 0; j < columnNames.length(); j++)
			QVERIFY(qFuzzyCompare(res.at(i).at(j).toFloat(), refData.at(i).at(j).toFloat()));
	}

	QString resFileInfoString = filter.fileInfoString(file);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(file, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	// Because the read data are floats, a float comparsion must be done, because
	// comparing float and double will not work properly
	COMPARE_ROW_VALUES_FLOAT(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::LtSpiceTranDoubleBinary() {
	using namespace LowPassFilter_transient_doubleFlag;

	READ_REFDATA(LTSpiceRefDataFile);
	const QString& file = LTSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(file, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
	auto res = filter.preview(file, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++)
		QCOMPARE(res.at(i), refData.at(i));

	QString resFileInfoString = filter.fileInfoString(file);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(file, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	COMPARE_ROW_VALUES(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::LtSpiceWakeup() {
	// The Spice file contains an additional option called
	// Backannotation which was not handled
	using namespace Wakeup;

	READ_REFDATA(LTSpiceRefDataFile);
	const QString& file = LTSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(file, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
	auto res = filter.preview(file, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++) {
		for (int j = 0; j < columnNames.length(); j++)
			QVERIFY(qFuzzyCompare(res.at(i).at(j).toFloat(), refData.at(i).at(j).toFloat()));
	}

	QString resFileInfoString = filter.fileInfoString(file);
	//	// For debugging purpose
	//	for (int i=0; i < resFileInfoString.length(); i++) {
	//		qDebug() << i << resFileInfoString.at(i) << refFileInfoString.at(i);
	//		QCOMPARE(resFileInfoString.at(i), refFileInfoString.at(i));
	//	}
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(file, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	// Because the read data are floats, a float comparsion must be done, because
	// comparing float and double will not work properly
	COMPARE_ROW_VALUES_FLOAT(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::DCTransfer() {
	// The Spice file contains an additional option called
	// Backannotation which was not handled
	using namespace DCTransfer;

	READ_REFDATA(LTSpiceRefDataFile);
	const QString& file = LTSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(file, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
	auto res = filter.preview(file, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++) {
		for (int j = 0; j < columnNames.length(); j++)
			QVERIFY(qFuzzyCompare(res.at(i).at(j).toFloat(), refData.at(i).at(j).toFloat()));
	}

	QString resFileInfoString = filter.fileInfoString(file);
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(file, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	// Because the read data are floats, a float comparsion must be done, because
	// comparing float and double will not work properly
	COMPARE_ROW_VALUES_FLOAT(sheet, refData, refDataRowCount, refColumnCount);
}

void SpiceFilterTest::FFT_From_TransientAnalysis() {
	// The Spice file contains an additional option called
	// Backannotation which was not handled
	using namespace FFT;

	READ_REFDATA(LTSpiceRefDataFile);
	const QString& file = LTSpiceFile;
	const int refColumnCount = refData.at(0).count();

	bool binary;
	QCOMPARE(SpiceFilter::isSpiceFile(file, &binary), true);
	QCOMPARE(binary, true);

	SpiceFilter filter;
	auto res = filter.preview(file, numberPreviewData);

	QCOMPARE(res.length(), numberPreviewData);
	for (int i = 0; i < res.length(); i++) {
		for (int j = 0; j < columnNames.length(); j++)
			QVERIFY(qFuzzyCompare(res.at(i).at(j).toFloat(), refData.at(i).at(j).toFloat()));
	}

	QString resFileInfoString = filter.fileInfoString(file);
	//	// For debugging purpose
	for (int i = 0; i < resFileInfoString.length(); i++) {
		qDebug() << i << resFileInfoString.at(i) << refFileInfoString.at(i);
		QCOMPARE(resFileInfoString.at(i), refFileInfoString.at(i));
	}
	QCOMPARE(resFileInfoString, refFileInfoString);

	Spreadsheet sheet(QStringLiteral("Test"), false);
	filter.readDataFromFile(file, &sheet, AbstractFileFilter::ImportMode::Replace);

	COMPARE_COLUMN_NAMES_MODE(sheet, columnNames, refColumnCount);

	// Because the read data are floats, a float comparsion must be done, because
	// comparing float and double will not work properly
	COMPARE_ROW_VALUES_FLOAT(sheet, refData, refDataRowCount, refColumnCount);
}

QTEST_MAIN(SpiceFilterTest)
