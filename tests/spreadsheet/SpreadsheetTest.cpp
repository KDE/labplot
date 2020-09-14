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

#include "SpreadsheetTest.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/Project.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"

#include <QApplication>
#include <QClipboard>
#include <QUndoStack>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif

extern "C" {
#include <gsl/gsl_math.h>
}

void SpreadsheetTest::initTestCase() {
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
	QLocale::setDefault(QLocale(QLocale::C));
}

//**********************************************************
//****************** Copy&Paste tests **********************
//**********************************************************

//**********************************************************
//********** Handling of different columns modes ***********
//**********************************************************
/*!
   insert two columns with float values into an empty spreadsheet
*/
void SpreadsheetTest::testCopyPasteColumnMode00() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = "10.0 100.0\n20.0 200.0";
	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	//spreadsheet size
	QCOMPARE(sheet.columnCount(), 2);
	QCOMPARE(sheet.rowCount(), 100);

	//column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	//values
	QCOMPARE(sheet.column(0)->valueAt(0), 10.0);
	QCOMPARE(sheet.column(1)->valueAt(0), 100.0);
	QCOMPARE(sheet.column(0)->valueAt(1), 20.0);
	QCOMPARE(sheet.column(1)->valueAt(1), 200.0);
}

/*!
   insert one column with integer values and one column with float numbers into an empty spreadsheet.
   the first column has to be converted to integer column.
*/
void SpreadsheetTest::testCopyPasteColumnMode01() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = "10 " + QString::number(std::numeric_limits<long long>::min())
						+ "\n20 " + QString::number(std::numeric_limits<long long>::max());
	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	//spreadsheet size
	QCOMPARE(sheet.columnCount(), 2);
	QCOMPARE(sheet.rowCount(), 100);

	//column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::BigInt);

	//values
	QCOMPARE(sheet.column(0)->integerAt(0), 10);
	QCOMPARE(sheet.column(1)->bigIntAt(0), std::numeric_limits<long long>::min());
	QCOMPARE(sheet.column(0)->integerAt(1), 20);
	QCOMPARE(sheet.column(1)->bigIntAt(1), std::numeric_limits<long long>::max());
}

/*!
   insert one column with integer and one column with big integer values into an empty spreadsheet.
   the first column has to be converted to integer column, the second to big integer.
*/
void SpreadsheetTest::testCopyPasteColumnMode02() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = "10 100.0\n20 200.0";
	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	//spreadsheet size
	QCOMPARE(sheet.columnCount(), 2);
	QCOMPARE(sheet.rowCount(), 100);

	//column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	//values
	QCOMPARE(sheet.column(0)->integerAt(0), 10);
	QCOMPARE(sheet.column(1)->valueAt(0), 100.0);
	QCOMPARE(sheet.column(0)->integerAt(1), 20);
	QCOMPARE(sheet.column(1)->valueAt(1), 200.0);
}


/*!
   Properly handle empty values in the tab separated data.
*/
void SpreadsheetTest::testCopyPasteColumnMode03() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = "1000		1000		1000\n"
						"985		985		985\n"
						"970	-7.06562	970		970\n"
						"955	-5.93881	955		955\n"
						"940	-4.97594	940	-4.97594	940";

	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	//spreadsheet size
	QCOMPARE(sheet.columnCount(), 5);
	QCOMPARE(sheet.rowCount(), 100);

	//column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(sheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(sheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Integer);

	//values
	QCOMPARE(sheet.column(0)->integerAt(0), 1000);
	QCOMPARE((bool)std::isnan(sheet.column(1)->valueAt(0)), true);
	QCOMPARE(sheet.column(2)->integerAt(0), 1000);
	QCOMPARE((bool)std::isnan(sheet.column(3)->valueAt(0)), true);
	QCOMPARE(sheet.column(4)->integerAt(0), 1000);

	QCOMPARE(sheet.column(0)->integerAt(1), 985);
	QCOMPARE((bool)std::isnan(sheet.column(1)->valueAt(1)), true);
	QCOMPARE(sheet.column(2)->integerAt(1), 985);
	QCOMPARE((bool)std::isnan(sheet.column(3)->valueAt(1)), true);
	QCOMPARE(sheet.column(4)->integerAt(1), 985);

	QCOMPARE(sheet.column(0)->integerAt(2), 970);
	QCOMPARE(sheet.column(1)->valueAt(2), -7.06562);
	QCOMPARE(sheet.column(2)->integerAt(2), 970);
	QCOMPARE((bool)std::isnan(sheet.column(3)->valueAt(2)), true);
	QCOMPARE(sheet.column(4)->integerAt(2), 970);

	QCOMPARE(sheet.column(0)->integerAt(3), 955);
	QCOMPARE(sheet.column(1)->valueAt(3), -5.93881);
	QCOMPARE(sheet.column(2)->integerAt(3), 955);
	QCOMPARE((bool)std::isnan(sheet.column(3)->valueAt(3)), true);
	QCOMPARE(sheet.column(4)->integerAt(3), 955);

	QCOMPARE(sheet.column(0)->integerAt(4), 940);
	QCOMPARE(sheet.column(1)->valueAt(4), -4.97594);
	QCOMPARE(sheet.column(2)->integerAt(4), 940);
	QCOMPARE(sheet.column(1)->valueAt(4), -4.97594);
	QCOMPARE(sheet.column(4)->integerAt(4), 940);
}

//**********************************************************
//********* Handling of spreadsheet size changes ***********
//**********************************************************
/*!
   insert irregular data, new columns should be added appropriately.
*/
void SpreadsheetTest::testCopyPasteSizeChange00() {
	Project project;
	Spreadsheet* sheet = new Spreadsheet("test", false);
	project.addChild(sheet);
	sheet->setColumnCount(2);
	sheet->setRowCount(100);

	const QString str = "0\n"
						"10 20\n"
						"11 21 31\n"
						"12 22 32 42\n"
						"13 23\n"
						"14";
	QApplication::clipboard()->setText(str);


	SpreadsheetView view(sheet, false);
	view.pasteIntoSelection();

	//spreadsheet size
	QCOMPARE(sheet->columnCount(), 4);
	QCOMPARE(sheet->rowCount(), 100);

	//column modes
	QCOMPARE(sheet->column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet->column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet->column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet->column(3)->columnMode(), AbstractColumn::ColumnMode::Integer);

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


	//undo the changes and check the results again
	project.undoStack()->undo();

	//spreadsheet size
	QCOMPARE(sheet->columnCount(), 2);
	QCOMPARE(sheet->rowCount(), 100);

	//column modes
	QCOMPARE(sheet->column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(sheet->column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	//values
	QCOMPARE((bool)std::isnan(sheet->column(0)->valueAt(0)), true);
	QCOMPARE((bool)std::isnan(sheet->column(1)->valueAt(0)), true);

	QCOMPARE((bool)std::isnan(sheet->column(0)->valueAt(1)), true);
	QCOMPARE((bool)std::isnan(sheet->column(1)->valueAt(1)), true);

	QCOMPARE((bool)std::isnan(sheet->column(0)->valueAt(2)), true);
	QCOMPARE((bool)std::isnan(sheet->column(1)->valueAt(2)), true);

	QCOMPARE((bool)std::isnan(sheet->column(0)->valueAt(3)), true);
	QCOMPARE((bool)std::isnan(sheet->column(1)->valueAt(3)), true);

	QCOMPARE((bool)std::isnan(sheet->column(0)->valueAt(4)), true);
	QCOMPARE((bool)std::isnan(sheet->column(1)->valueAt(4)), true);

	QCOMPARE((bool)std::isnan(sheet->column(0)->valueAt(5)), true);
	QCOMPARE((bool)std::isnan(sheet->column(1)->valueAt(5)), true);
}

/*!
   insert the data at the edge of the spreadsheet and paste the data.
   the spreadsheet has to be extended accordingly
*/
void SpreadsheetTest::testCopyPasteSizeChange01() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = "1.1 2.2\n"
						"3.3 4.4";
	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.goToCell(1, 1); //havigate to the edge of the spreadsheet
	view.pasteIntoSelection();

	//spreadsheet size
	QCOMPARE(sheet.columnCount(), 3);
	QCOMPARE(sheet.rowCount(), 100);

	//column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(sheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Numeric);

	//values
	QCOMPARE((bool)std::isnan(sheet.column(0)->valueAt(0)), true);
	QCOMPARE((bool)std::isnan(sheet.column(1)->valueAt(0)), true);
	QCOMPARE((bool)std::isnan(sheet.column(2)->valueAt(0)), true);

	QCOMPARE((bool)std::isnan(sheet.column(0)->valueAt(1)), true);
	QCOMPARE(sheet.column(1)->valueAt(1), 1.1);
	QCOMPARE(sheet.column(2)->valueAt(1), 2.2);

	QCOMPARE((bool)std::isnan(sheet.column(0)->valueAt(2)), true);
	QCOMPARE(sheet.column(1)->valueAt(2), 3.3);
	QCOMPARE(sheet.column(2)->valueAt(2), 4.4);
}

/////////////////////////////// Sorting tests ////////////////////////////
// single column

/*
 * check sorting single column of double values with NaN ascending
 */
void SpreadsheetTest::testSortSingleNumeric1() {
	const QVector<double> xData{0.5, -0.2, GSL_NAN, 2.0, -1.0};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->replaceValues(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, true);

	//values
	QCOMPARE(col->valueAt(0), -1.0);
	QCOMPARE(col->valueAt(1), -0.2);
	QCOMPARE(col->valueAt(2), 0.5);
	QCOMPARE(col->valueAt(3), 2.0);
}

/*
 * check sorting single column of double values with NaN descending
 */
void SpreadsheetTest::testSortSingleNumeric2() {
	const QVector<double> xData{0.5, -0.2, GSL_NAN, 2.0, -1.0};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->replaceValues(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, false);

	//values
	QCOMPARE(col->valueAt(0), 2.0);
	QCOMPARE(col->valueAt(1), 0.5);
	QCOMPARE(col->valueAt(2), -0.2);
	QCOMPARE(col->valueAt(3), -1.0);
}

/*
 * check sorting single column of integer values with empty entries ascending
 */
void SpreadsheetTest::testSortSingleInteger1() {
	const QVector<int> xData1{4, 5, 2};
	const QVector<int> xData2{3, 6, -1};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col->replaceInteger(0, xData1);
	col->replaceInteger(4, xData2);

	// sort
	sheet.sortColumns(nullptr, {col}, true);

	//values
	QCOMPARE(col->integerAt(0), -1);
	QCOMPARE(col->integerAt(1), 0);
	QCOMPARE(col->integerAt(2), 2);
	QCOMPARE(col->integerAt(3), 3);
	QCOMPARE(col->integerAt(4), 4);
	QCOMPARE(col->integerAt(5), 5);
	QCOMPARE(col->integerAt(6), 6);
}

/*
 * check sorting single column of integer values with empty entries ascending
 */
void SpreadsheetTest::testSortSingleInteger2() {
	const QVector<int> xData1{4, 5, 2};
	const QVector<int> xData2{3, 6, -1};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col->replaceInteger(0, xData1);
	col->replaceInteger(4, xData2);

	// sort
	sheet.sortColumns(nullptr, {col}, false);

	//values
	QCOMPARE(col->integerAt(6), -1);
	QCOMPARE(col->integerAt(5), 0);
	QCOMPARE(col->integerAt(4), 2);
	QCOMPARE(col->integerAt(3), 3);
	QCOMPARE(col->integerAt(2), 4);
	QCOMPARE(col->integerAt(1), 5);
	QCOMPARE(col->integerAt(0), 6);
}

/*
 * check sorting single column of big int values with empty entries ascending
 */
void SpreadsheetTest::testSortSingleBigInt1() {
	const QVector<qint64> xData1{40000000000, 50000000000, 20000000000};
	const QVector<qint64> xData2{30000000000, 60000000000, -10000000000};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::BigInt);
	col->replaceBigInt(0, xData1);
	col->replaceBigInt(4, xData2);

	// sort
	sheet.sortColumns(nullptr, {col}, true);

	//values
	QCOMPARE(col->bigIntAt(0), -10000000000ll);
	QCOMPARE(col->bigIntAt(1), 0);
	QCOMPARE(col->bigIntAt(2), 20000000000ll);
	QCOMPARE(col->bigIntAt(3), 30000000000ll);
	QCOMPARE(col->bigIntAt(4), 40000000000ll);
	QCOMPARE(col->bigIntAt(5), 50000000000ll);
	QCOMPARE(col->bigIntAt(6), 60000000000ll);
}

/*
 * check sorting single column of big int values with empty entries descending
 */
void SpreadsheetTest::testSortSingleBigInt2() {
	const QVector<qint64> xData1{40000000000, 50000000000, 20000000000};
	const QVector<qint64> xData2{30000000000, 60000000000, -10000000000};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::BigInt);
	col->replaceBigInt(0, xData1);
	col->replaceBigInt(4, xData2);

	// sort
	sheet.sortColumns(nullptr, {col}, false);

	//values
	QCOMPARE(col->bigIntAt(6), -10000000000ll);
	QCOMPARE(col->bigIntAt(5), 0);
	QCOMPARE(col->bigIntAt(4), 20000000000ll);
	QCOMPARE(col->bigIntAt(3), 30000000000ll);
	QCOMPARE(col->bigIntAt(2), 40000000000ll);
	QCOMPARE(col->bigIntAt(1), 50000000000ll);
	QCOMPARE(col->bigIntAt(0), 60000000000ll);
}

/*
 * check sorting single column of text with empty entries ascending
 */
void SpreadsheetTest::testSortSingleText1() {
	const QVector<QString> xData{"ben", "amy", "eddy", "", "carl", "dan"};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(1);
	sheet.setRowCount(8);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Text);
	col->replaceTexts(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, true);

	//values
	QCOMPARE(col->textAt(0), QLatin1String("amy"));
	QCOMPARE(col->textAt(1), QLatin1String("ben"));
	QCOMPARE(col->textAt(2), QLatin1String("carl"));
	QCOMPARE(col->textAt(3), QLatin1String("dan"));
	QCOMPARE(col->textAt(4), QLatin1String("eddy"));
	QCOMPARE(col->textAt(5), QLatin1String(""));
	QCOMPARE(col->textAt(6), QLatin1String(""));
}

/*
 * check sorting single column of text with empty entries descending
 */
void SpreadsheetTest::testSortSingleText2() {
	const QVector<QString> xData = {"ben", "amy", "eddy", "", "carl", "dan"};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(1);
	sheet.setRowCount(8);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Text);
	col->replaceTexts(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, false);

	//values
	QCOMPARE(col->textAt(0), QLatin1String("eddy"));
	QCOMPARE(col->textAt(1), QLatin1String("dan"));
	QCOMPARE(col->textAt(2), QLatin1String("carl"));
	QCOMPARE(col->textAt(3), QLatin1String("ben"));
	QCOMPARE(col->textAt(4), QLatin1String("amy"));
	QCOMPARE(col->textAt(5), QLatin1String(""));
	QCOMPARE(col->textAt(6), QLatin1String(""));
}

/*
 * check sorting single column of datetimes with invalid entries ascending
 */
void SpreadsheetTest::testSortSingleDateTime1() {
	const QVector<QDateTime> xData{
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 12)),
		QDateTime(QDate(2020, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 29), QTime(12, 12, 12)),	// invalid
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 13)),
		QDateTime(QDate(2020, 02, 29), QTime(11, 12, 12)),
		};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(1);
	sheet.setRowCount(8);
	auto* col{sheet.column(0)};
	col->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	col->replaceDateTimes(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, true);

	//values
	QCOMPARE(col->dateTimeAt(0), QDateTime(QDate(2019, 02, 28), QTime(12, 12, 12)));
	QCOMPARE(col->dateTimeAt(1), QDateTime(QDate(2020, 02, 28), QTime(12, 12, 12)));
	QCOMPARE(col->dateTimeAt(2), QDateTime(QDate(2020, 02, 29), QTime(11, 12, 12)));
	QCOMPARE(col->dateTimeAt(3), QDateTime(QDate(2020, 02, 29), QTime(12, 12, 12)));
	QCOMPARE(col->dateTimeAt(4), QDateTime(QDate(2020, 02, 29), QTime(12, 12, 13)));
}

/*
 * check sorting single column of datetimes with invalid entries descending
 */
void SpreadsheetTest::testSortSingleDateTime2() {

	const QVector<QDateTime> xData{
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 12)),
		QDateTime(QDate(2020, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 29), QTime(12, 12, 12)),	// invalid
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 13)),
		QDateTime(QDate(2020, 02, 29), QTime(11, 12, 12)),
		};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(1);
	sheet.setRowCount(8);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	col->replaceDateTimes(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, false);

	//values
	QCOMPARE(col->dateTimeAt(4), QDateTime(QDate(2019, 02, 28), QTime(12, 12, 12)));
	QCOMPARE(col->dateTimeAt(3), QDateTime(QDate(2020, 02, 28), QTime(12, 12, 12)));
	QCOMPARE(col->dateTimeAt(2), QDateTime(QDate(2020, 02, 29), QTime(11, 12, 12)));
	QCOMPARE(col->dateTimeAt(1), QDateTime(QDate(2020, 02, 29), QTime(12, 12, 12)));
	QCOMPARE(col->dateTimeAt(0), QDateTime(QDate(2020, 02, 29), QTime(12, 12, 13)));
}

// multiple column
/*
 * check sorting double values with NaN ascending as leading column
 */
void SpreadsheetTest::testSortNumeric1() {
	const QVector<double> xData{0.5, -0.2, GSL_NAN, 2.0, -1.0};
	const QVector<int> yData{1, 2, 3, 4, 5, 6};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(10);
	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->replaceValues(0, xData);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, true);

	//values
	QCOMPARE(col0->valueAt(0), -1.0);
	QCOMPARE(col0->valueAt(1), -0.2);
	QCOMPARE(col0->valueAt(2), 0.5);
	QCOMPARE(col0->valueAt(3), 2.0);
	//QCOMPARE(col0->valueAt(4), GSL_NAN);
	QCOMPARE(col1->integerAt(0), 5);
	QCOMPARE(col1->integerAt(1), 2);
	QCOMPARE(col1->integerAt(2), 1);
	QCOMPARE(col1->integerAt(3), 4);
	QCOMPARE(col1->integerAt(4), 3);
	QCOMPARE(col1->integerAt(5), 6);
}

/*
 * check sorting double values with NaN descending as leading column
 */
void SpreadsheetTest::testSortNumeric2() {
	const QVector<double> xData{0.5, -0.2, GSL_NAN, 2.0, -1.0};
	const QVector<int> yData{1, 2, 3, 4, 5, 6, 7};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(10);
	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->replaceValues(0, xData);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, false);

	//values
	QCOMPARE(col0->valueAt(0), 2.0);
	QCOMPARE(col0->valueAt(1), 0.5);
	QCOMPARE(col0->valueAt(2), -0.2);
	QCOMPARE(col0->valueAt(3), -1.0);
	QCOMPARE(col1->integerAt(0), 4);
	QCOMPARE(col1->integerAt(1), 1);
	QCOMPARE(col1->integerAt(2), 2);
	QCOMPARE(col1->integerAt(3), 5);
	QCOMPARE(col1->integerAt(4), 3);
	QCOMPARE(col1->integerAt(5), 6);
	QCOMPARE(col1->integerAt(6), 7);
}

/*
 * check sorting integer values with empty entries ascending as leading column
 */
void SpreadsheetTest::testSortInteger1() {
	const QVector<int> xData1{4, 5, 2};
	const QVector<int> xData2{3, 6, -1};
	const QVector<int> yData{1, 2, 3, 4, 5, 6, 7};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(7);
	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col0->replaceInteger(0, xData1);
	col0->replaceInteger(4, xData2);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, true);

	//values
	QCOMPARE(col0->integerAt(0), -1);
	QCOMPARE(col0->integerAt(1), 0);
	QCOMPARE(col0->integerAt(2), 2);
	QCOMPARE(col0->integerAt(3), 3);
	QCOMPARE(col0->integerAt(4), 4);
	QCOMPARE(col0->integerAt(5), 5);
	QCOMPARE(col0->integerAt(6), 6);
	QCOMPARE(col1->integerAt(0), 7);
	QCOMPARE(col1->integerAt(1), 4);
	QCOMPARE(col1->integerAt(2), 3);
	QCOMPARE(col1->integerAt(3), 5);
	QCOMPARE(col1->integerAt(4), 1);
	QCOMPARE(col1->integerAt(5), 2);
	QCOMPARE(col1->integerAt(6), 6);
}

/*
 * check sorting integer values with empty entries descending as leading column
 */
void SpreadsheetTest::testSortInteger2() {
	const QVector<int> xData1{4, 5, 2};
	const QVector<int> xData2{3, 6, -1};
	const QVector<int> yData{1, 2, 3, 4, 5, 6, 7};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(7);
	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col0->replaceInteger(0, xData1);
	col0->replaceInteger(4, xData2);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, false);

	//values
	QCOMPARE(col0->integerAt(6), -1);
	QCOMPARE(col0->integerAt(5), 0);
	QCOMPARE(col0->integerAt(4), 2);
	QCOMPARE(col0->integerAt(3), 3);
	QCOMPARE(col0->integerAt(2), 4);
	QCOMPARE(col0->integerAt(1), 5);
	QCOMPARE(col0->integerAt(0), 6);
	QCOMPARE(col1->integerAt(6), 7);
	QCOMPARE(col1->integerAt(5), 4);
	QCOMPARE(col1->integerAt(4), 3);
	QCOMPARE(col1->integerAt(3), 5);
	QCOMPARE(col1->integerAt(2), 1);
	QCOMPARE(col1->integerAt(1), 2);
	QCOMPARE(col1->integerAt(0), 6);
}

/*
 * check sorting big int values with empty entries ascending as leading column
 */
void SpreadsheetTest::testSortBigInt1() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(7);

	QVector<qint64> xData1{40000000000, 50000000000, 20000000000};
	QVector<qint64> xData2{30000000000, 60000000000, -10000000000};
	QVector<qint64> yData{1, 2, 3, 4, 5, 6, 7};

	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->setColumnMode(AbstractColumn::ColumnMode::BigInt);
	col0->replaceBigInt(0, xData1);
	col0->replaceBigInt(4, xData2);
	col1->setColumnMode(AbstractColumn::ColumnMode::BigInt);
	col1->replaceBigInt(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, true);

	//values
	QCOMPARE(col0->bigIntAt(0), -10000000000ll);
	QCOMPARE(col0->bigIntAt(1), 0);
	QCOMPARE(col0->bigIntAt(2), 20000000000ll);
	QCOMPARE(col0->bigIntAt(3), 30000000000ll);
	QCOMPARE(col0->bigIntAt(4), 40000000000ll);
	QCOMPARE(col0->bigIntAt(5), 50000000000ll);
	QCOMPARE(col0->bigIntAt(6), 60000000000ll);
	QCOMPARE(col1->bigIntAt(0), 7);
	QCOMPARE(col1->bigIntAt(1), 4);
	QCOMPARE(col1->bigIntAt(2), 3);
	QCOMPARE(col1->bigIntAt(3), 5);
	QCOMPARE(col1->bigIntAt(4), 1);
	QCOMPARE(col1->bigIntAt(5), 2);
	QCOMPARE(col1->bigIntAt(6), 6);
}

/*
 * check sorting big int values with empty entries descending as leading column
 */
void SpreadsheetTest::testSortBigInt2() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(7);

	QVector<qint64> xData1{40000000000, 50000000000, 20000000000};
	QVector<qint64> xData2{30000000000, 60000000000, -10000000000};
	QVector<qint64> yData{1, 2, 3, 4, 5, 6, 7};

	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->setColumnMode(AbstractColumn::ColumnMode::BigInt);
	col0->replaceBigInt(0, xData1);
	col0->replaceBigInt(4, xData2);
	col1->setColumnMode(AbstractColumn::ColumnMode::BigInt);
	col1->replaceBigInt(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, false);

	//values
	QCOMPARE(col0->bigIntAt(6), -10000000000ll);
	QCOMPARE(col0->bigIntAt(5), 0);
	QCOMPARE(col0->bigIntAt(4), 20000000000ll);
	QCOMPARE(col0->bigIntAt(3), 30000000000ll);
	QCOMPARE(col0->bigIntAt(2), 40000000000ll);
	QCOMPARE(col0->bigIntAt(1), 50000000000ll);
	QCOMPARE(col0->bigIntAt(0), 60000000000ll);
	QCOMPARE(col1->bigIntAt(6), 7);
	QCOMPARE(col1->bigIntAt(5), 4);
	QCOMPARE(col1->bigIntAt(4), 3);
	QCOMPARE(col1->bigIntAt(3), 5);
	QCOMPARE(col1->bigIntAt(2), 1);
	QCOMPARE(col1->bigIntAt(1), 2);
	QCOMPARE(col1->bigIntAt(0), 6);
}

/*
 * check sorting text with empty entries ascending as leading column
 */
void SpreadsheetTest::testSortText1() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(8);

	QVector<QString> xData{"ben", "amy", "eddy", "", "carl", "dan"};
	QVector<int> yData{1, 2, 3, 4, 5, 6, 7};

	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->setColumnMode(AbstractColumn::ColumnMode::Text);
	col0->replaceTexts(0, xData);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, true);

	//values
	QCOMPARE(col0->textAt(0), QLatin1String("amy"));
	QCOMPARE(col0->textAt(1), QLatin1String("ben"));
	QCOMPARE(col0->textAt(2), QLatin1String("carl"));
	QCOMPARE(col0->textAt(3), QLatin1String("dan"));
	QCOMPARE(col0->textAt(4), QLatin1String("eddy"));
	QCOMPARE(col0->textAt(5), QLatin1String(""));
	QCOMPARE(col0->textAt(6), QLatin1String(""));
	QCOMPARE(col1->integerAt(0), 2);
	QCOMPARE(col1->integerAt(1), 1);
	QCOMPARE(col1->integerAt(2), 5);
	QCOMPARE(col1->integerAt(3), 6);
	QCOMPARE(col1->integerAt(4), 3);
	QCOMPARE(col1->integerAt(5), 4);
	QCOMPARE(col1->integerAt(6), 7);
}

/*
 * check sorting text with empty entries descending as leading column
 */
void SpreadsheetTest::testSortText2() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(8);

	QVector<QString> xData{"ben", "amy", "eddy", "", "carl", "dan"};
	QVector<int> yData{1, 2, 3, 4, 5, 6, 7};

	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->setColumnMode(AbstractColumn::ColumnMode::Text);
	col0->replaceTexts(0, xData);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, false);

	//values
	QCOMPARE(col0->textAt(4), QLatin1String("amy"));
	QCOMPARE(col0->textAt(3), QLatin1String("ben"));
	QCOMPARE(col0->textAt(2), QLatin1String("carl"));
	QCOMPARE(col0->textAt(1), QLatin1String("dan"));
	QCOMPARE(col0->textAt(0), QLatin1String("eddy"));
	QCOMPARE(col0->textAt(5), QLatin1String(""));
	QCOMPARE(col0->textAt(6), QLatin1String(""));
	QCOMPARE(col1->integerAt(4), 2);
	QCOMPARE(col1->integerAt(3), 1);
	QCOMPARE(col1->integerAt(2), 5);
	QCOMPARE(col1->integerAt(1), 6);
	QCOMPARE(col1->integerAt(0), 3);
	QCOMPARE(col1->integerAt(5), 4);
	QCOMPARE(col1->integerAt(6), 7);
}

/*
 * check sorting datetimes with invalid entries ascending as leading column
 */
void SpreadsheetTest::testSortDateTime1() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(8);

	QVector<QDateTime> xData{
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 12)),
		QDateTime(QDate(2020, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 29), QTime(12, 12, 12)),	// invalid
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 13)),
		QDateTime(QDate(2020, 02, 29), QTime(11, 12, 12)),
		};
	QVector<int> yData = {1, 2, 3, 4, 5, 6, 7};

	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	col0->replaceDateTimes(0, xData);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, true);

	//values
	QCOMPARE(col0->dateTimeAt(0), QDateTime(QDate(2019, 02, 28), QTime(12, 12, 12)));
	QCOMPARE(col0->dateTimeAt(1), QDateTime(QDate(2020, 02, 28), QTime(12, 12, 12)));
	QCOMPARE(col0->dateTimeAt(2), QDateTime(QDate(2020, 02, 29), QTime(11, 12, 12)));
	QCOMPARE(col0->dateTimeAt(3), QDateTime(QDate(2020, 02, 29), QTime(12, 12, 12)));
	QCOMPARE(col0->dateTimeAt(4), QDateTime(QDate(2020, 02, 29), QTime(12, 12, 13)));
	QCOMPARE(col1->integerAt(0), 3);
	QCOMPARE(col1->integerAt(1), 2);
	QCOMPARE(col1->integerAt(2), 6);
	QCOMPARE(col1->integerAt(3), 1);
	QCOMPARE(col1->integerAt(4), 5);
	QCOMPARE(col1->integerAt(5), 4);
	QCOMPARE(col1->integerAt(6), 7);
}

/*
 * check sorting datetimes with invalid entries descending as leading column
 */
void SpreadsheetTest::testSortDateTime2() {
	const QVector<QDateTime> xData{
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 12)),
		QDateTime(QDate(2020, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 29), QTime(12, 12, 12)),	// invalid
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 13)),
		QDateTime(QDate(2020, 02, 29), QTime(11, 12, 12)),
		};
	const QVector<int> yData = {1, 2, 3, 4, 5, 6, 7};

	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(8);
	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	col0->replaceDateTimes(0, xData);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, false);

	//values
	QCOMPARE(col0->dateTimeAt(4), QDateTime(QDate(2019, 02, 28), QTime(12, 12, 12)));
	QCOMPARE(col0->dateTimeAt(3), QDateTime(QDate(2020, 02, 28), QTime(12, 12, 12)));
	QCOMPARE(col0->dateTimeAt(2), QDateTime(QDate(2020, 02, 29), QTime(11, 12, 12)));
	QCOMPARE(col0->dateTimeAt(1), QDateTime(QDate(2020, 02, 29), QTime(12, 12, 12)));
	QCOMPARE(col0->dateTimeAt(0), QDateTime(QDate(2020, 02, 29), QTime(12, 12, 13)));
	QCOMPARE(col1->integerAt(4), 3);
	QCOMPARE(col1->integerAt(3), 2);
	QCOMPARE(col1->integerAt(2), 6);
	QCOMPARE(col1->integerAt(1), 1);
	QCOMPARE(col1->integerAt(0), 5);
	QCOMPARE(col1->integerAt(5), 4);
	QCOMPARE(col1->integerAt(6), 7);
}

// performance

/*
 * check performance of sorting double values in single column
 */
void SpreadsheetTest::testSortPerformanceNumeric1() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(1);
	sheet.setRowCount(100000);

	QVector<double> xData;
	for (int i = 0; i < sheet.rowCount(); i++)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
		xData << QRandomGenerator::global()->generateDouble();
#else
		xData << (double)(qrand())/RAND_MAX;
#endif

	auto* col = sheet.column(0);
	col->replaceValues(0, xData);

	// sort
	QBENCHMARK {
		sheet.sortColumns(nullptr, {col}, true);
	}
}

/*
 * check performance of sorting double values with two columns
 */
void SpreadsheetTest::testSortPerformanceNumeric2() {
	Spreadsheet sheet("test", false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100000);

	QVector<double> xData;
	QVector<int> yData;
	for (int i = 0; i < sheet.rowCount(); i++) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
		xData << QRandomGenerator::global()->generateDouble();
#else
		xData << (double)(qrand())/RAND_MAX;
#endif
		yData << i + 1;
	}


	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->replaceValues(0, xData);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	QBENCHMARK {
		sheet.sortColumns(col0, {col0, col1}, true);
	}
}

QTEST_MAIN(SpreadsheetTest)
