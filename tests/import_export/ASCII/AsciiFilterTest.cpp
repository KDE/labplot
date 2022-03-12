/*
    File                 : AsciiFilterTest.cpp
    Project              : LabPlot
    Description          : Tests for the ascii filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AsciiFilterTest.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

void AsciiFilterTest::initTestCase() {
	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

//##############################################################################
//#################  handling of empty and sparse files ########################
//##############################################################################
void AsciiFilterTest::testEmptyFileAppend() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;

	const int rowCount = spreadsheet.rowCount();
	const int colCount = spreadsheet.columnCount();
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/empty_file.txt"));
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Append;

	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), rowCount);
	QCOMPARE(spreadsheet.columnCount(), colCount);
}

void AsciiFilterTest::testEmptyFilePrepend() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;

	const int rowCount = spreadsheet.rowCount();
	const int colCount = spreadsheet.columnCount();
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/empty_file.txt"));
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Prepend;

	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), rowCount);
	QCOMPARE(spreadsheet.columnCount(), colCount);
}

void AsciiFilterTest::testEmptyFileReplace() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;

	const int rowCount = spreadsheet.rowCount();
	const int colCount = spreadsheet.columnCount();
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/empty_file.txt"));
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), rowCount);
	QCOMPARE(spreadsheet.columnCount(), colCount);
}

void AsciiFilterTest::testEmptyLines01() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/empty_lines_01.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

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
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/sparse_file_01.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(",");
	filter.setHeaderEnabled(true);
	filter.setSimplifyWhitespacesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

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
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/sparse_file_02.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(",");
	filter.setNaNValueToZero(false);
	filter.setSimplifyWhitespacesEnabled(true);
	filter.setSkipEmptyParts(false);
	filter.setHeaderEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

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
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/sparse_file_03.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(",");
	filter.setNaNValueToZero(true);
	filter.setSimplifyWhitespacesEnabled(true);
	filter.setSkipEmptyParts(false);
	filter.setHeaderEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

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

//##############################################################################
//################################  header handling ############################
//##############################################################################
void AsciiFilterTest::testHeader01() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(false);
	filter.setVectorNames(QString());
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
}

void AsciiFilterTest::testHeader02() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(true);
	filter.setVectorNames(QString());
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 2);//out of 3 rows one row is used for the column names (header)
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("1"));

	//TODO: we start with the names "1" and "2" in the spreadsheet and try to rename them to "1" and "1" (names coming from the file)
	//-> the second column with the name "2" will be renamed to "3" because of the current logic in AbstractAspect::uniqueNameFor().
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("3"));
}

void AsciiFilterTest::testHeader03() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(false);
	filter.setVectorNames("x");
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 1); //one column name was specified, we import only one column
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
}

void AsciiFilterTest::testHeader04() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(false);
	filter.setVectorNames("x");
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 1); //one column name was specified -> we import only one column
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
}

void AsciiFilterTest::testHeader05() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(false);
	filter.setVectorNames("x y");
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2); //two names were specified -> we import two columns
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));
}

void AsciiFilterTest::testHeader06() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(false);
	filter.setVectorNames("x y z");
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2); //three names were specified, but there're only two columns in the file -> we import only two columns
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));
}

/*!
 * test with a file containing the header in the second line
 * with a subsequent comment line without any comment character.
 * this line shouldn't disturb the detection of numeric column modes.
 */
void AsciiFilterTest::testHeader07() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/comment_header_comment.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("TAB");
	filter.setHeaderLine(2);
	filter.setHeaderEnabled(true);
	filter.setDateTimeFormat(QLatin1String("yyyy-MM-dd hh:mm:ss.zzz"));
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 4);

	//column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("counter"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("t[min]"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("#1ch1"));

	//data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	//values
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
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/comment_header_comment.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("TAB");
	filter.setHeaderLine(2);
	filter.setHeaderEnabled(true);
	filter.setStartRow(2);
	filter.setDateTimeFormat(QLatin1String("yyyy-MM-dd hh:mm:ss.zzz"));
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 3);

	//column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("counter"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("t[min]"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("#1ch1"));

	//data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	//values
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
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_tab_with_header_with_spaces.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);

	//column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("first column"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("second column"));

	//data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	//values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 2);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 3);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 4);
}

//##############################################################################
//#####################  handling of different read ranges #####################
//##############################################################################
void AsciiFilterTest::testColumnRange00() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(false);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//no ranges specified, all rows and columns have to be read
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	//check the values for the first line
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1.716299);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testColumnRange01() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(false);
	filter.setCreateIndexEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//no ranges specified, all rows and columns have to be read plus the additional column for the index
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 4);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);

	//check the values for the first line
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.716299);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testColumnRange02() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(false);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//read all rows and the last two columns only
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 2);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	//check the values for the first line
	QCOMPARE(spreadsheet.column(0)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testColumnRange03() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(false);
	filter.setCreateIndexEnabled(true);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//read all rows and the last two columns only plus the additional column for the index
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	//check the values for the first line
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testColumnRange04() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(false);
	filter.setStartColumn(3);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//read all rows and the last column only
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 1);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);

	//check the values for the first line
	QCOMPARE(spreadsheet.column(0)->valueAt(0), -0.288690);
}

void AsciiFilterTest::testColumnRange05() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(false);
	filter.setStartColumn(3);
	filter.setEndColumn(2);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//wrong column range specified (start>end), nothing to read,
	//empty spreadsheet because of the replace mode
	QCOMPARE(spreadsheet.rowCount(), 0);
	QCOMPARE(spreadsheet.columnCount(), 0);
}

void AsciiFilterTest::testColumnRange06() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(false);
	filter.setCreateIndexEnabled(true);
	filter.setStartColumn(3);
	filter.setEndColumn(2);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//wrong column range specified (start>end), only the index column is created
	QCOMPARE(spreadsheet.rowCount(), 5);
	QCOMPARE(spreadsheet.columnCount(), 1);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
}

void AsciiFilterTest::testRowRange00() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(false);
	filter.setStartRow(3);
	filter.setEndRow(5);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//three rows to read
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	//check the values for the first and for the last lines
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1.711721);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.293267);

	QCOMPARE(spreadsheet.column(0)->valueAt(2), 1.716299);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), -0.494682);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), -0.284112);
}

void AsciiFilterTest::testRowRange01() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(false);
	filter.setStartRow(3);
	filter.setEndRow(10);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//end row larger than the number of available rows, three rows to read
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	//check the values for the first and for the last lines
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1.711721);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.293267);

	QCOMPARE(spreadsheet.column(0)->valueAt(2), 1.716299);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), -0.494682);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), -0.284112);
}

void AsciiFilterTest::testRowRange02() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(false);
	filter.setStartRow(3);
	filter.setEndRow(1);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//start bigger than end, no rows to read
	//wrong row range specified (start>end), nothing to read,
	//spreadsheet is not touched, default number of rows and columns
	//TODO: this is inconsistent with the handling for columns, see testColumnRange05()
	QCOMPARE(spreadsheet.rowCount(), 100);
	QCOMPARE(spreadsheet.columnCount(), 2);
}

void AsciiFilterTest::testRowColumnRange00() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/numeric_data.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(false);
	filter.setStartRow(3);
	filter.setEndRow(5);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//three rows and two columns to read
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	//check the values for the first and for the last lines
	QCOMPARE(spreadsheet.column(0)->valueAt(0), -0.485527);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -0.293267);

	QCOMPARE(spreadsheet.column(0)->valueAt(2), -0.494682);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), -0.284112);
}

//##############################################################################
//#####################  handling of different separators ######################
//##############################################################################


//##############################################################################
//#####################################  quoted strings ########################
//##############################################################################
void AsciiFilterTest::testQuotedStrings00() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/quoted_strings.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(",");
	filter.setHeaderEnabled(false);
	filter.setRemoveQuotesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//three rows and two columns to read
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
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/quoted_strings_with_header.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(",");
	filter.setHeaderEnabled(true);
	filter.setSimplifyWhitespacesEnabled(true);
	filter.setRemoveQuotesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//three rows and two columns to read
	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 4);

	//column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("col1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("col2"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("col3"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("col4"));

	//data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);

	//values
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
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/quoted_strings_one_line.txt"));

	QCOMPARE(QFile::exists(fileName), true);

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(",");
	filter.setHeaderEnabled(false);
	filter.setRemoveQuotesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//three rows and two columns to read
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
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/quoted_strings_one_line_with_header.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(",");
	filter.setHeaderEnabled(true);
	filter.setSimplifyWhitespacesEnabled(true);
	filter.setRemoveQuotesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//three rows and two columns to read
// 	QCOMPARE(spreadsheet.rowCount(), 1);
// 	QCOMPARE(spreadsheet.columnCount(), 4);

	//column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("col1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("col2"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("col3"));
	QCOMPARE(spreadsheet.column(3)->name(), QLatin1String("col4"));

	//data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);

	//values
	QCOMPARE(spreadsheet.column(0)->textAt(0), QLatin1String("a"));
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 1000);
	QCOMPARE(spreadsheet.column(2)->integerAt(0), 201811);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 1.1);
}


//##############################################################################
//###############################  skip comments ###############################
//##############################################################################
void AsciiFilterTest::testComments00() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/multi_line_comment.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(",");
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 2);

	//data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	//values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.1);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 2.2);
}

void AsciiFilterTest::testComments01() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/multi_line_comment_with_empty_lines.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(",");
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.columnCount(), 2);

	//data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	//values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.1);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 2.2);
}

/*!
 * test with an empty comment character
 */
void AsciiFilterTest::testComments02() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/separator_semicolon_with_header.txt"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setCommentCharacter("");
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 3);

	//column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("c1"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("c2"));

	//data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	//values
	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(1)->integerAt(0), 1);

	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(1)->integerAt(1), 2);

	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);
	QCOMPARE(spreadsheet.column(1)->integerAt(2), 3);
}


//##############################################################################
//#########################  handling of datetime data #########################
//##############################################################################
/*!
 * read data containing only two characters for the year - 'yy'. The default year in
 * QDateTime is 1900 . When reading such two-characters DateTime values we want
 * to have the current centure after the import.
 */
void AsciiFilterTest::testDateTime00() {
	Spreadsheet spreadsheet("test", false);
	AsciiFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/datetime_01.csv"));

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace;
	filter.setSeparatingCharacter(",");
	filter.setHeaderEnabled(true);
	filter.setDateTimeFormat(QLatin1String("dd/MM/yy hh:mm:ss"));
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	//spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);

	//column names
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("Date"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Water Pressure"));

	//data types
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	//values
	QDateTime value = QDateTime::fromString(QLatin1String("01/01/2019 00:00:00"), QLatin1String("dd/MM/yyyy hh:mm:ss"));
	QCOMPARE(spreadsheet.column(0)->dateTimeAt(0), value);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 14.7982);

	value = QDateTime::fromString(QLatin1String("01/01/2019 00:00:00"), QLatin1String("dd/MM/yyyy hh:mm:ss"));
	QCOMPARE(spreadsheet.column(0)->dateTimeAt(0), value);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 14.8026);
}

QTEST_MAIN(AsciiFilterTest)
