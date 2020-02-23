/***************************************************************************
    File                 : SpreadsheetTest.cpp
    Project              : LabPlot
    Description          : Tests for the Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
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

#include <QApplication>
#include <QClipboard>

#include "SpreadsheetTest.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "../src/commonfrontend/spreadsheet/SpreadsheetView.h"

void SpreadsheetTest::initTestCase() {
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

/*!
   insert two columns with float values into an empty spreadsheet
*/
void SpreadsheetTest::testCopyPaste00() {
	Spreadsheet* sheet = new Spreadsheet("test", false);

	const QString str = "10.0 100.0\n20.0 200.0";

	QApplication::clipboard()->setText(str);

	SpreadsheetView* view = new SpreadsheetView(sheet, false);
	view->pasteIntoSelection();

	//column modes
	QCOMPARE(sheet->column(0)->columnMode(), AbstractColumn::Numeric);
	QCOMPARE(sheet->column(1)->columnMode(), AbstractColumn::Numeric);

	//values
	QCOMPARE(sheet->column(0)->valueAt(0), 10.0);
	QCOMPARE(sheet->column(1)->valueAt(0), 100.0);
	QCOMPARE(sheet->column(0)->valueAt(1), 20.0);
	QCOMPARE(sheet->column(1)->valueAt(1), 200.0);
}

/*!
   insert one column with integer values and one column with float numbers into an empty spreadsheet.
   the first column has to be converted to integer column.
*/
void SpreadsheetTest::testCopyPaste01() {
	Spreadsheet* sheet = new Spreadsheet("test", false);

	const QString str = "10 100.0\n20 200.0";

	QApplication::clipboard()->setText(str);

	SpreadsheetView* view = new SpreadsheetView(sheet, false);
	view->pasteIntoSelection();

	//column modes
	QCOMPARE(sheet->column(0)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(sheet->column(1)->columnMode(), AbstractColumn::Numeric);

	//values
	QCOMPARE(sheet->column(0)->integerAt(0), 10);
	QCOMPARE(sheet->column(1)->valueAt(0), 100.0);
	QCOMPARE(sheet->column(0)->integerAt(1), 20);
	QCOMPARE(sheet->column(1)->valueAt(1), 200.0);
}

/*!
   insert irregular data, new columns should be added appropriately.
*/
void SpreadsheetTest::testCopyPaste02() {
	Spreadsheet* sheet = new Spreadsheet("test", false);

	const QString str = "0\n"
						"10 20\n"
						"11 21 31\n"
						"12 22 32 42\n"
						"13 23\n"
						"14";

	QApplication::clipboard()->setText(str);

	int rows = sheet->rowCount();

	SpreadsheetView* view = new SpreadsheetView(sheet, false);
	view->pasteIntoSelection();

	//spreadsheet size
	QCOMPARE(sheet->columnCount(), 4);
	QCOMPARE(sheet->rowCount(), rows);

	//column modes
	QCOMPARE(sheet->column(0)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(sheet->column(1)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(sheet->column(2)->columnMode(), AbstractColumn::Integer);
	QCOMPARE(sheet->column(3)->columnMode(), AbstractColumn::Integer);

	//values
	QCOMPARE(sheet->column(0)->integerAt(0), 0);
	QCOMPARE(sheet->column(1)->integerAt(0), 0);
	QCOMPARE(sheet->column(2)->integerAt(0), 0);
	QCOMPARE(sheet->column(3)->integerAt(0), 0);

	QCOMPARE(sheet->column(0)->integerAt(1), 10);
	QCOMPARE(sheet->column(1)->integerAt(1), 20);
	QCOMPARE(sheet->column(2)->integerAt(1), 0);
	QCOMPARE(sheet->column(3)->integerAt(1), 0);

	QCOMPARE(sheet->column(0)->integerAt(2), 11);
	QCOMPARE(sheet->column(1)->integerAt(2), 21);
	QCOMPARE(sheet->column(2)->integerAt(2), 31);
	QCOMPARE(sheet->column(3)->integerAt(2), 0);

	QCOMPARE(sheet->column(0)->integerAt(3), 12);
	QCOMPARE(sheet->column(1)->integerAt(3), 22);
	QCOMPARE(sheet->column(2)->integerAt(3), 32);
	QCOMPARE(sheet->column(3)->integerAt(3), 42);

	QCOMPARE(sheet->column(0)->integerAt(4), 13);
	QCOMPARE(sheet->column(1)->integerAt(4), 23);
	QCOMPARE(sheet->column(2)->integerAt(4), 0);
	QCOMPARE(sheet->column(3)->integerAt(4), 0);

	QCOMPARE(sheet->column(0)->integerAt(5), 14);
	QCOMPARE(sheet->column(1)->integerAt(5), 0);
	QCOMPARE(sheet->column(2)->integerAt(5), 0);
	QCOMPARE(sheet->column(3)->integerAt(5), 0);
}

QTEST_MAIN(SpreadsheetTest)
