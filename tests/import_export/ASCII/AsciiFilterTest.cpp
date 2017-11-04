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
}

//##############################################################################
//########################  handling of empty files ############################
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
	QCOMPARE(spreadsheet.column(1)->name(), QLatin1String("1"));
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
