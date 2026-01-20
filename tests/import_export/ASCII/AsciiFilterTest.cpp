/*
	File                 : AsciiFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the ascii filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2022-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AsciiFilterTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/AsciiFilterPrivate.h"
#include "backend/datasources/filters/FilterStatus.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include <KCompressionDevice>
#include <QXmlStreamWriter>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

void AsciiFilterTest::initialization() {
	{
		AsciiFilter filter;
		auto p = filter.properties();

		p.automaticSeparatorDetection = false;
		p.separator = QStringLiteral(",");
		p.columnNamesRaw = QStringLiteral("Column1, Column2");
		p.columnModesString = QStringLiteral("Int, Int");
		p.headerEnabled = false;

		QCOMPARE(filter.initialize(p).success(), true);
	}

	// One column mode to much
	{
		AsciiFilter filter;
		auto p = filter.properties();

		p.automaticSeparatorDetection = false;
		p.separator = QStringLiteral(",");
		p.columnNamesRaw = QStringLiteral("Column1, Column2");
		p.columnModesString = QStringLiteral("Int, Int, Double");
		p.headerEnabled = false;

		QCOMPARE(filter.initialize(p).success(), false);
	}

	// On column name to much
	{
		AsciiFilter filter;
		auto p = filter.properties();

		p.automaticSeparatorDetection = false;
		p.separator = QStringLiteral(",");
		p.columnNamesRaw = QStringLiteral("Column1, Column2, Column3");
		p.columnModesString = QStringLiteral("Int, Int");
		p.headerEnabled = false;

		QCOMPARE(filter.initialize(p).success(), false);
	}

	// No column names, they get determined automatically
	{
		AsciiFilter filter;
		{
			auto p = filter.properties();

			p.automaticSeparatorDetection = false;
			p.columnNamesRaw = QStringLiteral("");
			p.columnModesString = QStringLiteral("Int, Int");
			p.headerEnabled = false;

			QCOMPARE(filter.initialize(p).success(), true);
		}
		auto p = filter.properties();
		QCOMPARE(p.columnModes.size(), 2);
		QCOMPARE(p.columnNames.size(), 2);
	}

	// No column modes
	{
		AsciiFilter filter;
		auto p = filter.properties();

		p.automaticSeparatorDetection = false;
		p.columnNamesRaw = QStringLiteral("Column1, Column2");
		p.columnModesString = QStringLiteral("");
		p.headerEnabled = false;

		QCOMPARE(filter.initialize(p).success(), false);
	}
}

void AsciiFilterTest::lineCount() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/empty_file.txt"));
	QCOMPARE(AsciiFilter::lineCount(fileName), 0);

	const QString& fileName2 = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));
	QCOMPARE(AsciiFilter::lineCount(fileName2), 5);
	QCOMPARE(AsciiFilter::lineCount(fileName2, 3), 3);
	QCOMPARE(AsciiFilter::lineCount(fileName2, 10), 5);

	const QString& fileName3 = QFINDTESTDATA(QLatin1String("data/numeric_data_no-newline.txt"));
	QCOMPARE(AsciiFilter::lineCount(fileName3), 5);
	QCOMPARE(AsciiFilter::lineCount(fileName3, 3), 3);
	QCOMPARE(AsciiFilter::lineCount(fileName3, 10), 5);
}

// column modes
void AsciiFilterTest::read_HeaderEnabled_tooLessColumnModes() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.headerEnabled = true;
	p.columnModesString = QStringLiteral("Int"); // Too less, 2 expected
	filter.setProperties(p);

	QVERIFY(filter.lastError().isEmpty());
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	QVERIFY(!filter.lastError().isEmpty());
}

void AsciiFilterTest::read_HeaderEnabled_tooManyColumnModes() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.headerEnabled = true;
	p.columnModesString = QStringLiteral("Int, Int, Int"); // 2 expected
	filter.setProperties(p);

	QVERIFY(filter.lastError().isEmpty());
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	QVERIFY(!filter.lastError().isEmpty());
}

void AsciiFilterTest::read_HeaderDisabled_tooLessColumnModes() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("Column1, Column2");
	p.columnModesString = QStringLiteral("Int"); // 2 expected
	filter.setProperties(p);

	QVERIFY(filter.lastError().isEmpty());
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	QVERIFY(!filter.lastError().isEmpty());
}

void AsciiFilterTest::read_HeaderDisabled_tooManyColumnModes() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("Column1, Column2");
	p.columnModesString = QStringLiteral("Int, Int, Int"); // 2 expected
	filter.setProperties(p);

	QVERIFY(filter.lastError().isEmpty());
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	QVERIFY(!filter.lastError().isEmpty());
}

void AsciiFilterTest::read_HeaderDisabled_NotMatchingImport() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("Column1, Column2, Column3"); // 2 expected
	p.columnModesString = QStringLiteral("Int, Int, Int"); // 2 expected
	filter.setProperties(p);

	QVERIFY(filter.lastError().isEmpty());
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	QVERIFY(!filter.lastError().isEmpty());
}

void AsciiFilterTest::read_HeaderDisabled_tooLessColumnNames() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("Column1"); // 2 expected
	p.columnModesString = QStringLiteral("Int, Int");
	filter.setProperties(p);

	QVERIFY(filter.lastError().isEmpty());
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	QVERIFY(!filter.lastError().isEmpty());
}

/*!
 * \brief AsciiFilterTest::singleColumn
 * Testfile contains only a single column
 */
void AsciiFilterTest::singleColumn() {
	QStringList fileContent = {
		QStringLiteral("123"),
		QStringLiteral("234"),
		QStringLiteral("345"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	AsciiFilter filter;
	auto p = filter.properties();
	p.headerEnabled = false;
	p.intAsDouble = false;
	filter.setProperties(p);

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	QVERIFY(filter.lastError().isEmpty());

	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->name(), i18n("Column") + QStringLiteral(" 1"));
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->integerAt(0), 123);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 234);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 345);
}

void AsciiFilterTest::singleColumnSimplifyWhitespaceEnabled() {
	QStringList fileContent = {
		QStringLiteral("123"),
		QStringLiteral("234"),
		QStringLiteral("345"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	AsciiFilter filter;
	auto p = filter.properties();
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.simplifyWhitespaces = true;
	filter.setProperties(p);

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	QVERIFY(filter.lastError().isEmpty());

	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.column(0)->name(), i18n("Column") + QStringLiteral(" 1"));
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->integerAt(0), 123);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 234);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 345);
}

// ##############################################################################
// #################  handling of empty and sparse files ########################
// ##############################################################################
void AsciiFilterTest::testEmptyFileAppend() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	const int rowCount = spreadsheet.rowCount();
	const int colCount = spreadsheet.columnCount();
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/empty_file.txt"));

	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Append);

	QCOMPARE(spreadsheet.rowCount(), rowCount);
	QCOMPARE(spreadsheet.columnCount(), colCount);
}

void AsciiFilterTest::testEmptyFilePrepend() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	const int rowCount = spreadsheet.rowCount();
	const int colCount = spreadsheet.columnCount();
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/empty_file.txt"));

	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Prepend);

	QCOMPARE(spreadsheet.rowCount(), rowCount);
	QCOMPARE(spreadsheet.columnCount(), colCount);
}

void AsciiFilterTest::testEmptyFileReplace() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	const int rowCount = spreadsheet.rowCount();
	const int colCount = spreadsheet.columnCount();
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/empty_file.txt"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), rowCount);
	QCOMPARE(spreadsheet.columnCount(), colCount);
}

void AsciiFilterTest::testEmptyLines01() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/empty_lines_01.txt"));

	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(";");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	filter.setProperties(p);

	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("values"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 2);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 4);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 9);
	QCOMPARE(spreadsheet.column(2)->integerAt(0), 10);
	QCOMPARE(spreadsheet.column(2)->integerAt(1), 40);
	QCOMPARE(spreadsheet.column(2)->integerAt(2), 90);
}

void AsciiFilterTest::testSparseFile01() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/sparse_file_01.txt"));

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(",");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.simplifyWhitespaces = true;
	filter.setProperties(p);

	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("N"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Col1"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Col2"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 1);

	QCOMPARE(spreadsheet.column(2)->integerAt(0), 2);
	QCOMPARE(spreadsheet.column(2)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(2)->integerAt(2), 0);
}

void AsciiFilterTest::testSparseFile02() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/sparse_file_02.txt"));

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(",");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.simplifyWhitespaces = true;
	p.nanValue = NAN;
	p.skipEmptyParts = false;
	filter.setProperties(p);

	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("N"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Col1"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Col2"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.);
	QCOMPARE((bool)std::isnan(spreadsheet.column(1)->valueAt(1)), true);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 1.);

	QCOMPARE(spreadsheet.column(2)->valueAt(0), 2.);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 2.);
	QCOMPARE((bool)std::isnan(spreadsheet.column(2)->valueAt(2)), true);
}

void AsciiFilterTest::testSparseFile03() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/sparse_file_03.txt"));

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(",");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.simplifyWhitespaces = true;
	p.nanValue = 0; // Nan value to zero
	p.skipEmptyParts = false;
	filter.setProperties(p);

	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 4);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("N"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Col1"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Col2"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->integerAt(3), 0);

	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0.);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 1.);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 0.);

	QCOMPARE(spreadsheet.column(2)->valueAt(0), 2.);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 2.);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0.);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 3.);
}

void AsciiFilterTest::testFileEndingWithoutLinebreak() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/file_ending_without_line_break.txt"));

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral("\t");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	filter.setProperties(p);

	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 2);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("some data"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("more data"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->integerAt(3), 4);
	QCOMPARE(spreadsheet.column(0)->integerAt(4), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 4);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 9);
	QCOMPARE(spreadsheet.column(1)->integerAt(3), 16);
	QCOMPARE(spreadsheet.column(1)->integerAt(4), 25);
}

// ##############################################################################
// ################################  header handling ############################
// ##############################################################################
void AsciiFilterTest::testHeaderDisabled() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("x, y");
	p.intAsDouble = false;
	filter.setProperties(p);

	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 7);
}

void AsciiFilterTest::intAsDouble() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("x, y");
	p.intAsDouble = true;
	filter.setProperties(p);

	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	VALUES_EQUAL(spreadsheet.column(0)->valueAt(0), 1.);
	VALUES_EQUAL(spreadsheet.column(0)->valueAt(1), 2.);
	VALUES_EQUAL(spreadsheet.column(0)->valueAt(2), 3.);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	VALUES_EQUAL(spreadsheet.column(1)->valueAt(0), 5.);
	VALUES_EQUAL(spreadsheet.column(1)->valueAt(1), 6.);
	VALUES_EQUAL(spreadsheet.column(1)->valueAt(2), 7.);
}

void AsciiFilterTest::testFirstLineHeader() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.columnNamesRaw = QStringLiteral("");
	p.intAsDouble = false;
	filter.setProperties(p);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 2); // out of 3 rows one row is used for the column names (header)
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("5"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 3);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 7);
}

void AsciiFilterTest::testMissingParts() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1;5;4"),
		QStringLiteral("2;6;6"),
		QStringLiteral("3;7;6"),
		QStringLiteral("1;5;4"),
		QStringLiteral("2;6;6"),
		QStringLiteral("3;7;6"),
		QStringLiteral("1;5;4"),
		QStringLiteral("2;6;6"),
		QStringLiteral("3;7;6"),
		QStringLiteral("1;5;4"),
		QStringLiteral("2;;6"), // Missing value
	};
	QVERIFY(AsciiFilterPrivate::m_dataTypeLines < fileContent.size());

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("x, y, z");
	p.intAsDouble = false;
	p.skipEmptyParts = false;
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", fileContent);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 11);
	QCOMPARE(spreadsheet.columnCount(), 3); // one column name was specified, we import only one column

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 7);
	QCOMPARE(spreadsheet.column(1)->integerAt(3), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(4), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(5), 7);
	QCOMPARE(spreadsheet.column(1)->integerAt(6), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(7), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(8), 7);
	QCOMPARE(spreadsheet.column(1)->integerAt(9), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(10), 0); // Not available in file. Therefore it is zero
}

void AsciiFilterTest::testMissingPartsSkip() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	QStringList fileContent = {
		QStringLiteral("1;;4"),
		QStringLiteral("2;;6"),
		QStringLiteral("3;;6"),
		QStringLiteral("1;;4"),
		QStringLiteral("2;;6"),
		QStringLiteral("3;;6"),
		QStringLiteral("1;;4"),
		QStringLiteral("2;;6"),
		QStringLiteral("3;;6"),
		QStringLiteral("1;;4"),
		QStringLiteral("2;;6"), // Missing value
	};
	QVERIFY(AsciiFilterPrivate::m_dataTypeLines < fileContent.size());
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("x, z");
	p.intAsDouble = false;
	p.skipEmptyParts = true;
	filter.setProperties(p);

	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 11);
	QCOMPARE(spreadsheet.columnCount(), 2); // one column name was specified, we import only one column

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 4);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(3), 4);
	QCOMPARE(spreadsheet.column(1)->integerAt(4), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(5), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(6), 4);
	QCOMPARE(spreadsheet.column(1)->integerAt(7), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(8), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(9), 4);
	QCOMPARE(spreadsheet.column(1)->integerAt(10), 6);
}

void AsciiFilterTest::testImportSingleColumn() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("x");
	filter.setProperties(p);

	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// New Asciifilter returns an error when the number of columns for the header do not match the content

	// QCOMPARE(spreadsheet.rowCount(), 3);
	// QCOMPARE(spreadsheet.columnCount(), 1); // one column name was specified, we import only one column
	// QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));

	// QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	// VALUES_EQUAL(spreadsheet.column(0)->valueAt(0), 1.);
	// VALUES_EQUAL(spreadsheet.column(0)->valueAt(1), 2.);
	// VALUES_EQUAL(spreadsheet.column(0)->valueAt(2), 3.);
}

void AsciiFilterTest::commaSeparatedWhiteSpace() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("1, 5"),
		QStringLiteral("2, 6"),
		QStringLiteral("3, 7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(";"); // Doesn't matter
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("x,y");
	p.intAsDouble = false;
	p.simplifyWhitespaces = true;
	filter.setProperties(p);

	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2); // two names were specified -> we import two columns
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 7);
}

void AsciiFilterTest::tooManyHeaders() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const auto spreadSheetRowCount = spreadsheet.rowCount();
	const auto spreadsheetColumnCount = spreadsheet.columnCount();
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.simplifyWhitespaces = true;
	p.columnNamesRaw = QStringLiteral("x, y, z"); // Too many
	filter.setProperties(p);

	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// Nothing changed, because too many headers specified
	QCOMPARE(spreadsheet.rowCount(), spreadSheetRowCount);
	QCOMPARE(spreadsheet.columnCount(), spreadsheetColumnCount);
	// QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	// QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));
}

/*!
 * test with a file containing the header in the second line
 * with a subsequent comment line ignored by using startRow.
 */
void AsciiFilterTest::testHeaderLine2DataLine4() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("Created: 13.02.2020	20:08:54"),
		QStringLiteral("counter	t[min]	#1ch1"),
		QStringLiteral("Start: 13.02.2020	20:14:54"),
		QStringLiteral("1	0.0513	0.3448"),
		QStringLiteral("2	0.1005	0.3418"),
		QStringLiteral("3	0.1516	0.3433"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral("TAB");
	p.headerEnabled = true;
	p.headerLine = 2;
	p.startRow = 2;
	p.intAsDouble = false;
	filter.setProperties(p);

	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 3);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("counter"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("t[min]"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("#1ch1"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.0513);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 0.3448);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0.1005);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0.3418);

	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 0.1516);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0.3433);
}

/*!
 * test with a file containing the header in the second line
 * with a subsequent comment line ignored by using the "commentCharacter"
 * property
 */
void AsciiFilterTest::testHeaderLine2DataLine4_CommentLine() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("Created: 13.02.2020	20:08:54"),
		QStringLiteral("counter	t[min]	#1ch1"),
		QStringLiteral("Start: 13.02.2020	20:14:54"),
		QStringLiteral("1	0.0513	0.3448"),
		QStringLiteral("2	0.1005	0.3418"),
		QStringLiteral("3	0.1516	0.3433"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral("TAB");
	p.headerEnabled = true;
	p.headerLine = 2;
	p.startRow = 1;
	p.commentCharacter = QStringLiteral("Start"); // Instead of setting startRow to 2, use the Start keyword as "commentCharacter"
	p.intAsDouble = false;
	filter.setProperties(p);

	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 3);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("counter"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("t[min]"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("#1ch1"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.0513);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 0.3448);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0.1005);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0.3418);

	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 0.1516);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0.3433);
}

/*!
 * the header contains spaces in the column names, values are tab separated.
 * when using "auto" for the separator characters, the tab character has to
 * be properly recognized and used.
 */
// This is anymore possible, because the space will be used as separator. The user has to set the settings manually
// void AsciiFilterTest::testHeader08() {
// 	Spreadsheet spreadsheet(QStringLiteral("test"), false);
// 	AsciiFilter filter;
// 	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_tab_with_header_with_spaces.txt"));

// 	auto p = filter.properties();
// 	p.automaticSeparatorDetection = false;
// 	p.separator = QStringLiteral("TAB");
// 	p.headerEnabled = true;
// 	p.headerLine = 2;
// 	p.startRow = 1;
// 	p.commentCharacter = QStringLiteral("Start"); // Instead of setting startRow to 2, use the Start keyword as "commentCharacter"
// 	p.intAsDouble = false;
// 	filter.setProperties(p);

// 	filter.setHeaderEnabled(true);
// 	filter.setHeaderLine(1);
// 	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

// 	// spreadsheet size
// 	QCOMPARE(spreadsheet.columnCount(), 2);
// 	QCOMPARE(spreadsheet.rowCount(), 2);

// 	// column names
// 	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("first column"));
// 	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("second column"));

// 	// data types
// 	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
// 	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

// 	// values
// 	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
// 	QCOMPARE(spreadsheet.column(1)->integerAt(0), 2);

// 	QCOMPARE(spreadsheet.column(0)->integerAt(1), 3);
// 	QCOMPARE(spreadsheet.column(1)->integerAt(1), 4);
// }

/*!
 * test the handling of duplicated columns names provided by the user.
 */
void AsciiFilterTest::testHeaderDuplicateNames_HeaderDisabled() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("x, x");
	filter.setProperties(p);

	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("x 1")); // the duplicated name was renamed
}

/*!
 * test the handling of duplicated columns in the file to be imported.
 */
void AsciiFilterTest::testHeaderDuplicateNames_HeaderEnabled() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("x;x"),
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	filter.setProperties(p);

	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("x 1")); // the duplicated name was renamed

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 7);
}

/*!
 * test the handling of duplicated columns in the file to be imported.
 */
void AsciiFilterTest::testHeaderTwoImportsReverseColumnNames() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	{
		AsciiFilter filter;
		auto p = filter.properties();
		p.automaticSeparatorDetection = false;
		p.separator = QStringLiteral(" ");
		p.headerEnabled = true;
		p.headerLine = 1;
		p.intAsDouble = false;
		filter.setProperties(p);

		QStringList fileContent = {
			QStringLiteral("A B"),
			QStringLiteral("1 2"),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

		QCOMPARE(spreadsheet.rowCount(), 1);
		QCOMPARE(spreadsheet.columnCount(), 2);
		QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("A"));
		QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("B"));
	}

	{
		AsciiFilter filter;
		auto p = filter.properties();
		p.automaticSeparatorDetection = false;
		p.separator = QStringLiteral(" ");
		p.headerEnabled = true;
		p.headerLine = 1;
		p.intAsDouble = false;
		filter.setProperties(p);

		// import the second file with reversed column names into the same spreadsheet
		QStringList fileContent = {
			QStringLiteral("B A"), // Reversed A, B
			QStringLiteral("1 2"),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

		QCOMPARE(spreadsheet.rowCount(), 1);
		QCOMPARE(spreadsheet.columnCount(), 2);
		QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("B"));
		QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("A"));
	}
}

/*!
 * test the handling of column names with and without header
 */
void AsciiFilterTest::testHeader11a() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	{
		AsciiFilter filter;
		auto p = filter.properties();
		p.automaticSeparatorDetection = false;
		p.separator = QStringLiteral(" ");
		p.headerEnabled = true;
		p.headerLine = 1;
		p.intAsDouble = false;
		filter.setProperties(p);

		QStringList fileContent = {
			QStringLiteral("A B"),
			QStringLiteral("1 2"),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

		QCOMPARE(spreadsheet.rowCount(), 1);
		QCOMPARE(spreadsheet.columnCount(), 2);
		QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("A"));
		QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("B"));
	}

	{
		AsciiFilter filter;
		auto p = filter.properties();
		p.automaticSeparatorDetection = false;
		p.separator = QStringLiteral(" ");
		p.headerEnabled = false;
		p.headerLine = 1;
		p.columnNamesRaw = QStringLiteral("Column1, Column2");
		p.intAsDouble = false;
		filter.setProperties(p);

		QStringList fileContent = {
			QStringLiteral("0 5"),
			QStringLiteral("1 2"),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

		QCOMPARE(spreadsheet.rowCount(), 2);
		QCOMPARE(spreadsheet.columnCount(), 2);
		QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Column1"));
		QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Column2"));
	}
}

/*!
 * test column modes when header is in second line
 */
void AsciiFilterTest::testHeader12() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(" ");
	p.headerEnabled = true;
	p.headerLine = 2;
	p.intAsDouble = false;
	filter.setProperties(p);

	QStringList fileContent = {
		QStringLiteral("ignore"),
		QStringLiteral("x,y"),
		QStringLiteral("1,2"),
	};

	QString savePath;
	SAVE_FILE("testfile", fileContent);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 1);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
}

// ##############################################################################
// #####################  handling of different read ranges #####################
// ##############################################################################
namespace {
const QStringList numeric_data = {
	QStringLiteral("1.716299 -0.485527 -0.288690"),
	QStringLiteral("1.716299 -0.476371 -0.274957"),
	QStringLiteral("1.711721 -0.485527 -0.293267"),
	QStringLiteral("1.711721 -0.480949 -0.293267"),
	QStringLiteral("1.716299 -0.494682 -0.284112"),
};
}

void AsciiFilterTest::testColumnRange00() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.columnNamesRaw = QStringLiteral("x,y,z");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// no ranges specified, all rows and columns have to be read
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first line
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1.716299);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testCreateIndex() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.createIndex = true;
	p.columnNamesRaw = QStringLiteral("x,y,z");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// no ranges specified, all rows and columns have to be read plus the additional column for the index
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 4);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first line
	QCOMPARE(spreadsheet.column(0)->bigIntAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.716299);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), -0.288690);

	QCOMPARE(spreadsheet.column(0)->bigIntAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->bigIntAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->bigIntAt(3), 4);
	QCOMPARE(spreadsheet.column(0)->bigIntAt(4), 5);
}

void AsciiFilterTest::testCreateIndexAndTimestamp() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.createIndex = true;
	p.createTimestamp = true;
	p.columnNamesRaw = QStringLiteral("x,y,z");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// no ranges specified, all rows and columns have to be read plus the additional column for the index
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 5);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first line
	QCOMPARE(spreadsheet.column(0)->bigIntAt(0), 1);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 1.716299);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(4)->valueAt(0), -0.288690);

	// Index
	QCOMPARE(spreadsheet.column(0)->bigIntAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->bigIntAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->bigIntAt(3), 4);
	QCOMPARE(spreadsheet.column(0)->bigIntAt(4), 5);

	// Timestamp
	// TODO: maybe checking also the value that they are monotonic increasing?
	QVERIFY(spreadsheet.column(1)->dateTimeAt(1).isValid());
	QVERIFY(spreadsheet.column(1)->dateTimeAt(2).isValid());
	QVERIFY(spreadsheet.column(1)->dateTimeAt(3).isValid());
	QVERIFY(spreadsheet.column(1)->dateTimeAt(4).isValid());
}

void AsciiFilterTest::testStartColumn() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.createIndex = false;
	p.startColumn = 2;
	p.endColumn = 4;
	p.columnNamesRaw = QStringLiteral("x,y");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// read all rows and the last two columns only
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 2);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first line
	QCOMPARE(spreadsheet.column(0)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testStartColumn_IndexColumn() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.createIndex = true;
	p.startColumn = 2;
	p.endColumn = 4;
	p.columnNamesRaw = QStringLiteral("x,y");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// read all rows and the last two columns only plus the additional column for the index
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first line
	QCOMPARE(spreadsheet.column(0)->bigIntAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testLastColumnOnly() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.startColumn = 3;
	p.endColumn = 3;
	p.columnNamesRaw = QStringLiteral("z");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// read all rows and the last column only
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 1);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first line
	QCOMPARE(spreadsheet.column(0)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testWrongColumnRange_StartLargerEnd() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const auto spreadsheetRowCount = spreadsheet.rowCount();
	const auto spreadsheetColumnCount = spreadsheet.columnCount();
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.startColumn = 3;
	p.endColumn = 1; // Smaller than startColumn
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// Nothing changed
	QCOMPARE(spreadsheet.columnCount(), spreadsheetColumnCount);
	QCOMPARE(spreadsheet.rowCount(), spreadsheetRowCount);
}

void AsciiFilterTest::testWrongColumnRange() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.startColumn = 3;
	p.endColumn = 4; // There is only one column left, so ignoring!
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// Reading only the third column, because 4 does not exist
	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 5);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(0)->valueAt(0), -0.288690);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), -0.274957);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), -0.293267);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), -0.293267);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), -0.284112);
}

void AsciiFilterTest::testWrongColumnRange_IndexColumn() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.createIndex = true; // Index enabled
	p.startColumn = 3;
	p.endColumn = 4; // There is only one column left, so ignoring!
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// Wrong column range, so nothing changed
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 2);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 4);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 5);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.288690);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), -0.274957);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), -0.293267);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), -0.293267);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), -0.284112);
}

void AsciiFilterTest::testRowRange00() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.startRow = 3;
	p.endRow = 5;
	p.columnNamesRaw = QStringLiteral("x,y,z");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// three rows to read
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first and for the last lines
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1.711721);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.293267);

	QCOMPARE(spreadsheet.column(0)->valueAt(2), 1.716299);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), -0.494682);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), -0.284112);
}

/*!
 * \brief AsciiFilterTest::testRowRange01
 */
void AsciiFilterTest::testRowRange_EndRowLargerThanContent() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.startRow = 3;
	p.endRow = 10;
	p.columnNamesRaw = QStringLiteral("x,y,z");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// end row larger than the number of available rows, three rows to read
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first and for the last lines
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1.711721);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.293267);

	QCOMPARE(spreadsheet.column(0)->valueAt(2), 1.716299);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), -0.494682);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), -0.284112);
}

void AsciiFilterTest::testRowRange02() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	auto p = filter.defaultProperties();
	p.automaticSeparatorDetection = true;
	p.headerEnabled = false;
	p.startRow = 3;
	p.endRow = 1;
	filter.setProperties(p);

	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// start bigger than end, no rows to read
	// wrong row range specified (start>end), nothing to read,
	// spreadsheet is not touched, default number of rows and columns
	// TODO: this is inconsistent with the handling for columns, see testColumnRange05()
	QCOMPARE(spreadsheet.rowCount(), 100);
	QCOMPARE(spreadsheet.columnCount(), 2);
}

void AsciiFilterTest::testRowColumnRange00() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.startRow = 3;
	p.endRow = 10;
	p.startColumn = 2;
	p.endColumn = 3;
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// three rows and two columns to read
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first and for the last lines
	QCOMPARE(spreadsheet.column(0)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.293267);

	QCOMPARE(spreadsheet.column(0)->valueAt(2), -0.494682);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), -0.284112);
}

// ##############################################################################
// #####################  handling of different separators ######################
// ##############################################################################

/*!
 * \brief AsciiFilterTest::testSpaceSeparator
 * Automatic space as separator detection
 */
void AsciiFilterTest::testSpaceSeparator() {
	const QStringList data = {
		QStringLiteral("Column1 Column2"),
		QStringLiteral("123 345"),
		QStringLiteral("78120.123 23498.238"),
		QStringLiteral("345.34 234.43"),
	};

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	QString savePath;
	SAVE_FILE("testfile", data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->name(), QStringLiteral("Column1"));
	VALUES_EQUAL(spreadsheet.column(0)->valueAt(0), 123.);
	VALUES_EQUAL(spreadsheet.column(0)->valueAt(1), 78120.123);
	VALUES_EQUAL(spreadsheet.column(0)->valueAt(2), 345.34);

	QCOMPARE(spreadsheet.column(1)->name(), QStringLiteral("Column2"));
	VALUES_EQUAL(spreadsheet.column(1)->valueAt(0), 345.);
	VALUES_EQUAL(spreadsheet.column(1)->valueAt(1), 23498.238);
	VALUES_EQUAL(spreadsheet.column(1)->valueAt(2), 234.43);
}

void AsciiFilterTest::testSpaceSeparatorSimplifyWhiteSpace() {
	const QStringList data = {
		QStringLiteral("Column1 Column2"),
		QStringLiteral("123 345"),
		QStringLiteral("78120.123 23498.238"),
		QStringLiteral("345.34 234.43"),
	};

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto properties = filter.properties();
	properties.simplifyWhitespaces = true;
	filter.setProperties(properties);

	QString savePath;
	SAVE_FILE("testfile", data);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->name(), QStringLiteral("Column1"));
	VALUES_EQUAL(spreadsheet.column(0)->valueAt(0), 123.);
	VALUES_EQUAL(spreadsheet.column(0)->valueAt(1), 78120.123);
	VALUES_EQUAL(spreadsheet.column(0)->valueAt(2), 345.34);

	QCOMPARE(spreadsheet.column(1)->name(), QStringLiteral("Column2"));
	VALUES_EQUAL(spreadsheet.column(1)->valueAt(0), 345.);
	VALUES_EQUAL(spreadsheet.column(1)->valueAt(1), 23498.238);
	VALUES_EQUAL(spreadsheet.column(1)->valueAt(2), 234.43);
}

// ##############################################################################
// #####################################  quoted strings ########################
// ##############################################################################

namespace {
const QStringList quoted_strings = {
	QStringLiteral("\"a\", \"1000\", \"201811\", \"1.1\""),
	QStringLiteral("\"ab\", \"2000\", \"201812\", \"1.2\""),
	QStringLiteral("\"abc\", \"3000\", \"201901\", \"1.3\""),
};
}

void AsciiFilterTest::testQuotedStrings00() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.intAsDouble = false;
	p.removeQuotes = true;
	p.simplifyWhitespaces = true;
	p.columnNamesRaw = QStringLiteral("w,x,y,z");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", quoted_strings);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 4);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("a"));
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 1000);
	QCOMPARE(spreadsheet.column(2)->integerAt(0), 201811);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 1.1);

	QCOMPARE(spreadsheet.column(0)->textAt(1), QLatin1String("ab"));
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 2000);
	QCOMPARE(spreadsheet.column(2)->integerAt(1), 201812);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 1.2);

	QCOMPARE(spreadsheet.column(0)->textAt(2), QLatin1String("abc"));
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 3000);
	QCOMPARE(spreadsheet.column(2)->integerAt(2), 201901);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 1.3);
}

void AsciiFilterTest::testQuotedStrings01() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const QStringList content = {
		QStringLiteral("\"col1\", \"col2\", \"col3\", \"col4\""),
		QStringLiteral("\"a\", \"1000\", \"201811\", \"1.1\""),
		QStringLiteral("\"ab\", \"2000\", \"201812\", \"1.2\""),
		QStringLiteral("\"abc\", \"3000\", \"201901\", \"1.3\""),
	};

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(",");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.removeQuotes = true;
	p.simplifyWhitespaces = true;
	p.columnNamesRaw = QStringLiteral("w,x,y,z");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// three rows and two columns to read
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 4);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("col1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("col2"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("col3"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("col4"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("a"));
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 1000);
	QCOMPARE(spreadsheet.column(2)->integerAt(0), 201811);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 1.1);

	QCOMPARE(spreadsheet.column(0)->textAt(1), QLatin1String("ab"));
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 2000);
	QCOMPARE(spreadsheet.column(2)->integerAt(1), 201812);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 1.2);

	QCOMPARE(spreadsheet.column(0)->textAt(2), QLatin1String("abc"));
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 3000);
	QCOMPARE(spreadsheet.column(2)->integerAt(2), 201901);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 1.3);
}

void AsciiFilterTest::testQuotedStrings02() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const QStringList content = {
		QStringLiteral("\"a\", \"1000\", \"201811\", \"1.1\""),
	};

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.removeQuotes = true;
	p.simplifyWhitespaces = true;
	p.columnNamesRaw = QStringLiteral("w,x,y,z");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 1);
	QCOMPARE(spreadsheet.columnCount(), 4);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("a"));
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 1000);
	QCOMPARE(spreadsheet.column(2)->integerAt(0), 201811);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 1.1);
}

void AsciiFilterTest::testQuotedStrings03() {
	// TODO: really required? Same as testQuotedStrings01

	// Spreadsheet spreadsheet(QStringLiteral("test"), false);
	// Old::AsciiFilter filter;
	// const QString& fileName = QFINDTESTDATA(QLatin1String("data/quoted_strings_one_line_with_header.txt"));

	// filter.setSeparatingCharacter(QStringLiteral(","));
	// filter.setHeaderEnabled(true);
	// filter.setHeaderLine(1);
	// filter.setsimplifyWhitespaces(true);
	// filter.setremoveQuotes(true);
	// filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// // three rows and two columns to read
	// // 	QCOMPARE(spreadsheet.rowCount(), 1);
	// // 	QCOMPARE(spreadsheet.columnCount(), 4);

	// // column names
	// QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("col1"));
	// QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("col2"));
	// QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("col3"));
	// QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("col4"));

	// // data types
	// QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	// QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	// QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	// QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);

	// // values
	// QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("a"));
	// QCOMPARE(spreadsheet.column(1)->integerAt(0), 1000);
	// QCOMPARE(spreadsheet.column(2)->integerAt(0), 201811);
	// QCOMPARE(spreadsheet.column(3)->valueAt(0), 1.1);
}

/*!
 * test quoted text having separators inside - the text between quotes shouldn't be split into separate columns.
 */
void AsciiFilterTest::testQuotedStrings04() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	const QStringList content = {
		QStringLiteral("id, text, value"),
		QStringLiteral("1, \"some text, having a comma, and yet another comma\", 1.0"),
		QStringLiteral("2, \"more text\", 2.0"),
	};

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(",");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.removeQuotes = true;
	p.simplifyWhitespaces = true;
	p.columnNamesRaw = QStringLiteral("w,x,y,z");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 2);
	QCOMPARE(spreadsheet.columnCount(), 3);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("id"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("text"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("value"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->textAt(0), QLatin1String("some text, having a comma, and yet another comma"));
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 1.0);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(1)->textAt(1), QLatin1String("more text"));
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 2.0);
}

/*!
 * test quoted text having separators inside - a JSON file has a similar structure and we shouldn't crash because of this "wrong" data.
 */
void AsciiFilterTest::testIvalidFile_Json() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/object.json"));

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral("NotAvailable"); // Sure that this is not available
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.removeQuotes = true;
	p.simplifyWhitespaces = true;
	p.columnNamesRaw = QStringLiteral("w,x,y,z");
	filter.setProperties(p);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// everything should be read into one single text column.
	// the actual content is irrelevant, we just need to make sure we don't crash because of such wrong content
	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
}

// ##############################################################################
// ################################## locales ###################################
// ##############################################################################
void AsciiFilterTest::testUtf8Cyrillic() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/utf8_cyrillic.txt"));

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.removeQuotes = true;
	p.simplifyWhitespaces = false;
	p.columnNamesRaw = QStringLiteral("w,x,y,z");
	filter.setProperties(p);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QString::fromUtf8("перший_стовпець"));
	QCOMPARE(spreadsheet.column(1)->name(), QString::fromUtf8("другий_стовпець"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->rowCount(), 2);
	QCOMPARE(spreadsheet.column(1)->rowCount(), 2);

	// values
	QCOMPARE(spreadsheet.column(0)->textAt(0), QString::fromUtf8("тест1"));
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 1);

	QCOMPARE(spreadsheet.column(0)->textAt(1), QString::fromUtf8("тест2"));
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 2);
}

void AsciiFilterTest::testUtf16NotSupported() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/utf16.txt"));

	AsciiFilter filter;

	// preview
	filter.preview(fileName, 100);
	QCOMPARE(filter.d_ptr->lastStatus.type(), Status::Type::UTF16NotSupported);

	// read
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	QCOMPARE(filter.d_ptr->lastStatus.type(), Status::Type::UTF16NotSupported);
}

// ##############################################################################
// ###############################  skip comments ###############################
// ##############################################################################
void AsciiFilterTest::testMultilineComment() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/multi_line_comment.txt"));

	const QStringList content = {
		QStringLiteral("# this"),
		QStringLiteral("# is"),
		QStringLiteral("# a"),
		QStringLiteral("# multi-line"),
		QStringLiteral("# comment"),
		QStringLiteral(""), // Empty line
		QStringLiteral("1,1.1"),
		QStringLiteral("2,2.2"),
	};

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.removeQuotes = true;
	p.simplifyWhitespaces = true;
	p.skipEmptyParts = true; // Skipping empty lines
	p.startRow = 1;
	p.columnNamesRaw = QStringLiteral("w,x");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 2);

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.1);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 2.2);
}

void AsciiFilterTest::testComments01() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const QStringList content = {
		QStringLiteral("# this"),
		QStringLiteral("# is"),
		QStringLiteral("# a"),
		QStringLiteral("# multi-line"),
		QStringLiteral("# comment"),
		QStringLiteral(""), // empty line
		QStringLiteral("# further comment"),
		QStringLiteral(""), // empty line
		QStringLiteral("# yet another comment"),
		QStringLiteral(""), // empty line
		QStringLiteral("1,1.1"),
		QStringLiteral("2,2.2"),
	};

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(",");
	p.headerEnabled = false;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.removeQuotes = true;
	p.simplifyWhitespaces = true;
	p.columnNamesRaw = QStringLiteral("w,x");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.1);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 2.2);
}

/*!
 * test with an empty comment character
 */
void AsciiFilterTest::testComments02() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const QStringList content = {
		QStringLiteral("c1;c2"),
		QStringLiteral("1;1"),
		QStringLiteral("2;2"),
		QStringLiteral("3;3"),
	};

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.removeQuotes = true;
	p.simplifyWhitespaces = true;
	p.commentCharacter = QStringLiteral("");
	p.columnNamesRaw = QStringLiteral("w,x");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 3);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("c1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("c2"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 1);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 2);

	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 3);
}

// ##############################################################################
// #########################  handling of datetime data #########################
// ##############################################################################
namespace {
const QStringList dateTime01Content = {
	QStringLiteral("Date,Water Pressure"),
	QStringLiteral("01/01/19 00:00:00,14.7982"),
	QStringLiteral("01/01/19 00:30:00,14.8026"),
};
}

/*!
 * read data containing only two characters for the year - 'yy'. The default year in
 * QDateTime is 1900 . When reading such two-characters DateTime values we want
 * to have the current centure after the import.
 */
void AsciiFilterTest::testDateTime00() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(",");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.dateTimeFormat = QLatin1String("dd/MM/yy hh:mm:ss");
	p.baseYear = 2000;
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", dateTime01Content);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Date"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Water Pressure"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	auto value = QDateTime::fromString(QLatin1String("01/01/2019 00:00:00"), QLatin1String("dd/MM/yyyy hh:mm:ss"));
	value.setTimeSpec(Qt::UTC);
	QCOMPARE(spreadsheet.column(0)->dateTimeAt(0), value);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 14.7982);

	value = QDateTime::fromString(QLatin1String("01/01/2019 00:30:00"), QLatin1String("dd/MM/yyyy hh:mm:ss"));
	value.setTimeSpec(Qt::UTC);
	QCOMPARE(spreadsheet.column(0)->dateTimeAt(1), value);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 14.8026);
}

void AsciiFilterTest::testDateTimeDefaultDateTimeFormat() {
	const QStringList content = {
		QStringLiteral("Date,Water Pressure"),
		QStringLiteral("2019-01-01 00:00:00,14.7982"),
		QStringLiteral("2020-01-01 00:30:00,14.8026"),
	};

	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(",");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	// p.dateTimeFormat; Default Datetime format is used!
	p.baseYear = 2000;
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Date"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Water Pressure"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	auto value = QDateTime::fromString(QLatin1String("01/01/2019 00:00:00"), QLatin1String("dd/MM/yyyy hh:mm:ss"));
	value.setTimeSpec(Qt::UTC);
	QCOMPARE(spreadsheet.column(0)->dateTimeAt(0), value);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 14.7982);

	value = QDateTime::fromString(QLatin1String("01/01/2020 00:30:00"), QLatin1String("dd/MM/yyyy hh:mm:ss"));
	value.setTimeSpec(Qt::UTC);
	QCOMPARE(spreadsheet.column(0)->dateTimeAt(1), value);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 14.8026);
}

/*!
 * same as in the previous test, but with the auto-detection of the datetime format enabled by clearing the datetime format
 */
void AsciiFilterTest::testDateTimeAutodetect() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(",");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.dateTimeFormat = QLatin1String(); // auto detect the format
	p.baseYear = 2000;
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", dateTime01Content);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Date"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Water Pressure"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	auto value = QDateTime::fromString(QLatin1String("01/01/2019 00:00:00"), QLatin1String("dd/MM/yyyy hh:mm:ss"));
	value.setTimeSpec(Qt::UTC);
	QCOMPARE(spreadsheet.column(0)->dateTimeAt(0), value);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 14.7982);

	value = QDateTime::fromString(QLatin1String("01/01/2019 00:30:00"), QLatin1String("dd/MM/yyyy hh:mm:ss"));
	value.setTimeSpec(Qt::UTC);
	QCOMPARE(spreadsheet.column(0)->dateTimeAt(1), value);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 14.8026);
}

/* read datetime data before big int
 *  TODO: handle hex value
 */
void AsciiFilterTest::testDateTimeHex() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	const QStringList content = {
		QStringLiteral("20191218023608|F|1000|000|000|3190|0528|3269|15|09|1.29|0934|-0105|G 03935| 94.09|9680|5AD17"),
	};

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral("|");
	p.headerEnabled = false;
	p.headerLine = 1;
	p.intAsDouble = false;
	p.columnNamesRaw = QStringLiteral("c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17");
	p.dateTimeFormat = QLatin1String("yyyyMMddhhmmss");
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 17);
	QCOMPARE(spreadsheet.rowCount(), 1);

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(5)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(6)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(7)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(8)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(9)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(10)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(11)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(12)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(13)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(14)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(15)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(16)->columnMode(), AbstractColumn::ColumnMode::Text);

	auto value = QDateTime::fromString(QLatin1String("18/12/2019 02:36:08"), QLatin1String("dd/MM/yyyy hh:mm:ss"));
	value.setTimeSpec(Qt::UTC);
	QCOMPARE(spreadsheet.column(0)->dateTimeAt(0), value);
	QCOMPARE(spreadsheet.column(1)->textAt(0), QLatin1String("F"));
	QCOMPARE(spreadsheet.column(2)->integerAt(0), 1000);
	QCOMPARE(spreadsheet.column(3)->integerAt(0), 0);
	QCOMPARE(spreadsheet.column(4)->integerAt(0), 0);
	QCOMPARE(spreadsheet.column(5)->integerAt(0), 3190);
	QCOMPARE(spreadsheet.column(6)->integerAt(0), 528);
	QCOMPARE(spreadsheet.column(7)->integerAt(0), 3269);
	QCOMPARE(spreadsheet.column(8)->integerAt(0), 15);
	QCOMPARE(spreadsheet.column(9)->integerAt(0), 9);
	QCOMPARE(spreadsheet.column(10)->valueAt(0), 1.29);
	QCOMPARE(spreadsheet.column(11)->integerAt(0), 934);
	QCOMPARE(spreadsheet.column(12)->integerAt(0), -105);
	QCOMPARE(spreadsheet.column(13)->textAt(0), QLatin1String("G 03935"));
	QCOMPARE(spreadsheet.column(14)->valueAt(0), 94.09);
	QCOMPARE(spreadsheet.column(15)->integerAt(0), 9680);
	QCOMPARE(spreadsheet.column(16)->textAt(0), QLatin1String("5AD17"));
}

// Keep all values
void AsciiFilterTest::testAppendRows() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("x, y");
	p.intAsDouble = false;
	filter.setProperties(p);

	{
		QStringList fileContent = {
			QStringLiteral("1;5"),
			QStringLiteral("2;6"),
			QStringLiteral("3;7"),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	}

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 7);

	{
		QStringList fileContent2 = {
			QStringLiteral("11;12"),
			QStringLiteral("13;14"),
			QStringLiteral("15;16"),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent2);
		KCompressionDevice file(savePath);
		filter.readFromDevice(file, AbstractFileFilter::ImportMode::Replace, AbstractFileFilter::ImportMode::Append, 0, -1, -1);
	}

	QCOMPARE(spreadsheet.rowCount(), 6);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->integerAt(3), 11);
	QCOMPARE(spreadsheet.column(0)->integerAt(4), 13);
	QCOMPARE(spreadsheet.column(0)->integerAt(5), 15);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 7);
	QCOMPARE(spreadsheet.column(1)->integerAt(3), 12);
	QCOMPARE(spreadsheet.column(1)->integerAt(4), 14);
	QCOMPARE(spreadsheet.column(1)->integerAt(5), 16);
}

void AsciiFilterTest::keepLast() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("x, y");
	p.intAsDouble = false;
	filter.setProperties(p);

	{
		QStringList fileContent = {
			QStringLiteral("1;5"),
			QStringLiteral("2;6"),
			QStringLiteral("3;7"),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	}

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 7);

	{
		QStringList fileContent2 = {
			QStringLiteral("11;12"),
			QStringLiteral("13;14"),
			QStringLiteral("15;16"),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent2);
		KCompressionDevice file(savePath);
		filter.readFromDevice(file, AbstractFileFilter::ImportMode::Replace, AbstractFileFilter::ImportMode::Append, 0, -1, 4);
	}

	QCOMPARE(spreadsheet.rowCount(), 4);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 3);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 11);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 13);
	QCOMPARE(spreadsheet.column(0)->integerAt(3), 15);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 7);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 12);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 14);
	QCOMPARE(spreadsheet.column(1)->integerAt(3), 16);
}

void AsciiFilterTest::testAppendColumns() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	{
		QStringList fileContent = {
			QStringLiteral("1;5"),
			QStringLiteral("2;6"),
			QStringLiteral("3;7"),
		};
		AsciiFilter filter;

		auto p = filter.properties();
		p.automaticSeparatorDetection = false;
		p.separator = QStringLiteral(";");
		p.headerEnabled = false;
		p.columnNamesRaw = QStringLiteral("x, y");
		p.intAsDouble = false;
		filter.setProperties(p);

		QString savePath;
		SAVE_FILE("testfile", fileContent);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	}

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 7);

	{
		AsciiFilter filter;

		auto p = filter.properties();
		p.automaticSeparatorDetection = false;
		p.separator = QStringLiteral(";");
		p.headerEnabled = false;
		p.columnNamesRaw = QStringLiteral("x, y");
		p.intAsDouble = false;
		filter.setProperties(p);

		QStringList fileContent2 = {
			QStringLiteral("11;12"),
			QStringLiteral("13;14"),
			QStringLiteral("15;16"),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent2);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Append);
	}

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 4);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("x 1"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("y 1"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 7);

	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(2)->integerAt(0), 11);
	QCOMPARE(spreadsheet.column(2)->integerAt(1), 13);
	QCOMPARE(spreadsheet.column(2)->integerAt(2), 15);

	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(3)->integerAt(0), 12);
	QCOMPARE(spreadsheet.column(3)->integerAt(1), 14);
	QCOMPARE(spreadsheet.column(3)->integerAt(2), 16);
}

void AsciiFilterTest::testPrependColumns() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	{
		AsciiFilter filter;

		auto p = filter.properties();
		p.automaticSeparatorDetection = false;
		p.separator = QStringLiteral(";");
		p.headerEnabled = false;
		p.columnNamesRaw = QStringLiteral("x, y");
		p.intAsDouble = false;
		filter.setProperties(p);
		QStringList fileContent = {
			QStringLiteral("1;5"),
			QStringLiteral("2;6"),
			QStringLiteral("3;7"),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	}

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 5);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 7);

	{
		AsciiFilter filter;

		auto p = filter.properties();
		p.automaticSeparatorDetection = false;
		p.separator = QStringLiteral(";");
		p.headerEnabled = false;
		p.columnNamesRaw = QStringLiteral("x, y");
		p.intAsDouble = false;
		filter.setProperties(p);
		QStringList fileContent2 = {
			QStringLiteral("11;12"),
			QStringLiteral("13;14"),
			QStringLiteral("15;16"),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent2);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Prepend);
	}

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 4);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x 1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y 1"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("y"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 11);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 13);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 15);

	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 12);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 14);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 16);

	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(2)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(2)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(2)->integerAt(2), 3);

	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(3)->integerAt(0), 5);
	QCOMPARE(spreadsheet.column(3)->integerAt(1), 6);
	QCOMPARE(spreadsheet.column(3)->integerAt(2), 7);
}

void AsciiFilterTest::testCommaAsDecimalSeparator() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;

	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(",");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.removeQuotes = true;
	p.intAsDouble = false;
	p.simplifyWhitespaces = false;
	p.locale = QLocale::Language::German;

	filter.setProperties(p);

	{
		QStringList fileContent = {
			QStringLiteral("Commit Id,Description,Measurement 1,Measurement2,Measurement3,Measurement 4"),
			QStringLiteral("9b1cdd5607eaaa521fe74d632ce43f9a37302bf8,master,\"2,908\",\"2,926\",\"2,886\",\"3,043\""),
			QStringLiteral("41e0de50ee42ed464533af85e5e8b8c949509384,optimize determine Columns (Do not allocate all the time "
						   "columnNames),\"2,749\",\"2,727\",\"2,792\",\"2,757\""),
			QStringLiteral("fec3a75c04f4a60571e92ceb1997a4be5311e94b,Make constants Qchar instead of QLatin1String,\"2,77\",\"2,761\",\"2,767\",\"2,762\""),
			QStringLiteral("bd1a1b252a88e4bb78889d68ef42f185b8a737e7,Simplified check for the separator if only a single "
						   "character,\"2,24\",\"2,274\",\"2,148\",\"2,215\""),
			QStringLiteral("f584f71c6bfb4777bd01c13d2c912c5436f726d3,Determine separatorCharacter outside of the loop,\"2,2\",\"2,219\",\"2,201\",\"2,211\""),
		};

		QString savePath;
		SAVE_FILE("testfile", fileContent);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);
	}

	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 6);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Commit Id"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Description"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Measurement 1"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("Measurement2"));
	QCOMPARE(spreadsheet.column(4)->name(), QLatin1String("Measurement3"));
	QCOMPARE(spreadsheet.column(5)->name(), QLatin1String("Measurement 4"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(5)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->textAt(0), QStringLiteral("9b1cdd5607eaaa521fe74d632ce43f9a37302bf8"));
	QCOMPARE(spreadsheet.column(0)->textAt(1), QStringLiteral("41e0de50ee42ed464533af85e5e8b8c949509384"));
	QCOMPARE(spreadsheet.column(0)->textAt(2), QStringLiteral("fec3a75c04f4a60571e92ceb1997a4be5311e94b"));
	QCOMPARE(spreadsheet.column(0)->textAt(3), QStringLiteral("bd1a1b252a88e4bb78889d68ef42f185b8a737e7"));
	QCOMPARE(spreadsheet.column(0)->textAt(4), QStringLiteral("f584f71c6bfb4777bd01c13d2c912c5436f726d3"));

	QCOMPARE(spreadsheet.column(1)->textAt(0), QStringLiteral("master"));
	QCOMPARE(spreadsheet.column(1)->textAt(1), QStringLiteral("optimize determine Columns (Do not allocate all the time columnNames)"));
	QCOMPARE(spreadsheet.column(1)->textAt(2), QStringLiteral("Make constants Qchar instead of QLatin1String"));
	QCOMPARE(spreadsheet.column(1)->textAt(3), QStringLiteral("Simplified check for the separator if only a single character"));
	QCOMPARE(spreadsheet.column(1)->textAt(4), QStringLiteral("Determine separatorCharacter outside of the loop"));

	VALUES_EQUAL(spreadsheet.column(2)->valueAt(0), 2.908);
	VALUES_EQUAL(spreadsheet.column(2)->valueAt(1), 2.749);
	VALUES_EQUAL(spreadsheet.column(2)->valueAt(2), 2.77);
	VALUES_EQUAL(spreadsheet.column(2)->valueAt(3), 2.24);
	VALUES_EQUAL(spreadsheet.column(2)->valueAt(4), 2.2);

	VALUES_EQUAL(spreadsheet.column(3)->valueAt(0), 2.926);
	VALUES_EQUAL(spreadsheet.column(3)->valueAt(1), 2.727);
	VALUES_EQUAL(spreadsheet.column(3)->valueAt(2), 2.761);
	VALUES_EQUAL(spreadsheet.column(3)->valueAt(3), 2.274);
	VALUES_EQUAL(spreadsheet.column(3)->valueAt(4), 2.219);

	VALUES_EQUAL(spreadsheet.column(4)->valueAt(0), 2.886);
	VALUES_EQUAL(spreadsheet.column(4)->valueAt(1), 2.792);
	VALUES_EQUAL(spreadsheet.column(4)->valueAt(2), 2.767);
	VALUES_EQUAL(spreadsheet.column(4)->valueAt(3), 2.148);
	VALUES_EQUAL(spreadsheet.column(4)->valueAt(4), 2.201);

	VALUES_EQUAL(spreadsheet.column(5)->valueAt(0), 3.043);
	VALUES_EQUAL(spreadsheet.column(5)->valueAt(1), 2.757);
	VALUES_EQUAL(spreadsheet.column(5)->valueAt(2), 2.762);
	VALUES_EQUAL(spreadsheet.column(5)->valueAt(3), 2.215);
	VALUES_EQUAL(spreadsheet.column(5)->valueAt(4), 2.211);
}

void AsciiFilterTest::testMatrixHeader() {
	Matrix matrix(QStringLiteral("test"), false);

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(" ");
	p.headerEnabled = false;
	p.headerLine = 1;
	p.intAsDouble = false;
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", numeric_data);
	filter.readDataFromFile(savePath, &matrix, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(matrix.rowCount(), 5);
	QCOMPARE(matrix.columnCount(), 3);

	QCOMPARE(matrix.mode(), AbstractColumn::ColumnMode::Double);

	// check all values
	QCOMPARE(matrix.cell<double>(0, 0), 1.716299);
	QCOMPARE(matrix.cell<double>(0, 1), -0.485527);
	QCOMPARE(matrix.cell<double>(0, 2), -0.288690);
	QCOMPARE(matrix.cell<double>(1, 0), 1.716299);
	QCOMPARE(matrix.cell<double>(1, 1), -0.476371);
	QCOMPARE(matrix.cell<double>(1, 2), -0.274957);
	QCOMPARE(matrix.cell<double>(2, 0), 1.711721);
	QCOMPARE(matrix.cell<double>(2, 1), -0.485527);
	QCOMPARE(matrix.cell<double>(2, 2), -0.293267);
	QCOMPARE(matrix.cell<double>(3, 0), 1.711721);
	QCOMPARE(matrix.cell<double>(3, 1), -0.480949);
	QCOMPARE(matrix.cell<double>(3, 2), -0.293267);
	QCOMPARE(matrix.cell<double>(4, 0), 1.716299);
	QCOMPARE(matrix.cell<double>(4, 1), -0.494682);
	QCOMPARE(matrix.cell<double>(4, 2), -0.284112);
}

// ##############################################################################
// ############# updates in the dependent objects after the import ##############
// ##############################################################################
/*!
 * test the update of the column values calculated via a formula after the values
 * in the source spreadsheet were modified by the import.
 */
void AsciiFilterTest::spreadsheetFormulaUpdateAfterImport() {
	Project project;

	// create the first spreadsheet with the source data
	auto* spreadsheet = new Spreadsheet(QStringLiteral("test"), false);
	spreadsheet->setColumnCount(2);
	spreadsheet->setRowCount(3);
	project.addChild(spreadsheet);

	auto* col = spreadsheet->column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c1"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	col = spreadsheet->column(1);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c2"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	// create the second spreadsheet with one single column calculated via a formula from the first spreadsheet
	auto* spreadsheetFormula = new Spreadsheet(QStringLiteral("formula"), false);
	spreadsheetFormula->setColumnCount(1);
	spreadsheetFormula->setRowCount(3);
	project.addChild(spreadsheetFormula);
	col = spreadsheetFormula->column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);

	QStringList variableNames{QLatin1String("x"), QLatin1String("y")};
	QVector<Column*> variableColumns{spreadsheet->column(0), spreadsheet->column(1)};
	col->setFormula(QLatin1String("x+y"), variableNames, variableColumns, true);

	int formulaChangedCounter = 0;
	connect(col, &Column::formulaChanged, [&formulaChangedCounter]() {
		formulaChangedCounter++;
	});
	col->updateFormula();
	QCOMPARE(formulaChangedCounter, 1);

	// check the results of the calculation first
	QCOMPARE(spreadsheetFormula->columnCount(), 1);
	QCOMPARE(spreadsheetFormula->rowCount(), 3);
	QCOMPARE(col->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(col->valueAt(0), 20.);
	QCOMPARE(col->valueAt(1), 40.);
	QCOMPARE(col->valueAt(2), 60.);

	{
		// import the data into the source spreadsheet
		const QStringList content = {
			QStringLiteral("c1;c2"),
			QStringLiteral("1;1"),
			QStringLiteral("2;2"),
			QStringLiteral("3;3"),
		};
		AsciiFilter filter;
		auto p = filter.properties();
		p.automaticSeparatorDetection = true;
		p.separator = QStringLiteral(" ");
		p.headerEnabled = true;
		p.headerLine = 1;
		p.intAsDouble = false;
		filter.setProperties(p);

		QString savePath;
		SAVE_FILE("testfile", content);
		formulaChangedCounter = 0;
		filter.readDataFromFile(savePath, spreadsheet, AbstractFileFilter::ImportMode::Replace);
		QCOMPARE(formulaChangedCounter, 1);
	}

	// re-check the results of the calculation
	QCOMPARE(spreadsheetFormula->columnCount(), 1);
	QCOMPARE(spreadsheetFormula->rowCount(), 3);
	QCOMPARE(col->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(col->valueAt(0), 2.); // c1 + c2
	QCOMPARE(col->valueAt(1), 4.); // c1 + c2
	QCOMPARE(col->valueAt(2), 6.); // c1 + c2

	// Reread to check if it still works
	{
		// import the data into the source spreadsheet
		const QStringList content = {
			QStringLiteral("c1;c2"),
			QStringLiteral("500;666"),
			QStringLiteral("501;876"),
			QStringLiteral("502;370"),
		};
		AsciiFilter filter;
		auto p = filter.properties();
		p.automaticSeparatorDetection = true;
		p.separator = QStringLiteral(" ");
		p.headerEnabled = true;
		p.headerLine = 1;
		p.intAsDouble = false;
		filter.setProperties(p);

		QString savePath;
		SAVE_FILE("testfile", content);
		formulaChangedCounter = 0;
		filter.readDataFromFile(savePath, spreadsheet, AbstractFileFilter::ImportMode::Replace);
	}
	QCOMPARE(formulaChangedCounter, 1);

	// re-check the results of the calculation
	QCOMPARE(spreadsheetFormula->columnCount(), 1);
	QCOMPARE(spreadsheetFormula->rowCount(), 3);
	QCOMPARE(col->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(col->valueAt(0), 1166.); // c1 + c2
	QCOMPARE(col->valueAt(1), 1377.); // c1 + c2
	QCOMPARE(col->valueAt(2), 872.); // c1 + c2
}

/*!
 * test the update of the column values calculated via a formula after one of the source columns
 * was deleted first and was restored and the source values were modified by the import.
 */
void AsciiFilterTest::spreadsheetFormulaUpdateAfterImportWithColumnRestore() {
	Project project; // need a project object since the column restore logic is in project

	// create the first spreadsheet with the source data
	auto* spreadsheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(spreadsheet);
	spreadsheet->setColumnCount(2);
	spreadsheet->setRowCount(3);

	auto* col = spreadsheet->column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c1"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	col = spreadsheet->column(1);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c2"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	// create the second spreadsheet with one single column calculated via a formula from the first spreadsheet
	auto* spreadsheetFormula = new Spreadsheet(QStringLiteral("formula"), false);
	project.addChild(spreadsheetFormula);
	spreadsheetFormula->setColumnCount(1);
	spreadsheetFormula->setRowCount(3);
	col = spreadsheetFormula->column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);

	QStringList variableNames{QLatin1String("x"), QLatin1String("y")};
	QVector<Column*> variableColumns{spreadsheet->column(0), spreadsheet->column(1)};
	col->setFormula(QLatin1String("x+y"), variableNames, variableColumns, true);
	col->updateFormula();

	// delete the first column in the source spreadsheet and check the results of the calculation first,
	// the cells should be empty
	spreadsheet->removeChild(spreadsheet->column(0));
	QCOMPARE(spreadsheetFormula->columnCount(), 1);
	QCOMPARE(spreadsheetFormula->rowCount(), 3);
	QCOMPARE(col->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE((bool)std::isnan(col->valueAt(0)), true);
	QCOMPARE((bool)std::isnan(col->valueAt(1)), true);
	QCOMPARE((bool)std::isnan(col->valueAt(2)), true);

	// import the data into the source spreadsheet, the deleted column with the name "c1" is re-created again
	const QStringList content = {
		QStringLiteral("c1;c2"),
		QStringLiteral("1;1"),
		QStringLiteral("2;2"),
		QStringLiteral("3;3"),
	};
	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = true;
	p.separator = QStringLiteral(" ");
	p.headerEnabled = true;
	p.headerLine = 1;
	p.intAsDouble = false;
	filter.setProperties(p);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// re-check the results of the calculation after one of the source columns was re-created and the values were changed
	QCOMPARE(spreadsheetFormula->columnCount(), 1);
	QCOMPARE(spreadsheetFormula->rowCount(), 3);
	QCOMPARE(col->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(col->valueAt(0), 2.);
	QCOMPARE(col->valueAt(1), 4.);
	QCOMPARE(col->valueAt(2), 6.);
}

/*!
 * test the update of the xycurve and plot ranges after the values
 * in the source columns were modified by the import.
 */
void AsciiFilterTest::plotUpdateAfterImport() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	// create the spreadsheet with the source data
	auto* spreadsheet = new Spreadsheet(QStringLiteral("test"), false);
	spreadsheet->setColumnCount(2);
	spreadsheet->setRowCount(3);
	project.addChild(spreadsheet);

	auto* col = spreadsheet->column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c1"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	col = spreadsheet->column(1);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c2"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	// create a xy-curve with the both columns in the source spreadsheet and check the ranges
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(p);
	auto* curve = new XYCurve(QStringLiteral("curve"));
	p->addChild(curve);
	curve->setXColumn(spreadsheet->column(0));
	curve->setYColumn(spreadsheet->column(1));

	auto rangeX = p->range(Dimension::X);
	QCOMPARE(rangeX.start(), 10);
	QCOMPARE(rangeX.end(), 30);

	auto rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeY.start(), 10);
	QCOMPARE(rangeY.end(), 30);

	// import the data into the source spreadsheet
	const QStringList content = {
		QStringLiteral("c1;c2"),
		QStringLiteral("1;1"),
		QStringLiteral("2;2"),
		QStringLiteral("3;3"),
	};
	AsciiFilter filter;
	auto properties = filter.properties();
	properties.automaticSeparatorDetection = true;
	properties.separator = QStringLiteral(" ");
	properties.headerEnabled = true;
	properties.headerLine = 1;
	properties.intAsDouble = false;
	filter.setProperties(properties);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// re-check the plot ranges with the new data
	rangeX = p->range(Dimension::X);
	QCOMPARE(rangeX.start(), 1);
	QCOMPARE(rangeX.end(), 3);

	rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeY.start(), 1);
	QCOMPARE(rangeY.end(), 3);
}

/*!
 * test the update of the xycurve and plot ranges after one of the source columns
 * was deleted first and was restored and the source values were modified by the import.
 */
void AsciiFilterTest::plotUpdateAfterImportWithColumnRestore() {
	Project project; // need a project object since the column restore logic is in project

	// create the spreadsheet with the source data
	auto* spreadsheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(spreadsheet);
	spreadsheet->setColumnCount(2);
	spreadsheet->setRowCount(3);

	auto* col = spreadsheet->column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c1"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	col = spreadsheet->column(1);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c2"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	// create a xy-curve with the both columns in the source spreadsheet and check the ranges
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	project.addChild(p);
	auto* curve = new XYCurve(QStringLiteral("curve"));
	p->addChild(curve);
	curve->setXColumn(spreadsheet->column(0));
	curve->setYColumn(spreadsheet->column(1));

	auto rangeX = p->range(Dimension::X);
	QCOMPARE(rangeX.start(), 10);
	QCOMPARE(rangeX.end(), 30);

	auto rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeY.start(), 10);
	QCOMPARE(rangeY.end(), 30);

	// delete the first source column
	spreadsheet->removeChild(spreadsheet->column(0));

	// import the data into the source spreadsheet, the deleted column with the name "c1" is re-created again
	const QStringList content = {
		QStringLiteral("c1;c2"),
		QStringLiteral("1;1"),
		QStringLiteral("2;2"),
		QStringLiteral("3;3"),
	};
	AsciiFilter filter;
	auto properties = filter.properties();
	properties.automaticSeparatorDetection = true;
	properties.separator = QStringLiteral(" ");
	properties.headerEnabled = true;
	properties.headerLine = 1;
	properties.intAsDouble = false;
	filter.setProperties(properties);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// re-check the plot ranges with the new data
	rangeX = p->range(Dimension::X);
	QCOMPARE(rangeX.start(), 1);
	QCOMPARE(rangeX.end(), 3);

	rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeY.start(), 1);
	QCOMPARE(rangeY.end(), 3);
}

/*!
 * test the update of the xycurve and plot ranges after the order or columns (their names)
 * was changed during the import.
 */
void AsciiFilterTest::plotUpdateAfterImportWithColumnRenaming() {
	Project project; // need a project object since the column restore logic is in project

	// create the spreadsheet with the source data
	auto* spreadsheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(spreadsheet);
	spreadsheet->setColumnCount(3);
	spreadsheet->setRowCount(3);

	auto* col = spreadsheet->column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c0")); // dummy column, values not required

	col = spreadsheet->column(1);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c1"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	col = spreadsheet->column(2);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c2"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	// create a xy-curve with the both columns in the source spreadsheet and check the ranges
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	project.addChild(p);
	auto* curve = new XYCurve(QStringLiteral("curve"));
	p->addChild(curve);
	curve->setXColumn(spreadsheet->column(1)); // use "c1" for x
	curve->setYColumn(spreadsheet->column(2)); // use "c2" for y

	auto rangeX = p->range(Dimension::X);
	QCOMPARE(rangeX.start(), 10);
	QCOMPARE(rangeX.end(), 30);

	auto rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeY.start(), 10);
	QCOMPARE(rangeY.end(), 30);

	// import the data into the source spreadsheet:
	// the columns "c0", "c1" and "c2" are renamed to "c1", "c2" and "c3" and xy-curve should still be using "c1" and "c2"
	// event hough their positions in the spreadsheet have changed
	const QStringList content = {
		QStringLiteral("c1;c2"),
		QStringLiteral("1;1"),
		QStringLiteral("2;2"),
		QStringLiteral("3;3"),
	};
	AsciiFilter filter;
	auto properties = filter.properties();
	properties.automaticSeparatorDetection = true;
	properties.separator = QStringLiteral(" ");
	properties.headerEnabled = true;
	properties.headerLine = 1;
	properties.intAsDouble = false;
	filter.setProperties(properties);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// check the connection to the new columns
	QCOMPARE(curve->xColumn(), spreadsheet->column(0)); // "c1" for x
	QCOMPARE(curve->yColumn(), spreadsheet->column(1)); // "c2" for y

	// re-check the plot ranges with the new data
	rangeX = p->range(Dimension::X);
	QCOMPARE(rangeX.start(), 1);
	QCOMPARE(rangeX.end(), 3);

	rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeY.start(), 1);
	QCOMPARE(rangeY.end(), 3);
}

/*!
 * test the update of the xycurve after the source columns were renamed
 * during the import, the curve becomes invalid after this.
 */
void AsciiFilterTest::plotUpdateAfterImportWithColumnRemove() {
	Project project; // need a project object since the column restore logic is in project

	// create the spreadsheet with the source data
	auto* spreadsheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(spreadsheet);
	spreadsheet->setColumnCount(2);
	spreadsheet->setRowCount(3);

	auto* col = spreadsheet->column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("1"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	col = spreadsheet->column(1);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("2"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	// create a xy-curve with the both columns in the source spreadsheet and check the ranges
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	project.addChild(p);
	auto* curve = new XYCurve(QStringLiteral("curve"));
	p->addChild(curve);
	curve->setXColumn(spreadsheet->column(0)); // use "1" for x
	curve->setYColumn(spreadsheet->column(1)); // use "2" for y

	auto rangeX = p->range(Dimension::X);
	QCOMPARE(rangeX.start(), 10);
	QCOMPARE(rangeX.end(), 30);

	auto rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeY.start(), 10);
	QCOMPARE(rangeY.end(), 30);

	// import the data into the source spreadsheet, the columns are renamed to "c1" and "c2"
	const QStringList content = {
		QStringLiteral("c1;c2"),
		QStringLiteral("1;1"),
		QStringLiteral("2;2"),
		QStringLiteral("3;3"),
	};
	AsciiFilter filter;
	auto properties = filter.properties();
	properties.automaticSeparatorDetection = true;
	properties.separator = QStringLiteral(" ");
	properties.headerEnabled = true;
	properties.headerLine = 1;
	properties.intAsDouble = false;
	filter.setProperties(properties);

	QString savePath;
	SAVE_FILE("testfile", content);
	filter.readDataFromFile(savePath, spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// the assignment to the data columns got lost since the columns were renamed
	QCOMPARE(curve->xColumn(), nullptr);
	QCOMPARE(curve->yColumn(), nullptr);

	// TODO: further checks to make sure the curve was really and properly invalidated
}

// ##############################################################################
// ################################# Benchmarks #################################
// ##############################################################################
void AsciiFilterTest::benchDoubleImport_data() {
	QTest::addColumn<size_t>("lineCount");
	// can't transfer file name since needed in clean up

	QTemporaryFile file;
	if (!file.open()) // needed to generate file name
		return;

	file.setAutoRemove(false);
	benchDataFileName = file.fileName();

	QString testName(QString::number(paths) + QLatin1String(" random double paths"));

	QTest::newRow(qPrintable(testName)) << lines;
	DEBUG("CREATE DATA FILE " << STDSTRING(benchDataFileName) << ", lines = " << lines)

	gsl_rng_env_setup();
	gsl_rng* r = gsl_rng_alloc(gsl_rng_default);
	gsl_rng_set(r, 12345);

	// create file
	QTextStream out(&file);
	// for higher precision
	// out.setRealNumberPrecision(13);

	// create data
	double path[paths] = {0.0};

	const double delta = 0.25;
	const int dt = 1;
	const double sigma = delta * delta * dt;
	for (size_t i = 0; i < lines; ++i) {
		// std::cout << "line " << i+1 << std::endl;

		for (int p = 0; p < paths; ++p) {
			path[p] += gsl_ran_gaussian_ziggurat(r, sigma);
			out << path[p];
			if (p < paths - 1)
				out << ' ';
		}
		out << QStringLiteral("\n");
	}

	DEBUG(Q_FUNC_INFO << ", DONE")
}

void AsciiFilterTest::benchDoubleImport() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);

	AsciiFilter filter;
	auto properties = filter.properties();
	properties.automaticSeparatorDetection = true;
	properties.separator = QStringLiteral(" ");
	properties.headerEnabled = false;
	properties.headerLine = 1;
	properties.intAsDouble = false;
	filter.setProperties(properties);

	const int p = paths; // need local variable
	QBENCHMARK {
		filter.readDataFromFile(benchDataFileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

		QCOMPARE(spreadsheet.columnCount(), p);
		QCOMPARE(spreadsheet.rowCount(), lines);

		QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.120998);
		QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.119301);
		QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.0209980);
	}
}

void AsciiFilterTest::benchDoubleImport_cleanup() {
	DEBUG("REMOVE DATA FILE " << STDSTRING(benchDataFileName))
	QFile::remove(benchDataFileName);
}

void AsciiFilterTest::benchMarkCompare_SimplifyWhiteSpace() {
	const int numberColumns = 5;
	const int numberRows = 10;

	QStringList content;

	// Header
	QString headerLine;
	for (int column = 0; column < numberColumns - 1; column++) {
		headerLine.append(QStringLiteral("c%1,").arg(QString::number(column + 1)));
	}
	headerLine.append(QStringLiteral("c%1").arg(QString::number(numberColumns)));
	content.append(headerLine);

	// Create data
	for (int row = 0; row < numberRows; row++) {
		QString line;
		for (int column = 0; column < numberColumns - 1; column++) {
			line.append(QStringLiteral("0.123234234,"));
		}
		line.append(QStringLiteral("0.123234234"));
		content.append(line);
	}

	QString savePath;
	SAVE_FILE("testfile", content);

	QBENCHMARK {
		Spreadsheet spreadsheet(QStringLiteral("test"), false);
		AsciiFilter filter;
		auto properties = filter.properties();
		properties.automaticSeparatorDetection = false;
		properties.separator = QStringLiteral(",");
		properties.headerEnabled = true;
		properties.headerLine = 1;
		properties.intAsDouble = false;
		properties.simplifyWhitespaces = false;
		properties.columnModesString = QStringLiteral("Double,Double,Double,Double,Double");
		properties.columnNamesRaw = QStringLiteral("1,2,3,4,5");
		// QCOMPARE(filter.initialize(properties), i18n("Success"));
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

		QCOMPARE(spreadsheet.columnCount(), numberColumns);
		QCOMPARE(spreadsheet.rowCount(), numberRows);

		QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
		VALUES_EQUAL(spreadsheet.column(0)->valueAt(0), 0.123234234);
		VALUES_EQUAL(spreadsheet.column(1)->valueAt(0), 0.123234234);
		VALUES_EQUAL(spreadsheet.column(2)->valueAt(0), 0.123234234);
	}

	QBENCHMARK {
		Spreadsheet spreadsheet(QStringLiteral("test"), false);
		AsciiFilter filter;
		auto properties = filter.properties();
		properties.automaticSeparatorDetection = true;
		properties.separator = QStringLiteral(" ");
		properties.headerEnabled = true;
		properties.simplifyWhitespaces = true;
		properties.headerLine = 1;
		properties.intAsDouble = false;
		filter.setProperties(properties);
		filter.readDataFromFile(savePath, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

		QCOMPARE(spreadsheet.columnCount(), numberColumns);
		QCOMPARE(spreadsheet.rowCount(), numberRows);

		QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
		VALUES_EQUAL(spreadsheet.column(0)->valueAt(0), 0.123234234);
		VALUES_EQUAL(spreadsheet.column(1)->valueAt(0), 0.123234234);
		VALUES_EQUAL(spreadsheet.column(2)->valueAt(0), 0.123234234);
	}
}

void AsciiFilterTest::determineSeparator() {
	QString separator;
	bool removeQuotes = true;
	bool simplifyWhiteSpaces = true;
	QCOMPARE(AsciiFilterPrivate::determineSeparator(QStringLiteral("header1,header2,header3\n"), removeQuotes, simplifyWhiteSpaces, separator).success(), true);
	QCOMPARE(separator, QStringLiteral(","));

	QCOMPARE(AsciiFilterPrivate::determineSeparator(QStringLiteral("header1;header2;header3\n"), removeQuotes, simplifyWhiteSpaces, separator).success(), true);
	QCOMPARE(separator, QStringLiteral(";"));

	// Quotes handling
	QCOMPARE(AsciiFilterPrivate::determineSeparator(QStringLiteral("\"header1,|\";header2;header3\n"), removeQuotes, simplifyWhiteSpaces, separator).success(),
			 true);
	QCOMPARE(separator, QStringLiteral(";"));

	// Whitespaces
	QCOMPARE(
		AsciiFilterPrivate::determineSeparator(QStringLiteral("\t header1; \theader2; \theader3 \n"), removeQuotes, simplifyWhiteSpaces, separator).success(),
		true);
	QCOMPARE(separator, QStringLiteral(";"));

	// Whitespace in header string
	QCOMPARE(AsciiFilterPrivate::determineSeparator(QStringLiteral("\"\t header1\"; \theader2; \theader3 \n"), removeQuotes, simplifyWhiteSpaces, separator)
				 .success(),
			 true);
	QCOMPARE(separator, QStringLiteral(";"));

	// Space separator
	simplifyWhiteSpaces = false;
	QCOMPARE(AsciiFilterPrivate::determineSeparator(QStringLiteral("header1 header2 header3\n"), removeQuotes, simplifyWhiteSpaces, separator).success(), true);
	QCOMPARE(separator, QStringLiteral(" "));

	simplifyWhiteSpaces = true; // With simplify whitespace
	QCOMPARE(AsciiFilterPrivate::determineSeparator(QStringLiteral("   header1\t     header2     header3\n"), removeQuotes, simplifyWhiteSpaces, separator)
				 .success(),
			 true);
	QCOMPARE(separator, QStringLiteral(" "));

	// Tab separator
	simplifyWhiteSpaces = false;
	QCOMPARE(AsciiFilterPrivate::determineSeparator(QStringLiteral("header1\theader2\theader3\n"), removeQuotes, simplifyWhiteSpaces, separator).success(),
			 true);
	QCOMPARE(separator, QStringLiteral("\t"));

	// Space in quoted text
	simplifyWhiteSpaces = false;
	QCOMPARE(AsciiFilterPrivate::determineSeparator(QStringLiteral("\"header 1\"\theader2\theader3\n"), removeQuotes, simplifyWhiteSpaces, separator).success(),
			 true);
	QCOMPARE(separator, QStringLiteral("\t"));
}

void AsciiFilterTest::determineColumns() {
	AsciiFilter::Properties p;
	p.removeQuotes = true;
	p.simplifyWhitespaces = true;
	p.skipEmptyParts = true;
	p.startColumn = 1;
	p.endColumn = -1;
	p.separator = QStringLiteral(",");

	auto expectedHeader = QStringList{QStringLiteral("header1"), QStringLiteral("header2"), QStringLiteral("header3")};
	QCOMPARE(AsciiFilterPrivate::determineColumnsSimplifyWhiteSpace(QStringLiteral("header1,header2,header3\n"), p), expectedHeader);

	expectedHeader = QStringList{QStringLiteral("header 1"), QStringLiteral("header 2"), QStringLiteral("header 3")};
	QCOMPARE(AsciiFilterPrivate::determineColumnsSimplifyWhiteSpace(QStringLiteral("\"header 1\",\"header 2\",\"header 3\"\n"), p), expectedHeader);

	// No whitespace simplifying! High performance parser
	p.simplifyWhitespaces = false;

	const auto separatorLength = p.separator.size();
	bool separatorSingleCharacter = separatorLength == 1;
	QChar separatorCharacter;
	if (separatorLength)
		separatorCharacter = p.separator[separatorLength - 1];

	p.removeQuotes = false;
	auto expectedHeaderNoWhiteSpace = QVector<QStringView>{QStringLiteral("header1"), QStringLiteral("header2"), QStringLiteral("header3")};
	QVector<QStringView> columnNames(3);
	QCOMPARE(AsciiFilterPrivate::determineColumns(QStringLiteral("header1,header2,header3\n"), p, separatorSingleCharacter, separatorCharacter, columnNames),
			 3);
	QCOMPARE(columnNames, expectedHeaderNoWhiteSpace);

	p.removeQuotes = true;
	expectedHeaderNoWhiteSpace = QVector<QStringView>{QStringLiteral("header 1"), QStringLiteral("header 2"), QStringLiteral("header 3")};
	QCOMPARE(AsciiFilterPrivate::determineColumns(QStringLiteral("\"header 1\",\"header 2\",\"header 3\"\n"),
												  p,
												  separatorSingleCharacter,
												  separatorCharacter,
												  columnNames),
			 3);
	QCOMPARE(columnNames, expectedHeaderNoWhiteSpace);

	// Without \n at the end
	p.removeQuotes = true;
	expectedHeaderNoWhiteSpace = QVector<QStringView>{QStringLiteral("header 1"), QStringLiteral("header 2"), QStringLiteral("header 3")};
	QCOMPARE(AsciiFilterPrivate::determineColumns(QStringLiteral("\"header 1\",\"header 2\",\"header 3"),
												  p,
												  separatorSingleCharacter,
												  separatorCharacter,
												  columnNames),
			 3);
	QCOMPARE(columnNames, expectedHeaderNoWhiteSpace);

	// Quotes still inside
	p.removeQuotes = false;
	expectedHeaderNoWhiteSpace = QVector<QStringView>{QStringLiteral("\"header 1\""), QStringLiteral("\"header 2\""), QStringLiteral("\"header 3\"")};
	QCOMPARE(AsciiFilterPrivate::determineColumns(QStringLiteral("\"header 1\",\"header 2\",\"header 3\"\n"),
												  p,
												  separatorSingleCharacter,
												  separatorCharacter,
												  columnNames),
			 3);
	QCOMPARE(columnNames, expectedHeaderNoWhiteSpace);

	p.removeQuotes = true;
	expectedHeaderNoWhiteSpace = QVector<QStringView>{QStringLiteral("header 1"), QStringLiteral("header2"), QStringLiteral("header 3")};
	// Second header has no quotes
	QCOMPARE(AsciiFilterPrivate::determineColumns(QStringLiteral("\"header 1\",header2,\"header 3\"\n"),
												  p,
												  separatorSingleCharacter,
												  separatorCharacter,
												  columnNames),
			 3);
	QCOMPARE(columnNames, expectedHeaderNoWhiteSpace);

	// Benchmark
	p.removeQuotes = false;

	expectedHeaderNoWhiteSpace = QVector<QStringView>{QStringLiteral("header1"), QStringLiteral("header2"), QStringLiteral("header3")};
	p.simplifyWhitespaces = false;
	QBENCHMARK {
		QCOMPARE(
			AsciiFilterPrivate::determineColumns(QStringLiteral("header1,header2,header3\n"), p, separatorSingleCharacter, separatorCharacter, columnNames),
			3);
	}
	QCOMPARE(columnNames, expectedHeaderNoWhiteSpace);

	expectedHeader = QStringList{QStringLiteral("header1"), QStringLiteral("header2"), QStringLiteral("header3")};
	p.simplifyWhitespaces = true;
	QBENCHMARK {
		QCOMPARE(AsciiFilterPrivate::determineColumnsSimplifyWhiteSpace(QStringLiteral("header1,header2,header3\n"), p), expectedHeader);
	}
}

void AsciiFilterTest::determineColumnsWhiteSpaces() {
	AsciiFilter::Properties p;
	p.simplifyWhitespaces = true;
	p.skipEmptyParts = true;
	p.startColumn = 1;
	p.endColumn = -1;
	p.separator = QStringLiteral(" ");

	auto expectedHeader = QStringList{QStringLiteral("header1"), QStringLiteral("header2"), QStringLiteral("header3")};
	QCOMPARE(AsciiFilterPrivate::determineColumnsSimplifyWhiteSpace(QStringLiteral("   header1\t     header2     header3    \n"), p), expectedHeader);
}

void AsciiFilterTest::deleteSpreadsheet() {
	QStringList fileContent = {
		QStringLiteral("1;5"),
		QStringLiteral("2;6"),
		QStringLiteral("3;7"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(";");
	p.headerEnabled = false;
	p.columnNamesRaw = QStringLiteral("x, y");
	p.intAsDouble = false;
	filter.setProperties(p);

	filter.preview(savePath, -1);
	// Should not crash just because the previous spreadsheet is anymore available
	filter.preview(savePath, -1);
}

void AsciiFilterTest::saveLoad() {
	QString s;
	{
		AsciiFilter filter;
		auto p = filter.properties();
		p.automaticSeparatorDetection = false;
		p.baseYear = 1200;
		p.columnModesString = QStringLiteral("Int, Int, Double");
		p.columnNamesRaw = QStringLiteral("Column1, Column2, Column3");
		p.commentCharacter = QStringLiteral("ACommenCharacter");
		p.createIndex = true;
		p.createTimestamp = false;
		p.dateTimeFormat = QStringLiteral("ADateTimeFormat");
		p.startColumn = 5;
		p.endColumn = 6;
		p.startRow = 22;
		p.endRow = 28;
		p.headerEnabled = false;
		p.headerLine = 3829;
		p.intAsDouble = true;
		p.nanValue = 29304;
		p.removeQuotes = true;
		p.skipEmptyParts = true;
		filter.setProperties(p);
		QXmlStreamWriter writer(&s);
		filter.save(&writer);
	}

	{
		XmlStreamReader reader(s);
		AsciiFilter filter;
		while (!reader.atEnd()) {
			reader.readNext();
			if (reader.isEndElement())
				break;

			if (!reader.isStartElement())
				continue;

			if (reader.name() == QLatin1String("asciiFilter")) {
				QVERIFY(filter.load(&reader));
				break;
			}
		}
		const auto& ws = reader.warningStrings();
		QVERIFY(ws.isEmpty());

		const auto p = filter.properties();
		QCOMPARE(p.automaticSeparatorDetection, false);
		QCOMPARE(p.baseYear, 1200);
		QCOMPARE(p.columnModesString, QStringLiteral("Int,Int,Double"));
		QCOMPARE(p.columnNamesRaw, QStringLiteral("Column1, Column2, Column3"));
		QCOMPARE(p.commentCharacter, QStringLiteral("ACommenCharacter"));
		QCOMPARE(p.createIndex, true);
		QCOMPARE(p.createTimestamp, false);
		QCOMPARE(p.dateTimeFormat, QStringLiteral("ADateTimeFormat"));
		QCOMPARE(p.startColumn, 5);
		QCOMPARE(p.endColumn, 6);
		QCOMPARE(p.startRow, 22);
		QCOMPARE(p.endRow, 28);
		QCOMPARE(p.headerEnabled, false);
		QCOMPARE(p.headerLine, 3829);
		QCOMPARE(p.intAsDouble, true);
		QCOMPARE(p.nanValue, 29304);
		QCOMPARE(p.removeQuotes, true);
		QCOMPARE(p.skipEmptyParts, true);
	}
}

void AsciiFilterTest::bufferReader() {
	{
		const auto b = QByteArray("Hallo123");
		BufferReader reader(b);

		QByteArray out(10, 0);
		QCOMPARE(reader.readData(out.data(), 10), 8);
		QCOMPARE(out[0], QLatin1Char('H'));
		QCOMPARE(out[1], QLatin1Char('a'));
		QCOMPARE(out[2], QLatin1Char('l'));
		QCOMPARE(out[3], QLatin1Char('l'));
		QCOMPARE(out[4], QLatin1Char('o'));
		QCOMPARE(out[5], QLatin1Char('1'));
		QCOMPARE(out[6], QLatin1Char('2'));
		QCOMPARE(out[7], QLatin1Char('3'));
	}

	{
		const auto b = QByteArray("Hallo1234");
		BufferReader reader(b);

		QByteArray out(10, 0);
		QCOMPARE(reader.readData(out.data(), 2), 2);
		QCOMPARE(out[0], QLatin1Char('H'));
		QCOMPARE(out[1], QLatin1Char('a'));

		QCOMPARE(reader.readData(out.data(), 2), 2);
		QCOMPARE(out[0], QLatin1Char('l'));
		QCOMPARE(out[1], QLatin1Char('l'));

		QCOMPARE(reader.readData(out.data(), 2), 2);
		QCOMPARE(out[0], QLatin1Char('o'));
		QCOMPARE(out[1], QLatin1Char('1'));

		QCOMPARE(reader.readData(out.data(), 5), 3);
		QCOMPARE(out[0], QLatin1Char('2'));
		QCOMPARE(out[1], QLatin1Char('3'));
		QCOMPARE(out[2], QLatin1Char('4'));
		QCOMPARE(reader.canReadLine(), false);
	}
}

void AsciiFilterTest::invalidDataColumnCount() {
	QStringList fileContent = {
		QStringLiteral("Column1;Column2"),
		QStringLiteral("TestText,12,3;567"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	AsciiFilter filter;
	auto p = filter.properties();
	p.automaticSeparatorDetection = false;
	p.separator = QStringLiteral(","); // Wrong separator used
	p.headerEnabled = true;
	filter.setProperties(p);
	filter.preview(savePath, -1);

	const auto status = filter.d_ptr->lastStatus;
	QCOMPARE(status.type(), Status::Type::InvalidNumberDataColumns);
	// QCOMPARE(status.message(),
	// 		 i18n("Invalid number of data columns. First row column count: 1. 2th row column count >= 2. Check if the correct separator is used and the data "
	// 			  "contains same number of columns."));
}

QTEST_MAIN(AsciiFilterTest)
