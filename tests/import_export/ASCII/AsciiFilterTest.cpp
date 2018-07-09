/***************************************************************************
    File                 : AsciiFilterTest.cpp
    Project              : LabPlot
    Description          : Tests for the ascii filter
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "AsciiFilterTest.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

void AsciiFilterTest::initTestCase() {
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();

	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

//##############################################################################
//#################  handling of empty and sparse files ########################
//##############################################################################
void AsciiFilterTest::testEmptyFileAppend() {
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;

	const int rowCount = spreadsheet.rowCount();
	const int colCount = spreadsheet.columnCount();
	const QString fileName = m_dataDir + "empty_file.txt";
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Append;

	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), rowCount);
	QCOMPARE(spreadsheet.columnCount(), colCount);
}

void AsciiFilterTest::testEmptyFilePrepend() {
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;

	const int rowCount = spreadsheet.rowCount();
	const int colCount = spreadsheet.columnCount();
	const QString fileName = m_dataDir + "empty_file.txt";
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Prepend;

	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), rowCount);
	QCOMPARE(spreadsheet.columnCount(), colCount);
}

void AsciiFilterTest::testEmptyFileReplace() {
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;

	const int rowCount = spreadsheet.rowCount();
	const int colCount = spreadsheet.columnCount();
	const QString fileName = m_dataDir + "empty_file.txt";
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), rowCount);
	QCOMPARE(spreadsheet.columnCount(), colCount);
}

void AsciiFilterTest::testEmptyLines01() {
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;
	const QString fileName = m_dataDir + "empty_lines_01.txt";

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
	filter.setSeparatingCharacter("auto");
	filter.setHeaderEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("values"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::Integer);

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
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;
	const QString fileName = m_dataDir + "sparse_file_01.txt";

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
	filter.setSeparatingCharacter(",");
	filter.setHeaderEnabled(true);
	filter.setSimplifyWhitespacesEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("N"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Col1"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Col2"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::Integer);

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
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;
	const QString fileName = m_dataDir + "sparse_file_02.txt";

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
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

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::Numeric);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::Numeric);

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
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;
	const QString fileName = m_dataDir + "sparse_file_03.txt";

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
	filter.setSeparatingCharacter(",");
	filter.setNaNValueToZero(true);
	filter.setSimplifyWhitespacesEnabled(true);
	filter.setSkipEmptyParts(true);
	filter.setHeaderEnabled(true);
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 4);
	QCOMPARE(spreadsheet.columnCount(), 3);

	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("N"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("Col1"));
	QCOMPARE(spreadsheet.column(2)->name(), QLatin1String("Col2"));

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::Numeric);
	QCOMPARE(spreadsheet.column(2)->columnMode(), AbstractColumn::Numeric);

	QCOMPARE(spreadsheet.column(0)->integerAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->integerAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->integerAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->integerAt(3), 3);

 	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 2.);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 1.);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 0.);

	QCOMPARE(spreadsheet.column(2)->valueAt(0), 2.);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0.);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0.);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 0.);
}

//##############################################################################
//################################  header handling ############################
//##############################################################################
void AsciiFilterTest::testHeader01() {
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;
	const QString fileName = m_dataDir + "separator_semicolon.txt";

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(false);
	filter.setVectorNames("");
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2);
}

void AsciiFilterTest::testHeader02() {
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;
	const QString fileName = m_dataDir + "separator_semicolon.txt";

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(true);
	filter.setVectorNames("");
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 2);//out of 3 rows one row is used for the column names (header)
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("1"));

	//TODO: we start with the names "1" and "2" in the spreadsheet and try to rename them to "1" and "1" (names coming from the file)
	//-> the second column with the name "2" will be renamed to "3" because of the current logic in AbstractAspect::uniqueNameFor().
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("3"));
}

void AsciiFilterTest::testHeader03() {
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;
	const QString fileName = m_dataDir + "separator_semicolon.txt";

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(false);
	filter.setVectorNames("x");
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 1); //one column name was specified, we import only one column
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
}

void AsciiFilterTest::testHeader04() {
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;
	const QString fileName = m_dataDir + "separator_semicolon.txt";

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(false);
	filter.setVectorNames("x");
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 1); //one column name was specified -> we import only one column
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
}

void AsciiFilterTest::testHeader05() {
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;
	const QString fileName = m_dataDir + "separator_semicolon.txt";

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
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
	Spreadsheet spreadsheet(0, "test", false);
	AsciiFilter filter;
	const QString fileName = m_dataDir + "separator_semicolon.txt";

	AbstractFileFilter::ImportMode mode = AbstractFileFilter::Replace;
	filter.setSeparatingCharacter(";");
	filter.setHeaderEnabled(false);
	filter.setVectorNames("x y z");
	filter.readDataFromFile(fileName, &spreadsheet, mode);

	QCOMPARE(spreadsheet.rowCount(), 3);
	QCOMPARE(spreadsheet.columnCount(), 2); //thee names were specified, but there're only two columns in the file -> we import only two columns
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("x"));
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("y"));
}

//##############################################################################
//#####################  handling of different separators ######################
//##############################################################################


QTEST_MAIN(AsciiFilterTest)
