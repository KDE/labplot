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
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

void AsciiFilterTest::initTestCase() {
	KLocalizedString::setApplicationDomain("labplot2");
	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	// TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
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

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
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

	filter.setSeparatingCharacter(QStringLiteral(","));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.setSimplifyWhitespacesEnabled(true);
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

	filter.setSeparatingCharacter(QStringLiteral(","));
	filter.setNaNValueToZero(false);
	filter.setSimplifyWhitespacesEnabled(true);
	filter.setSkipEmptyParts(false);
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
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

	filter.setSeparatingCharacter(QStringLiteral(","));
	filter.setNaNValueToZero(true);
	filter.setSimplifyWhitespacesEnabled(true);
	filter.setSkipEmptyParts(false);
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
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

	filter.setHeaderEnabled(true);
	// filter.setHeaderLine(1);
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
void AsciiFilterTest::testHeader01() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(false);
	filter.setVectorNames(QString());
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
}

void AsciiFilterTest::testHeader02() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.setVectorNames(QString());
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 2); // out of 3 rows one row is used for the column names (header)
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("2"));
}

void AsciiFilterTest::testHeader03() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(false);
	filter.setVectorNames(QStringLiteral("x"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 1); // one column name was specified, we import only one column
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
}

void AsciiFilterTest::testHeader04() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(false);
	filter.setVectorNames(QStringLiteral("x"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 1); // one column name was specified -> we import only one column
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
}

void AsciiFilterTest::testHeader05() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(false);
	filter.setVectorNames(QStringLiteral("x y"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2); // two names were specified -> we import two columns
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));
}

void AsciiFilterTest::testHeader06() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(false);
	filter.setVectorNames(QStringLiteral("x y z"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2); // three names were specified, but there're only two columns in the file -> we import only two columns
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));
}

/*!
 * test with a file containing the header in the second line
 * with a subsequent comment line without any comment character.
 * this line shouldn't disturb the detection of numeric column modes.
 */
void AsciiFilterTest::testHeader07() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/comment_header_comment.txt"));

	filter.setSeparatingCharacter(QStringLiteral("TAB"));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(2);
	filter.setDateTimeFormat(QLatin1String("yyyy-MM-dd hh:mm:ss.zzz"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 4);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("counter"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("t[min]"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("#1ch1"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 0);
	QCOMPARE((bool)std::isnan(spreadsheet.column(1)->valueAt(0)), true);
	QCOMPARE((bool)std::isnan(spreadsheet.column(2)->valueAt(0)), true);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0.0513);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0.3448);

	QCOMPARE(spreadsheet.column(0)->integerAt(2), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 0.1005);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0.3418);

	QCOMPARE(spreadsheet.column(0)->integerAt(3), 3);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 0.1516);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 0.3433);
}

/*!
 * test with a file containing the header in the second line
 * with a subsequent comment line ignored by using startRow.
 */
void AsciiFilterTest::testHeader07a() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/comment_header_comment.txt"));

	filter.setSeparatingCharacter(QStringLiteral("TAB"));
	filter.setHeaderLine(2);
	filter.setHeaderEnabled(true);
	filter.setStartRow(2);
	filter.setDateTimeFormat(QLatin1String("yyyy-MM-dd hh:mm:ss.zzz"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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
void AsciiFilterTest::testHeader08() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_tab_with_header_with_spaces.txt"));

	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("first column"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("second column"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 2);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 3);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 4);
}

/*!
 * test the handling of duplicated columns names provided by the user.
 */
void AsciiFilterTest::testHeader09() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(false);
	filter.setVectorNames(QStringList{QStringLiteral("x"), QStringLiteral("x")});
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("x 1")); // the duplicated name was renamed
}

/*!
 * test the handling of duplicated columns in the file to be imported.
 */
void AsciiFilterTest::testHeader10() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon_with_header_duplicated_names.txt"));

	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("x 1")); // the duplicated name was renamed
}

/*!
 * test the handling of duplicated columns in the file to be imported.
 */
void AsciiFilterTest::testHeader11() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/column_names.txt"));

	filter.setSeparatingCharacter(QStringLiteral(" "));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 1);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("A"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("B"));

	// import the second file with reversed column names into the same spreadsheet
	AsciiFilter filter2; // create a new filter so we go through the prepare logic from scratch for the 2nd file
	const QString& fileName2 = QFINDTESTDATA(QLatin1String("data/column_names_reversed.txt"));
	filter2.setSeparatingCharacter(QStringLiteral(" "));
	filter.setHeaderEnabled(true);
	filter2.setHeaderLine(1);
	filter2.readDataFromFile(fileName2, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 1);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("B"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("A"));
}

/*!
 * test the handling of column names with and without header
 */
void AsciiFilterTest::testHeader11a() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/column_names.txt"));

	filter.setSeparatingCharacter(QStringLiteral(" "));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 1);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("A"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("B"));

	AsciiFilter filter2;
	filter2.setHeaderEnabled(false);
	filter2.setSeparatingCharacter(QStringLiteral(" "));
	filter2.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.rowCount(), 2);
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Column 1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Column 2"));
}

/*!
 * test column modes when header is in second line
 */
void AsciiFilterTest::testHeader12() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/lds_test.csv"));

	// filter.setSeparatingCharacter(QStringLiteral(" "));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(2);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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
void AsciiFilterTest::testColumnRange00() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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

void AsciiFilterTest::testColumnRange01() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.setCreateIndexEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// no ranges specified, all rows and columns have to be read plus the additional column for the index
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 4);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first line
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.716299);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testColumnRange02() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// read all rows and the last two columns only
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 2);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first line
	QCOMPARE(spreadsheet.column(0)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testColumnRange03() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.setCreateIndexEnabled(true);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// read all rows and the last two columns only plus the additional column for the index
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first line
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testColumnRange04() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.setStartColumn(3);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// read all rows and the last column only
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 1);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);

	// check the values for the first line
	QCOMPARE(spreadsheet.column(0)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testColumnRange05() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.setStartColumn(3);
	filter.setEndColumn(2);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// wrong column range specified (start>end), nothing to read,
	// empty spreadsheet because of the replace mode
	QCOMPARE(spreadsheet.rowCount(), 0);
	QCOMPARE(spreadsheet.columnCount(), 0);
}

void AsciiFilterTest::testColumnRange06() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.setCreateIndexEnabled(true);
	filter.setStartColumn(3);
	filter.setEndColumn(2);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// wrong column range specified (start>end), only the index column is created
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 1);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
}

void AsciiFilterTest::testRowRange00() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.setStartRow(3);
	filter.setEndRow(5);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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

void AsciiFilterTest::testRowRange01() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.setStartRow(3);
	filter.setEndRow(10);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.setStartRow(3);
	filter.setEndRow(1);
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
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.setStartRow(3);
	filter.setEndRow(5);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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

// ##############################################################################
// #####################################  quoted strings ########################
// ##############################################################################
void AsciiFilterTest::testQuotedStrings00() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/quoted_strings.txt"));

	filter.setSeparatingCharacter(QStringLiteral(","));
	filter.setHeaderEnabled(false);
	filter.setRemoveQuotesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// three rows and two columns to read
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
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/quoted_strings_with_header.txt"));

	filter.setSeparatingCharacter(QStringLiteral(","));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.setSimplifyWhitespacesEnabled(true);
	filter.setRemoveQuotesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/quoted_strings_one_line.txt"));

	QCOMPARE(QFile::exists(fileName), true);

	filter.setSeparatingCharacter(QStringLiteral(","));
	filter.setHeaderEnabled(false);
	filter.setRemoveQuotesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// three rows and two columns to read
	// 	QCOMPARE(spreadsheet.rowCount(), 1);
	// 	QCOMPARE(spreadsheet.columnCount(), 4);

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
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/quoted_strings_one_line_with_header.txt"));

	filter.setSeparatingCharacter(QStringLiteral(","));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.setSimplifyWhitespacesEnabled(true);
	filter.setRemoveQuotesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// three rows and two columns to read
	// 	QCOMPARE(spreadsheet.rowCount(), 1);
	// 	QCOMPARE(spreadsheet.columnCount(), 4);

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
}

/*!
 * test quoted text having separators inside - the text between quotes shouldn't be splitted into separate columns.
 */
void AsciiFilterTest::testQuotedStrings04() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/quoted_strings_with_separator_inside.csv"));

	filter.setHeaderLine(1);
	filter.setSimplifyWhitespacesEnabled(true); // TODO: this shouldn't be required, but QString::split() seems to introduce blanks...
	filter.setRemoveQuotesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// three rows and two columns to read
	//	QCOMPARE(spreadsheet.rowCount(), 2);
	//	QCOMPARE(spreadsheet.columnCount(), 3);

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
 * test quoted text having separators inside - a JSON file has a similar structure and we should't crash because of this "wrong" data.
 */
void AsciiFilterTest::testQuotedStrings05() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/object.json"));

	filter.setSimplifyWhitespacesEnabled(true); // TODO: this shouldn't be required, but QString::split() seems to introduce blanks...
	filter.setRemoveQuotesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// everything should be read into one single text column.
	// the actuall content is irrelevant, we just need to make sure we don't crash because of such wrong content
	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
}

// ##############################################################################
// ################################## locales ###################################
// ##############################################################################
void AsciiFilterTest::testUtf8Cyrillic() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/utf8_cyrillic.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// column names
	QCOMPARE(spreadsheet.column(0)->name(), QString::fromUtf8("перший_стовпець"));
	QCOMPARE(spreadsheet.column(1)->name(), QString::fromUtf8("другий_стовпець"));

	// data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// values
	QCOMPARE(spreadsheet.column(0)->textAt(0), QString::fromUtf8("тест1"));
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 1);

	QCOMPARE(spreadsheet.column(0)->textAt(1), QString::fromUtf8("тест2"));
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 2);
}

// ##############################################################################
// ###############################  skip comments ###############################
// ##############################################################################
void AsciiFilterTest::testComments00() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/multi_line_comment.txt"));

	filter.setHeaderEnabled(false);
	filter.setSeparatingCharacter(QStringLiteral(","));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/multi_line_comment_with_empty_lines.txt"));

	filter.setHeaderEnabled(false);
	filter.setSeparatingCharacter(QStringLiteral(","));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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

/*!
 * test with an empty comment character
 */
void AsciiFilterTest::testComments02() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon_with_header.txt"));

	filter.setCommentCharacter(QString());
	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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
/*!
 * read data containing only two characters for the year - 'yy'. The default year in
 * QDateTime is 1900 . When reading such two-characters DateTime values we want
 * to have the current centure after the import.
 */
void AsciiFilterTest::testDateTime00() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/datetime_01.csv"));

	filter.setSeparatingCharacter(QStringLiteral(","));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.setDateTimeFormat(QLatin1String("dd/MM/yy hh:mm:ss"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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

/*!
 * same as in the previous test, but with the auto-detection of the datetime format
 */
void AsciiFilterTest::testDateTime01() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/datetime_01.csv"));

	filter.setSeparatingCharacter(QStringLiteral(","));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.setDateTimeFormat(QLatin1String()); // auto detect the format
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/datetime-hex.dat"));

	filter.setHeaderEnabled(false);
	filter.setSeparatingCharacter(QStringLiteral("|"));
	filter.setDateTimeFormat(QLatin1String("yyyyMMddhhmmss"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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

void AsciiFilterTest::testMatrixHeader() {
	Matrix matrix(QStringLiteral("test"), false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	filter.setSeparatingCharacter(QStringLiteral("auto"));
	filter.setHeaderEnabled(false);
	filter.readDataFromFile(fileName, &matrix, AbstractFileFilter::ImportMode::Replace);

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
	// create the first spreadsheet with the source data
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	spreadsheet.setColumnCount(2);
	spreadsheet.setRowCount(3);

	auto* col = spreadsheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c1"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	col = spreadsheet.column(1);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c2"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	// create the second spreadsheet with one single column calculated via a formula from the first spreadsheet
	Spreadsheet spreadsheetFormula(QStringLiteral("formula"), false);
	spreadsheetFormula.setColumnCount(1);
	spreadsheetFormula.setRowCount(3);
	col = spreadsheetFormula.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);

	QStringList variableNames{QLatin1String("x"), QLatin1String("y")};
	QVector<Column*> variableColumns{spreadsheet.column(0), spreadsheet.column(1)};
	col->setFormula(QLatin1String("x+y"), variableNames, variableColumns, true);
	col->updateFormula();

	// check the results of the calculation first
	QCOMPARE(spreadsheetFormula.columnCount(), 1);
	QCOMPARE(spreadsheetFormula.rowCount(), 3);
	QCOMPARE(col->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(col->valueAt(0), 20.);
	QCOMPARE(col->valueAt(1), 40.);
	QCOMPARE(col->valueAt(2), 60.);

	// import the data into the source spreadsheet
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon_with_header.txt"));
	filter.setCommentCharacter(QString());
	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// re-check the results of the calculation
	QCOMPARE(spreadsheetFormula.columnCount(), 1);
	QCOMPARE(spreadsheetFormula.rowCount(), 3);
	QCOMPARE(col->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(col->valueAt(0), 2.);
	QCOMPARE(col->valueAt(1), 4.);
	QCOMPARE(col->valueAt(2), 6.);
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
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon_with_header.txt"));
	filter.setCommentCharacter(QString());
	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, spreadsheet, AbstractFileFilter::ImportMode::Replace);

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
	// create the spreadsheet with the source data
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	spreadsheet.setColumnCount(2);
	spreadsheet.setRowCount(3);

	auto* col = spreadsheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c1"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	col = spreadsheet.column(1);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("c2"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	// create a xy-curve with the both columns in the source spreadsheet and check the ranges
	CartesianPlot p(QStringLiteral("plot"));
	auto* curve = new XYCurve(QStringLiteral("curve"));
	p.addChild(curve);
	curve->setXColumn(spreadsheet.column(0));
	curve->setYColumn(spreadsheet.column(1));

	auto rangeX = p.range(Dimension::X);
	QCOMPARE(rangeX.start(), 10);
	QCOMPARE(rangeX.end(), 30);

	auto rangeY = p.range(Dimension::Y);
	QCOMPARE(rangeY.start(), 10);
	QCOMPARE(rangeY.end(), 30);

	// import the data into the source spreadsheet
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon_with_header.txt"));
	filter.setCommentCharacter(QString());
	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// re-check the plot ranges with the new data
	rangeX = p.range(Dimension::X);
	QCOMPARE(rangeX.start(), 1);
	QCOMPARE(rangeX.end(), 3);

	rangeY = p.range(Dimension::Y);
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
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon_with_header.txt"));
	filter.setCommentCharacter(QString());
	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, spreadsheet, AbstractFileFilter::ImportMode::Replace);

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
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon_with_header.txt"));
	filter.setCommentCharacter(QString());
	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, spreadsheet, AbstractFileFilter::ImportMode::Replace);

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
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon_with_header.txt"));
	filter.setCommentCharacter(QString());
	filter.setSeparatingCharacter(QStringLiteral(";"));
	filter.setHeaderEnabled(true);
	filter.setHeaderLine(1);
	filter.readDataFromFile(fileName, spreadsheet, AbstractFileFilter::ImportMode::Replace);

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
	filter.setHeaderEnabled(false);

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

QTEST_MAIN(AsciiFilterTest)
