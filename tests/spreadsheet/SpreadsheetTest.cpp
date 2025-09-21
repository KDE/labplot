/*
	File                 : SpreadsheetTest.cpp
	Project              : LabPlot
	Description          : Tests for the Spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/ColumnStringIO.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/datasources/filters/VectorBLFFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/spreadsheet/SpreadsheetModel.h"
#include "backend/spreadsheet/StatisticsSpreadsheet.h"
#include "frontend/ProjectExplorer.h"
#include "frontend/dockwidgets/SpreadsheetDock.h"
#include "frontend/spreadsheet/FlattenColumnsDialog.h"
#include "frontend/spreadsheet/SearchReplaceWidget.h"
#include "frontend/spreadsheet/SpreadsheetView.h"

#ifdef HAVE_VECTOR_BLF
#include <Vector/BLF.h>
#endif

#include <QClipboard>
#include <QModelIndex>
#include <QRandomGenerator>
#include <QUndoStack>

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
	QLocale::setDefault(QLocale::C); // . as decimal separator
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = QStringLiteral("10.0 100.0\n20.0 200.0");
	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	// spreadsheet size
	QCOMPARE(sheet.columnCount(), 2);
	QCOMPARE(sheet.rowCount(), 100);

	// column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	QCOMPARE(sheet.column(0)->valueAt(0), 10.0);
	QCOMPARE(sheet.column(1)->valueAt(0), 100.0);
	QCOMPARE(sheet.column(0)->valueAt(1), 20.0);
	QCOMPARE(sheet.column(1)->valueAt(1), 200.0);
}

/*!
   insert one column with integer and one column with big integer values into an empty spreadsheet.
   the first column has to be converted to integer column, the second to big integer.
*/
void SpreadsheetTest::testCopyPasteColumnMode01() {
#ifdef __FreeBSD__
	return;
#endif
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = QStringLiteral("10 ") + QString::number(std::numeric_limits<long long>::min()) + QStringLiteral("\n20 ")
		+ QString::number(std::numeric_limits<long long>::max());
	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	// spreadsheet size
	QCOMPARE(sheet.columnCount(), 2);
	QCOMPARE(sheet.rowCount(), 100);

	// column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::BigInt);

	// values
	QCOMPARE(sheet.column(0)->integerAt(0), 10);
	QCOMPARE(sheet.column(1)->bigIntAt(0), std::numeric_limits<long long>::min());
	QCOMPARE(sheet.column(0)->integerAt(1), 20);
	QCOMPARE(sheet.column(1)->bigIntAt(1), std::numeric_limits<long long>::max());
}

/*!
   insert one column with integer values and one column with float numbers into an empty spreadsheet.
   the first column has to be converted to integer column, the second to float.
*/
void SpreadsheetTest::testCopyPasteColumnMode02() {
	QLocale::setDefault(QLocale::C); // . as decimal separator
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = QStringLiteral("10 100.0\n20 200.0");
	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	// spreadsheet size
	QCOMPARE(sheet.columnCount(), 2);
	QCOMPARE(sheet.rowCount(), 100);

	// column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	QCOMPARE(sheet.column(0)->integerAt(0), 10);
	QCOMPARE(sheet.column(1)->valueAt(0), 100.0);
	QCOMPARE(sheet.column(0)->integerAt(1), 20);
	QCOMPARE(sheet.column(1)->valueAt(1), 200.0);
}

/*!
   Properly handle empty values in the tab separated data.
*/
void SpreadsheetTest::testCopyPasteColumnMode03() {
	QLocale::setDefault(QLocale::C); // . as decimal separator
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = QStringLiteral(
		"1000		1000		1000\n"
		"985		985		985\n"
		"970	-7.06562	970		970\n"
		"955	-5.93881	955		955\n"
		"940	-4.97594	940	-4.97594	940");

	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	// spreadsheet size
	QCOMPARE(sheet.columnCount(), 5);
	QCOMPARE(sheet.rowCount(), 100);

	// column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.column(3)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet.column(4)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// values
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

/*!
	automatically detect the proper format for the datetime columns
 */
void SpreadsheetTest::testCopyPasteColumnMode04() {
	QLocale::setDefault(QLocale::C); // . as decimal separator
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = QStringLiteral(
		"2020-09-20 11:21:40:849	7.7\n"
		"2020-09-20 11:21:41:830	4.2");

	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	// spreadsheet size
	QCOMPARE(sheet.columnCount(), 2);
	QCOMPARE(sheet.rowCount(), 100);

	// column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	auto* filter = static_cast<DateTime2StringFilter*>(sheet.column(0)->outputFilter());
	const QString& format = filter->format();

	QCOMPARE(sheet.column(0)->dateTimeAt(0).toString(format), QLatin1String("2020-09-20 11:21:40:849"));
	QCOMPARE(sheet.column(1)->valueAt(0), 7.7);

	QCOMPARE(sheet.column(0)->dateTimeAt(1).toString(format), QLatin1String("2020-09-20 11:21:41:830"));
	QCOMPARE(sheet.column(1)->valueAt(1), 4.2);
}

/*!
	automatically detect the proper format for the datetime columns, time part only
 */
void SpreadsheetTest::testCopyPasteColumnMode05() {
	QLocale::setDefault(QLocale::C); // . as decimal separator
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = QStringLiteral(
		"11:21:40	7.7\n"
		"11:21:41	4.2");

	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	// spreadsheet size
	QCOMPARE(sheet.columnCount(), 2);
	QCOMPARE(sheet.rowCount(), 100);

	// column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	auto* filter = static_cast<DateTime2StringFilter*>(sheet.column(0)->outputFilter());
	const QString& format = filter->format();

	QCOMPARE(sheet.column(0)->dateTimeAt(0).toString(format), QLatin1String("11:21:40"));
	QCOMPARE(sheet.column(1)->valueAt(0), 7.7);

	QCOMPARE(sheet.column(0)->dateTimeAt(1).toString(format), QLatin1String("11:21:41"));
	QCOMPARE(sheet.column(1)->valueAt(1), 4.2);
}

/*!
	automatically detect the proper format for the datetime columns having the format "yyyy-MM-dd hh:mm:ss"
 */
void SpreadsheetTest::testCopyPasteColumnMode06() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = QStringLiteral(
		"2018-03-21 10:00:00 1\n"
		"2018-03-21 10:30:00 2");

	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	// spreadsheet size
	QCOMPARE(sheet.columnCount(), 2);
	QCOMPARE(sheet.rowCount(), 100);

	// column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// values
	auto* filter = static_cast<DateTime2StringFilter*>(sheet.column(0)->outputFilter());
	const QString& format = filter->format();

	QCOMPARE(sheet.column(0)->dateTimeAt(0).toString(format), QLatin1String("2018-03-21 10:00:00"));
	QCOMPARE(sheet.column(1)->integerAt(0), 1);

	QCOMPARE(sheet.column(0)->dateTimeAt(1).toString(format), QLatin1String("2018-03-21 10:30:00"));
	QCOMPARE(sheet.column(1)->integerAt(1), 2);
}

/*!
   insert one column with whitespaces surrounding the actual values.
   the whitespaces have to be removed, one single column is processed only.
*/
void SpreadsheetTest::testCopyPasteColumnMode07() {
	QLocale::setDefault(QLocale::C); // . as decimal separator
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(100);

	const QString str = QStringLiteral("  10  \n  20  \n");
	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	// spreadsheet size and column mode
	QCOMPARE(sheet.columnCount(), 1);
	QCOMPARE(sheet.rowCount(), 100);
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// values
	QCOMPARE(sheet.column(0)->integerAt(0), 10);
	QCOMPARE(sheet.column(0)->integerAt(1), 20);
}

//**********************************************************
//********* Handling of spreadsheet size changes ***********
//**********************************************************
/*!
   insert irregular data, new columns should be added appropriately.
*/
void SpreadsheetTest::testCopyPasteSizeChange00() {
	Project project;
	Spreadsheet* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);
	sheet->setColumnCount(2);
	sheet->setRowCount(100);

	const QString str = QStringLiteral(
		"0\n"
		"10 20\n"
		"11 21 31\n"
		"12 22 32 42\n"
		"13 23\n"
		"14");
	QApplication::clipboard()->setText(str);

	SpreadsheetView view(sheet, false);
	view.pasteIntoSelection();

	// spreadsheet size
	QCOMPARE(sheet->columnCount(), 4);
	QCOMPARE(sheet->rowCount(), 100);

	// column modes
	QCOMPARE(sheet->column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet->column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet->column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet->column(3)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// values
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

	// undo the changes and check the results again
	project.undoStack()->undo();

	// spreadsheet size
	QCOMPARE(sheet->columnCount(), 2);
	QCOMPARE(sheet->rowCount(), 100);

	// column modes
	QCOMPARE(sheet->column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet->column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
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

void SpreadsheetTest::testCopyPasteUtf8() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = QString::fromUtf8("тест1 1\nтест2 2");

	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.pasteIntoSelection();

	// data types
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	// values
	QCOMPARE(sheet.column(0)->textAt(0), QString::fromUtf8("тест1"));
	QCOMPARE(sheet.column(1)->integerAt(0), 1);

	QCOMPARE(sheet.column(0)->textAt(1), QString::fromUtf8("тест2"));
	QCOMPARE(sheet.column(1)->integerAt(1), 2);
}
/*!
   insert the data at the edge of the spreadsheet and paste the data.
   the spreadsheet has to be extended accordingly
*/
void SpreadsheetTest::testCopyPasteSizeChange01() {
	QLocale::setDefault(QLocale::C); // . as decimal separator
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(100);

	const QString str = QStringLiteral(
		"1.1 2.2\n"
		"3.3 4.4");
	QApplication::clipboard()->setText(str);

	SpreadsheetView view(&sheet, false);
	view.goToCell(1, 1); // navigate to the edge of the spreadsheet
	view.pasteIntoSelection();

	// spreadsheet size
	QCOMPARE(sheet.columnCount(), 3);
	QCOMPARE(sheet.rowCount(), 100);

	// column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(sheet.column(2)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
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

// **********************************************************
// *********************** sorting  *************************
// **********************************************************
// single column

/*
 * check sorting single column of double values with NaN ascending
 */
void SpreadsheetTest::testSortSingleNumeric1() {
	const QVector<double> xData{0.5, -0.2, GSL_NAN, 2.0, -1.0};

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->replaceValues(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, true);

	// values
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

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->replaceValues(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, false);

	// values
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

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col->replaceInteger(0, xData1);
	col->replaceInteger(4, xData2);

	// sort
	sheet.sortColumns(nullptr, {col}, true);

	// values
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

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col->replaceInteger(0, xData1);
	col->replaceInteger(4, xData2);

	// sort
	sheet.sortColumns(nullptr, {col}, false);

	// values
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

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::BigInt);
	col->replaceBigInt(0, xData1);
	col->replaceBigInt(4, xData2);

	// sort
	sheet.sortColumns(nullptr, {col}, true);

	// values
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

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(7);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::BigInt);
	col->replaceBigInt(0, xData1);
	col->replaceBigInt(4, xData2);

	// sort
	sheet.sortColumns(nullptr, {col}, false);

	// values
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
	const QVector<QString> xData{QStringLiteral("ben"),
								 QStringLiteral("amy"),
								 QStringLiteral("eddy"),
								 QString(),
								 QStringLiteral("carl"),
								 QStringLiteral("dan")};

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(8);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Text);
	col->replaceTexts(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, true);

	// values
	QCOMPARE(col->textAt(0), QLatin1String("amy"));
	QCOMPARE(col->textAt(1), QLatin1String("ben"));
	QCOMPARE(col->textAt(2), QLatin1String("carl"));
	QCOMPARE(col->textAt(3), QLatin1String("dan"));
	QCOMPARE(col->textAt(4), QLatin1String("eddy"));
	QCOMPARE(col->textAt(5), QString());
	QCOMPARE(col->textAt(6), QString());
}

/*
 * check sorting single column of text with empty entries descending
 */
void SpreadsheetTest::testSortSingleText2() {
	const QVector<QString> xData =
		{QStringLiteral("ben"), QStringLiteral("amy"), QStringLiteral("eddy"), QString(), QStringLiteral("carl"), QStringLiteral("dan")};

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(8);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Text);
	col->replaceTexts(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, false);

	// values
	QCOMPARE(col->textAt(0), QLatin1String("eddy"));
	QCOMPARE(col->textAt(1), QLatin1String("dan"));
	QCOMPARE(col->textAt(2), QLatin1String("carl"));
	QCOMPARE(col->textAt(3), QLatin1String("ben"));
	QCOMPARE(col->textAt(4), QLatin1String("amy"));
	QCOMPARE(col->textAt(5), QString());
	QCOMPARE(col->textAt(6), QString());
}

/*
 * check sorting single column of datetimes with invalid entries ascending
 */
void SpreadsheetTest::testSortSingleDateTime1() {
	const QVector<QDateTime> xData{
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 12)),
		QDateTime(QDate(2020, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 29), QTime(12, 12, 12)), // invalid
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 13)),
		QDateTime(QDate(2020, 02, 29), QTime(11, 12, 12)),
	};

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(8);
	auto* col{sheet.column(0)};
	col->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	col->replaceDateTimes(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, true);

	// values
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
		QDateTime(QDate(2019, 02, 29), QTime(12, 12, 12)), // invalid
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 13)),
		QDateTime(QDate(2020, 02, 29), QTime(11, 12, 12)),
	};

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(8);
	auto* col = sheet.column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	col->replaceDateTimes(0, xData);

	// sort
	sheet.sortColumns(nullptr, {col}, false);

	// values
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

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(10);
	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->replaceValues(0, xData);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, true);

	// values
	QCOMPARE(col0->valueAt(0), -1.0);
	QCOMPARE(col0->valueAt(1), -0.2);
	QCOMPARE(col0->valueAt(2), 0.5);
	QCOMPARE(col0->valueAt(3), 2.0);
	// QCOMPARE(col0->valueAt(4), GSL_NAN);
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

	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(10);
	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->replaceValues(0, xData);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, false);

	// values
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

	Spreadsheet sheet(QStringLiteral("test"), false);
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

	// values
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

	Spreadsheet sheet(QStringLiteral("test"), false);
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

	// values
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
	Spreadsheet sheet(QStringLiteral("test"), false);
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

	// values
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
	Spreadsheet sheet(QStringLiteral("test"), false);
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

	// values
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
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(8);

	QVector<QString> xData{QStringLiteral("ben"), QStringLiteral("amy"), QStringLiteral("eddy"), QString(), QStringLiteral("carl"), QStringLiteral("dan")};
	QVector<int> yData{1, 2, 3, 4, 5, 6, 7};

	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->setColumnMode(AbstractColumn::ColumnMode::Text);
	col0->replaceTexts(0, xData);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, true);

	// values
	QCOMPARE(col0->textAt(0), QLatin1String("amy"));
	QCOMPARE(col0->textAt(1), QLatin1String("ben"));
	QCOMPARE(col0->textAt(2), QLatin1String("carl"));
	QCOMPARE(col0->textAt(3), QLatin1String("dan"));
	QCOMPARE(col0->textAt(4), QLatin1String("eddy"));
	QCOMPARE(col0->textAt(5), QString());
	QCOMPARE(col0->textAt(6), QString());
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
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(8);

	QVector<QString> xData{QStringLiteral("ben"), QStringLiteral("amy"), QStringLiteral("eddy"), QString(), QStringLiteral("carl"), QStringLiteral("dan")};
	QVector<int> yData{1, 2, 3, 4, 5, 6, 7};

	auto* col0{sheet.column(0)};
	auto* col1{sheet.column(1)};
	col0->setColumnMode(AbstractColumn::ColumnMode::Text);
	col0->replaceTexts(0, xData);
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->replaceInteger(0, yData);

	// sort
	sheet.sortColumns(col0, {col0, col1}, false);

	// values
	QCOMPARE(col0->textAt(4), QLatin1String("amy"));
	QCOMPARE(col0->textAt(3), QLatin1String("ben"));
	QCOMPARE(col0->textAt(2), QLatin1String("carl"));
	QCOMPARE(col0->textAt(1), QLatin1String("dan"));
	QCOMPARE(col0->textAt(0), QLatin1String("eddy"));
	QCOMPARE(col0->textAt(5), QString());
	QCOMPARE(col0->textAt(6), QString());
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
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(8);

	QVector<QDateTime> xData{
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 12)),
		QDateTime(QDate(2020, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 28), QTime(12, 12, 12)),
		QDateTime(QDate(2019, 02, 29), QTime(12, 12, 12)), // invalid
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

	// values
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
		QDateTime(QDate(2019, 02, 29), QTime(12, 12, 12)), // invalid
		QDateTime(QDate(2020, 02, 29), QTime(12, 12, 13)),
		QDateTime(QDate(2020, 02, 29), QTime(11, 12, 12)),
	};
	const QVector<int> yData = {1, 2, 3, 4, 5, 6, 7};

	Spreadsheet sheet(QStringLiteral("test"), false);
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

	// values
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
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(10000);

	QVector<double> xData;
	WARN("CREATE DATA")
	for (int i = 0; i < sheet.rowCount(); i++)
		xData << QRandomGenerator::global()->generateDouble();

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
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(10000);

	QVector<double> xData;
	QVector<int> yData;
	WARN("CREATE DATA")
	for (int i = 0; i < sheet.rowCount(); i++) {
		xData << QRandomGenerator::global()->generateDouble();
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

// **********************************************************
// ********************* drop/mask  *************************
// **********************************************************
void SpreadsheetTest::testRemoveRowsWithMissingValues() {
	// prepare the spreadsheet
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(5);

	auto* col0{sheet.column(0)};
	col0->setColumnMode(AbstractColumn::ColumnMode::Double);
	col0->setValueAt(0, 0.);
	// missing value for row = 1
	col0->setValueAt(2, 2.);
	col0->setValueAt(3, 3.);

	auto* col1{sheet.column(1)};
	col1->setColumnMode(AbstractColumn::ColumnMode::Double);
	col1->setValueAt(0, 0.);
	col1->setValueAt(1, 1.);
	// missing value for row = 2
	col1->setValueAt(3, 3.);

	// remove rows with empty values and check the results
	sheet.removeEmptyRows();
	QCOMPARE(sheet.rowCount(), 2);
	QCOMPARE(col0->valueAt(0), 0.);
	QCOMPARE(col0->valueAt(1), 3.);
	QCOMPARE(col1->valueAt(0), 0.);
	QCOMPARE(col1->valueAt(1), 3.);
}

void SpreadsheetTest::testMaskRowsWithMissingValues() {
	// prepare the spreadsheet
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(2);
	sheet.setRowCount(5);

	auto* col0{sheet.column(0)};
	col0->setColumnMode(AbstractColumn::ColumnMode::Double);
	col0->setValueAt(0, 0.);
	// missing value for row = 1
	col0->setValueAt(2, 2.);
	col0->setValueAt(3, 3.);

	auto* col1{sheet.column(1)};
	col1->setColumnMode(AbstractColumn::ColumnMode::Double);
	col1->setValueAt(0, 0.);
	col1->setValueAt(1, 1.);
	// missing value for row = 2
	col1->setValueAt(3, 3.);

	// mask rows with empty values and check the results
	sheet.maskEmptyRows();
	QCOMPARE(sheet.rowCount(), 5);
	QCOMPARE(col0->isMasked(0), false);
	QCOMPARE(col0->isMasked(1), true);
	QCOMPARE(col0->isMasked(2), true);
	QCOMPARE(col0->isMasked(3), false);
	QCOMPARE(col0->isMasked(4), true);
	QCOMPARE(col1->isMasked(0), false);
	QCOMPARE(col1->isMasked(1), true);
	QCOMPARE(col1->isMasked(2), true);
	QCOMPARE(col1->isMasked(3), false);
	QCOMPARE(col1->isMasked(4), true);
}

// **********************************************************
// ********************* flattening  ************************
// **********************************************************
void SpreadsheetTest::testFlatten00() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);
	sheet->setColumnCount(5);
	sheet->setRowCount(4);

	// "Year"
	auto* col1 = sheet->column(0);
	col1->setName(QStringLiteral("Year"));
	col1->setColumnMode(AbstractColumn::ColumnMode::Text);
	col1->setTextAt(0, QStringLiteral("2021"));
	col1->setTextAt(1, QStringLiteral("2022"));
	col1->setTextAt(2, QStringLiteral("2021"));
	col1->setTextAt(3, QStringLiteral("2022"));

	// "Country"
	auto* col2 = sheet->column(1);
	col2->setName(QStringLiteral("Country"));
	col2->setColumnMode(AbstractColumn::ColumnMode::Text);
	col2->setTextAt(0, QStringLiteral("Germany"));
	col2->setTextAt(1, QStringLiteral("Germany"));
	col2->setTextAt(2, QStringLiteral("Poland"));
	col2->setTextAt(3, QStringLiteral("Poland"));

	// "Sales for Product 1"
	auto* col3 = sheet->column(2);
	col3->setName(QStringLiteral("Product 1"));
	col3->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col3->setIntegerAt(0, 1);
	col3->setIntegerAt(1, 10);
	col3->setIntegerAt(2, 4);
	col3->setIntegerAt(3, 40);

	// "Sales for Product 2"
	auto* col4 = sheet->column(3);
	col4->setName(QStringLiteral("Product 2"));
	col4->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col4->setIntegerAt(0, 2);
	col4->setIntegerAt(1, 20);
	col4->setIntegerAt(2, 5);
	col4->setIntegerAt(3, 50);

	// "Sales for Product 3"
	auto* col5 = sheet->column(4);
	col5->setName(QStringLiteral("Product 3"));
	col5->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col5->setIntegerAt(0, 3);
	col5->setIntegerAt(1, 30);
	col5->setIntegerAt(2, 6);
	col5->setIntegerAt(3, 60);

	// flatten the product columns relatively to year and country
	FlattenColumnsDialog dlg(sheet);
	QVector<Column*> referenceColumns;
	referenceColumns << col1;
	referenceColumns << col2;

	QVector<Column*> valueColumns;
	valueColumns << col3;
	valueColumns << col4;
	valueColumns << col5;

	dlg.flatten(sheet, valueColumns, referenceColumns);

	// checks
	// make sure a new target spreadsheet with the flattened data was created
	const auto& sheets = project.children<Spreadsheet>();
	QCOMPARE(sheets.count(), 2);
	auto* targetSheet = sheets.at(1);
	QCOMPARE(targetSheet->columnCount(), 4); // two reference columns, column "Category" and column "Value"
	QCOMPARE(targetSheet->rowCount(), 12);

	// check values
	col1 = targetSheet->column(0);
	QCOMPARE(col1->textAt(0), QStringLiteral("2021"));
	QCOMPARE(col1->textAt(1), QStringLiteral("2021"));
	QCOMPARE(col1->textAt(2), QStringLiteral("2021"));
	QCOMPARE(col1->textAt(3), QStringLiteral("2022"));
	QCOMPARE(col1->textAt(4), QStringLiteral("2022"));
	QCOMPARE(col1->textAt(5), QStringLiteral("2022"));
	QCOMPARE(col1->textAt(6), QStringLiteral("2021"));
	QCOMPARE(col1->textAt(7), QStringLiteral("2021"));
	QCOMPARE(col1->textAt(8), QStringLiteral("2021"));
	QCOMPARE(col1->textAt(9), QStringLiteral("2022"));
	QCOMPARE(col1->textAt(10), QStringLiteral("2022"));
	QCOMPARE(col1->textAt(11), QStringLiteral("2022"));

	col4 = targetSheet->column(3);
	QCOMPARE(col4->integerAt(0), 1);
	QCOMPARE(col4->integerAt(1), 2);
	QCOMPARE(col4->integerAt(2), 3);
	QCOMPARE(col4->integerAt(3), 10);
	QCOMPARE(col4->integerAt(4), 20);
	QCOMPARE(col4->integerAt(5), 30);
	QCOMPARE(col4->integerAt(6), 4);
	QCOMPARE(col4->integerAt(7), 5);
	QCOMPARE(col4->integerAt(8), 6);
	QCOMPARE(col4->integerAt(9), 40);
	QCOMPARE(col4->integerAt(10), 50);
	QCOMPARE(col4->integerAt(11), 60);
}

// test with a missing value in one of the reference columns
void SpreadsheetTest::testFlatten01() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);
	sheet->setColumnCount(5);
	sheet->setRowCount(2);

	// "Year"
	auto* col1 = sheet->column(0);
	col1->setName(QStringLiteral("Year"));
	col1->setColumnMode(AbstractColumn::ColumnMode::Text);
	col1->setTextAt(0, QStringLiteral("2021"));
	col1->setTextAt(1, QStringLiteral("2022"));

	// "Country"
	auto* col2 = sheet->column(1);
	col2->setName(QStringLiteral("Country"));
	col2->setColumnMode(AbstractColumn::ColumnMode::Text);
	col2->setTextAt(0, QStringLiteral("Germany"));
	// missing value in the second row

	// "Sales for Product 1"
	auto* col3 = sheet->column(2);
	col3->setName(QStringLiteral("Product 1"));
	col3->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col3->setIntegerAt(0, 1);
	col3->setIntegerAt(1, 10);

	// "Sales for Product 2"
	auto* col4 = sheet->column(3);
	col4->setName(QStringLiteral("Product 2"));
	col4->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col4->setIntegerAt(0, 2);
	col4->setIntegerAt(1, 20);

	// "Sales for Product 3"
	auto* col5 = sheet->column(4);
	col5->setName(QStringLiteral("Product 3"));
	col5->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col5->setIntegerAt(0, 3);
	col5->setIntegerAt(1, 30);

	// flatten the product columns relatively to year and country
	FlattenColumnsDialog dlg(sheet);
	QVector<Column*> referenceColumns;
	referenceColumns << col1;
	referenceColumns << col2;

	QVector<Column*> valueColumns;
	valueColumns << col3;
	valueColumns << col4;
	valueColumns << col5;

	dlg.flatten(sheet, valueColumns, referenceColumns);

	// checks
	// make sure a new target spreadsheet with the flattened data was created
	const auto& sheets = project.children<Spreadsheet>();
	QCOMPARE(sheets.count(), 2);
	auto* targetSheet = sheets.at(1);
	QCOMPARE(targetSheet->columnCount(), 4); // two reference columns, column "Category" and column "Value"
	QCOMPARE(targetSheet->rowCount(), 6);

	// check values
	col1 = targetSheet->column(0);
	QCOMPARE(col1->textAt(0), QStringLiteral("2021"));
	QCOMPARE(col1->textAt(1), QStringLiteral("2021"));
	QCOMPARE(col1->textAt(2), QStringLiteral("2021"));
	QCOMPARE(col1->textAt(3), QStringLiteral("2022"));
	QCOMPARE(col1->textAt(4), QStringLiteral("2022"));
	QCOMPARE(col1->textAt(5), QStringLiteral("2022"));

	col1 = targetSheet->column(1);
	QCOMPARE(col1->textAt(0), QStringLiteral("Germany"));
	QCOMPARE(col1->textAt(1), QStringLiteral("Germany"));
	QCOMPARE(col1->textAt(2), QStringLiteral("Germany"));
	QCOMPARE(col1->textAt(3), QString());
	QCOMPARE(col1->textAt(4), QString());
	QCOMPARE(col1->textAt(5), QString());

	col4 = targetSheet->column(3);
	QCOMPARE(col4->integerAt(0), 1);
	QCOMPARE(col4->integerAt(1), 2);
	QCOMPARE(col4->integerAt(2), 3);
	QCOMPARE(col4->integerAt(3), 10);
	QCOMPARE(col4->integerAt(4), 20);
	QCOMPARE(col4->integerAt(5), 30);
}

// test with missing values in the reference columns - no result should be produced for these rows
void SpreadsheetTest::testFlatten02() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);
	sheet->setColumnCount(5);
	sheet->setRowCount(2);

	// "Year"
	auto* col1 = sheet->column(0);
	col1->setName(QStringLiteral("Year"));
	col1->setColumnMode(AbstractColumn::ColumnMode::Text);
	col1->setTextAt(0, QStringLiteral("2021"));
	// missing value in the second row

	// "Country"
	auto* col2 = sheet->column(1);
	col2->setName(QStringLiteral("Country"));
	col2->setColumnMode(AbstractColumn::ColumnMode::Text);
	col2->setTextAt(0, QStringLiteral("Germany"));
	// missing value in the second rows

	// "Sales for Product 1"
	auto* col3 = sheet->column(2);
	col3->setName(QStringLiteral("Product 1"));
	col3->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col3->setIntegerAt(0, 1);
	col3->setIntegerAt(1, 10);

	// "Sales for Product 2"
	auto* col4 = sheet->column(3);
	col4->setName(QStringLiteral("Product 2"));
	col4->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col4->setIntegerAt(0, 2);
	col4->setIntegerAt(1, 20);

	// "Sales for Product 3"
	auto* col5 = sheet->column(4);
	col5->setName(QStringLiteral("Product 3"));
	col5->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col5->setIntegerAt(0, 3);
	col5->setIntegerAt(1, 30);

	// flatten the product columns relatively to year and country
	FlattenColumnsDialog dlg(sheet);
	QVector<Column*> referenceColumns;
	referenceColumns << col1;
	referenceColumns << col2;

	QVector<Column*> valueColumns;
	valueColumns << col3;
	valueColumns << col4;
	valueColumns << col5;

	dlg.flatten(sheet, valueColumns, referenceColumns);

	// checks
	// make sure a new target spreadsheet with the flattened data was created
	const auto& sheets = project.children<Spreadsheet>();
	QCOMPARE(sheets.count(), 2);
	auto* targetSheet = sheets.at(1);
	QCOMPARE(targetSheet->columnCount(), 4); // two reference columns, column "Category" and column "Value"
	QCOMPARE(targetSheet->rowCount(), 3);

	// check values
	col1 = targetSheet->column(0);
	QCOMPARE(col1->textAt(0), QStringLiteral("2021"));
	QCOMPARE(col1->textAt(1), QStringLiteral("2021"));
	QCOMPARE(col1->textAt(2), QStringLiteral("2021"));

	col1 = targetSheet->column(1);
	QCOMPARE(col1->textAt(0), QStringLiteral("Germany"));
	QCOMPARE(col1->textAt(1), QStringLiteral("Germany"));
	QCOMPARE(col1->textAt(2), QStringLiteral("Germany"));

	col4 = targetSheet->column(3);
	QCOMPARE(col4->integerAt(0), 1);
	QCOMPARE(col4->integerAt(1), 2);
	QCOMPARE(col4->integerAt(2), 3);
}

// test with missing no reference columns
void SpreadsheetTest::testFlatten03() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);
	sheet->setColumnCount(3);
	sheet->setRowCount(2);

	// "Sales for Product 1"
	auto* col1 = sheet->column(0);
	col1->setName(QStringLiteral("Product 1"));
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col1->setIntegerAt(0, 1);
	col1->setIntegerAt(1, 10);

	// "Sales for Product 2"
	auto* col2 = sheet->column(1);
	col2->setName(QStringLiteral("Product 2"));
	col2->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col2->setIntegerAt(0, 2);
	col2->setIntegerAt(1, 20);

	// "Sales for Product 3"
	auto* col3 = sheet->column(2);
	col3->setName(QStringLiteral("Product 3"));
	col3->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col3->setIntegerAt(0, 3);
	col3->setIntegerAt(1, 30);

	// flatten the product columns without any reference columns
	QVector<Column*> valueColumns;
	valueColumns << col1;
	valueColumns << col2;
	valueColumns << col3;

	FlattenColumnsDialog dlg(sheet);
	dlg.flatten(sheet, valueColumns, QVector<Column*>());

	// checks
	// make sure a new target spreadsheet with the flattened data was created
	const auto& sheets = project.children<Spreadsheet>();
	QCOMPARE(sheets.count(), 2);
	auto* targetSheet = sheets.at(1);
	QCOMPARE(targetSheet->columnCount(), 2); // no reference columns, only column "Category" and column "Value"
	QCOMPARE(targetSheet->rowCount(), 6);

	// check values
	col1 = targetSheet->column(0);
	QCOMPARE(col1->textAt(0), QStringLiteral("Product 1"));
	QCOMPARE(col1->textAt(1), QStringLiteral("Product 2"));
	QCOMPARE(col1->textAt(2), QStringLiteral("Product 3"));
	QCOMPARE(col1->textAt(3), QStringLiteral("Product 1"));
	QCOMPARE(col1->textAt(4), QStringLiteral("Product 2"));
	QCOMPARE(col1->textAt(5), QStringLiteral("Product 3"));

	col2 = targetSheet->column(1);
	QCOMPARE(col2->integerAt(0), 1);
	QCOMPARE(col2->integerAt(1), 2);
	QCOMPARE(col2->integerAt(2), 3);
	QCOMPARE(col2->integerAt(3), 10);
	QCOMPARE(col2->integerAt(4), 20);
	QCOMPARE(col2->integerAt(5), 30);
}

// **********************************************************
// ******************** search&replace  *********************
// **********************************************************
Spreadsheet* SpreadsheetTest::createSearchReplaceSpreadsheet() {
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);

	sheet->setColumnCount(4);
	sheet->setRowCount(4);

	// first text column
	auto* col1 = sheet->column(0);
	col1->setName(QStringLiteral("text"));
	col1->setColumnMode(AbstractColumn::ColumnMode::Text);
	col1->setTextAt(0, QStringLiteral("A"));
	col1->setTextAt(1, QStringLiteral("B"));
	col1->setTextAt(2, QStringLiteral("A"));
	col1->setTextAt(3, QStringLiteral("C"));

	// first numeric (integer) column
	auto* col2 = sheet->column(1);
	col2->setName(QStringLiteral("integer"));
	col2->setColumnMode(AbstractColumn::ColumnMode::Integer);
	col2->setIntegerAt(0, 1);
	col2->setIntegerAt(1, 2);
	col2->setIntegerAt(2, 4);
	col2->setIntegerAt(3, 2);

	// second text column
	auto* col3 = sheet->column(2);
	col3->setName(QStringLiteral("text2"));
	col3->setColumnMode(AbstractColumn::ColumnMode::Text);
	col3->setTextAt(0, QStringLiteral("B1"));
	col3->setTextAt(1, QStringLiteral("A"));
	col3->setTextAt(2, QStringLiteral("C2"));
	col3->setTextAt(3, QStringLiteral("A2"));

	// second numeric (double) column
	auto* col4 = sheet->column(3);
	col4->setName(QStringLiteral("double"));
	col4->setColumnMode(AbstractColumn::ColumnMode::Double);
	col4->setValueAt(0, 4);
	col4->setValueAt(1, 3);
	col4->setValueAt(2, 2);
	col4->setValueAt(3, 1);

	return sheet;
}

/*!
 * simple search ignoring data types, column-major order
 */
void SpreadsheetTest::testSearchSimple00() {
	Project project;
	auto* sheet = createSearchReplaceSpreadsheet();
	project.addChild(sheet);

	// navigate to the (0,0) cell having the text value "A"
	auto* view = static_cast<SpreadsheetView*>(sheet->view());
	view->goToCell(0, 0);

	// check the initial selection
	auto indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	auto curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 0);
	QCOMPARE(curIndex.column(), 0);

	// initialize the search&replace widget
	auto* searchReplaceWidget = new SearchReplaceWidget(sheet, view);
	searchReplaceWidget->setReplaceEnabled(false);
	const auto& firstIndex = indexes.constFirst();
	const auto* column = sheet->column(firstIndex.column());
	const int row = firstIndex.row();
	searchReplaceWidget->setInitialPattern(column->columnMode(), column->asStringColumn()->textAt(row));

	// checks: the initial cell text is "A", we navigate with 'next'
	// and then back with 'prev' in the column-major order looking for "A"

	// next
	searchReplaceWidget->findNextSimple(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 0);

	// next
	searchReplaceWidget->findNextSimple(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 2);

	// next
	searchReplaceWidget->findNextSimple(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 3);
	QCOMPARE(curIndex.column(), 2);

	// next, last matching cell reached
	searchReplaceWidget->findNextSimple(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 3);
	QCOMPARE(curIndex.column(), 2);

	// previous
	searchReplaceWidget->findPreviousSimple(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 2);

	// previous
	searchReplaceWidget->findPreviousSimple(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 0);

	// previous
	searchReplaceWidget->findPreviousSimple(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 0);
	QCOMPARE(curIndex.column(), 0);

	// previous, last matching cell reached
	searchReplaceWidget->findPreviousSimple(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 0);
	QCOMPARE(curIndex.column(), 0);
}

/*!
 * extended search for Text, column-major order
 */
void SpreadsheetTest::testSearchExtended00() {
#ifdef __FreeBSD__
	// ASSERT failure in QBoxLayout::insert: "index out of range"
	return;
#endif
	Project project;
	auto* sheet = createSearchReplaceSpreadsheet();
	project.addChild(sheet);

	// navigate to the (0,0) cell having the text value "A"
	auto* view = static_cast<SpreadsheetView*>(sheet->view());
	view->goToCell(0, 0);

	// check the initial selection
	auto indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	auto curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 0);
	QCOMPARE(curIndex.column(), 0);

	// initialize the search&replace widget
	auto* searchReplaceWidget = new SearchReplaceWidget(sheet, view);
	searchReplaceWidget->setReplaceEnabled(true);
	searchReplaceWidget->setDataType(SearchReplaceWidget::DataType::Text);
	searchReplaceWidget->setOrder(SearchReplaceWidget::Order::ColumnMajor);
	const auto& firstIndex = indexes.constFirst();
	const auto* column = sheet->column(firstIndex.column());
	const int row = firstIndex.row();
	searchReplaceWidget->setInitialPattern(column->columnMode(), column->asStringColumn()->textAt(row));

	// checks: the initial cell text is "A", we navigate with 'next'
	// and then back with 'prev' in the column-major order looking for "A"

	// next
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 0);

	// next
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 2);

	// next, last matching cell reached
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 2);

	// previous
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 0);

	// previous
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 0);
	QCOMPARE(curIndex.column(), 0);

	// previous, last matching cell reached
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 0);
	QCOMPARE(curIndex.column(), 0);
}

/*!
 * extended search for Text, row-major order
 */
void SpreadsheetTest::testSearchExtended01() {
#ifdef __FreeBSD__
	// ASSERT failure in QBoxLayout::insert: "index out of range"
	return;
#endif
	Project project;
	auto* sheet = createSearchReplaceSpreadsheet();
	project.addChild(sheet);

	// navigate to the (0,0) cell having the text value "A"
	auto* view = static_cast<SpreadsheetView*>(sheet->view());
	view->goToCell(0, 0);

	// check the initial selection
	auto indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	auto curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 0);
	QCOMPARE(curIndex.column(), 0);

	// initialize the search&replace widget
	auto* searchReplaceWidget = new SearchReplaceWidget(sheet, view);
	searchReplaceWidget->setReplaceEnabled(true);
	searchReplaceWidget->setDataType(SearchReplaceWidget::DataType::Text);
	searchReplaceWidget->setOrder(SearchReplaceWidget::Order::RowMajor);
	const auto& firstIndex = indexes.constFirst();
	const auto* column = sheet->column(firstIndex.column());
	const int row = firstIndex.row();
	searchReplaceWidget->setInitialPattern(column->columnMode(), column->asStringColumn()->textAt(row));

	// checks: the initial cell text is "A", we navigate with 'next'
	// and then back with 'prev' in the row-major order looking for "A"

	// next
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 2);

	// next
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 0);

	// next, last matching cell reached
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 0);

	// previous
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 2);

	// previous
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 0);
	QCOMPARE(curIndex.column(), 0);

	// previous, last matching cell reached
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 0);
	QCOMPARE(curIndex.column(), 0);
}

/*!
 * search for Numeric, column-major order
 */
void SpreadsheetTest::testSearchExtended02() {
#ifdef __FreeBSD__
	// ASSERT failure in QBoxLayout::insert: "index out of range"
	return;
#endif
	Project project;
	auto* sheet = createSearchReplaceSpreadsheet();
	project.addChild(sheet);

	// navigate to the (1,1) cell having the numeric value "2"
	auto* view = static_cast<SpreadsheetView*>(sheet->view());
	view->goToCell(1, 1);

	// check the initial selection
	auto indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	auto curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 1);

	// initialize the search&replace widget
	auto* searchReplaceWidget = new SearchReplaceWidget(sheet, view);
	searchReplaceWidget->setReplaceEnabled(true);
	searchReplaceWidget->setDataType(SearchReplaceWidget::DataType::Numeric);
	searchReplaceWidget->setOrder(SearchReplaceWidget::Order::ColumnMajor);
	const auto& firstIndex = indexes.constFirst();
	const auto* column = sheet->column(firstIndex.column());
	const int row = firstIndex.row();
	searchReplaceWidget->setInitialPattern(column->columnMode(), column->asStringColumn()->textAt(row));

	// checks: the initial cell text is "2", we navigate with 'next'
	// and then back with 'prev' in the column-major order looking for "2"

	// next
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 3);
	QCOMPARE(curIndex.column(), 1);

	// next
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 3);

	// next, last matching cell reached
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 3);

	// previous
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 3);
	QCOMPARE(curIndex.column(), 1);

	// previous
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 1);

	// previous, last matching cell reached
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 1);
}

/*!
 * search for Numeric, row major
 */
void SpreadsheetTest::testSearchExtended03() {
#ifdef __FreeBSD__
	// ASSERT failure in QBoxLayout::insert: "index out of range"
	return;
#endif
	Project project;
	auto* sheet = createSearchReplaceSpreadsheet();
	project.addChild(sheet);

	// navigate to the (1,1) cell having the numeric value "2"
	auto* view = static_cast<SpreadsheetView*>(sheet->view());
	view->goToCell(1, 1);

	// check the initial selection
	auto indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	auto curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 1);

	// initialize the search&replace widget
	auto* searchReplaceWidget = new SearchReplaceWidget(sheet, view);
	searchReplaceWidget->setReplaceEnabled(true);
	searchReplaceWidget->setDataType(SearchReplaceWidget::DataType::Numeric);
	searchReplaceWidget->setOrder(SearchReplaceWidget::Order::RowMajor);
	const auto& firstIndex = indexes.constFirst();
	const auto* column = sheet->column(firstIndex.column());
	const int row = firstIndex.row();
	searchReplaceWidget->setInitialPattern(column->columnMode(), column->asStringColumn()->textAt(row));

	// checks: the initial cell text is "2", we navigate with 'next'
	// and then back with 'prev' in the column-major order looking for "2"

	// next
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 3);

	// next
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 3);
	QCOMPARE(curIndex.column(), 1);

	// next, last matching cell reached
	searchReplaceWidget->findNext(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 3);
	QCOMPARE(curIndex.column(), 1);

	// previous
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 3);

	// previous
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 1);

	// previous, last matching cell reached
	searchReplaceWidget->findPrevious(true);
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 1);
}

void SpreadsheetTest::testSearchFindAll() {
#ifdef __FreeBSD__
	// ASSERT failure in QBoxLayout::insert: "index out of range"
	return;
#endif
	Project project;
	auto* sheet = createSearchReplaceSpreadsheet();
	project.addChild(sheet);

	// initialize the search&replace widget
	auto* searchReplaceWidget = new SearchReplaceWidget(sheet, nullptr);
	searchReplaceWidget->setReplaceEnabled(true);
	searchReplaceWidget->setDataType(SearchReplaceWidget::DataType::Text);
	searchReplaceWidget->setOrder(SearchReplaceWidget::Order::ColumnMajor);
	searchReplaceWidget->setTextOperator(SearchReplaceWidget::OperatorText::StartsWith);

	// search for cells starting with "B" and highlight them ("find")
	searchReplaceWidget->setInitialPattern(AbstractColumn::ColumnMode::Text, QLatin1String("B"));
	searchReplaceWidget->findAll();

	// checks
	auto* view = static_cast<SpreadsheetView*>(sheet->view());
	auto indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.size(), 2);
	QCOMPARE(indexes.at(0).row(), 1);
	QCOMPARE(indexes.at(0).column(), 0);
	QCOMPARE(indexes.at(1).row(), 0);
	QCOMPARE(indexes.at(1).column(), 2);
}

/*!
 * replace a numeric value in int and double columns via "replace next"
 */
void SpreadsheetTest::testSearchReplaceNumeric() {
#ifdef __FreeBSD__
	// ASSERT failure in QBoxLayout::insert: "index out of range"
	return;
#endif
	Project project;
	auto* sheet = createSearchReplaceSpreadsheet();
	project.addChild(sheet);

	// navigate to the (1,1) cell having the numeric value 2
	auto* view = static_cast<SpreadsheetView*>(sheet->view());
	view->goToCell(1, 1);

	// check the initial selection
	auto indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	auto curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 1);

	// initialize the search&replace widget
	auto* searchReplaceWidget = new SearchReplaceWidget(sheet, view);
	searchReplaceWidget->setReplaceEnabled(true);
	searchReplaceWidget->setDataType(SearchReplaceWidget::DataType::Numeric);
	searchReplaceWidget->setOrder(SearchReplaceWidget::Order::ColumnMajor);
	searchReplaceWidget->setReplaceText(QLatin1String("5"));
	const auto& firstIndex = indexes.constFirst();
	const auto* column = sheet->column(firstIndex.column());
	const int row = firstIndex.row();
	searchReplaceWidget->setInitialPattern(column->columnMode(), column->asStringColumn()->textAt(row));

	// checks: the initial cell text is (1,1) with the value 2, we replace this value with 5 via replaceNext,
	// and proceed to other cells with further replaceNext calls.
	const auto& columns = sheet->children<Column>();

	// replace next - the value in the currently selected cell should be replaced first
	searchReplaceWidget->replaceNext();
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 1);
	QCOMPARE(columns.at(1)->integerAt(1), 5);

	// replace next
	searchReplaceWidget->replaceNext();
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 3);
	QCOMPARE(curIndex.column(), 1);
	QCOMPARE(columns.at(1)->integerAt(1), 5);

	// replace next
	searchReplaceWidget->replaceNext();
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 3);
	QCOMPARE(columns.at(3)->valueAt(2), 5);
}

/*!
 * replace a text value in text columns via "replace next"
 */
void SpreadsheetTest::testSearchReplaceText() {
#ifdef __FreeBSD__
	// ASSERT failure in QBoxLayout::insert: "index out of range"
	return;
#endif
	Project project;
	auto* sheet = createSearchReplaceSpreadsheet();
	project.addChild(sheet);

	// navigate to the (0,0) cell having the text value "A"
	auto* view = static_cast<SpreadsheetView*>(sheet->view());
	view->goToCell(0, 0);

	// check the initial selection
	auto indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	auto curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 0);
	QCOMPARE(curIndex.column(), 0);

	// initialize the search&replace widget
	auto* searchReplaceWidget = new SearchReplaceWidget(sheet, view);
	searchReplaceWidget->setReplaceEnabled(true);
	searchReplaceWidget->setDataType(SearchReplaceWidget::DataType::Text);
	searchReplaceWidget->setOrder(SearchReplaceWidget::Order::ColumnMajor);
	searchReplaceWidget->setReplaceText(QLatin1String("AAA"));
	const auto& firstIndex = indexes.constFirst();
	const auto* column = sheet->column(firstIndex.column());
	const int row = firstIndex.row();
	searchReplaceWidget->setInitialPattern(column->columnMode(), column->asStringColumn()->textAt(row));

	// checks: the initial cell text is (0,0) with the value "A", we replace this value with "AAA" via replaceNext,
	// and proceed to other cells with further replaceNext calls.
	const auto& columns = sheet->children<Column>();

	// replace next - the value in the currently selected cell should be replaced first
	searchReplaceWidget->replaceNext();
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 0);
	QCOMPARE(curIndex.column(), 0);
	QCOMPARE(columns.at(0)->textAt(0), QLatin1String("AAA"));

	// replace next
	searchReplaceWidget->replaceNext();
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 2);
	QCOMPARE(curIndex.column(), 0);
	QCOMPARE(columns.at(0)->textAt(2), QLatin1String("AAA"));

	// replace next
	searchReplaceWidget->replaceNext();
	indexes = view->selectionModel()->selectedIndexes();
	QCOMPARE(indexes.count(), 1);
	curIndex = indexes.constFirst();
	QCOMPARE(curIndex.row(), 1);
	QCOMPARE(curIndex.column(), 2);
	QCOMPARE(columns.at(2)->textAt(1), QLatin1String("AAA"));
}

void SpreadsheetTest::testSearchReplaceAll() {
#ifdef __FreeBSD__
	// ASSERT failure in QBoxLayout::insert: "index out of range"
	return;
#endif
	Project project;
	auto* sheet = createSearchReplaceSpreadsheet();
	project.addChild(sheet);

	// initialize the search&replace widget
	auto* searchReplaceWidget = new SearchReplaceWidget(sheet, nullptr);
	searchReplaceWidget->setReplaceEnabled(true);
	searchReplaceWidget->setDataType(SearchReplaceWidget::DataType::Text);
	searchReplaceWidget->setOrder(SearchReplaceWidget::Order::ColumnMajor);
	searchReplaceWidget->setTextOperator(SearchReplaceWidget::OperatorText::RegEx);

	// search for "A" or "C" and replace with "test"
	searchReplaceWidget->setInitialPattern(AbstractColumn::ColumnMode::Text, QLatin1String("[A.C]"));
	searchReplaceWidget->setReplaceText(QLatin1String("test"));
	searchReplaceWidget->replaceAll();

	// checks
	const auto& columns = sheet->children<Column>();
	QCOMPARE(columns.at(0)->textAt(0), QLatin1String("test"));
	QCOMPARE(columns.at(0)->textAt(2), QLatin1String("test"));
	QCOMPARE(columns.at(2)->textAt(1), QLatin1String("test"));
	QCOMPARE(columns.at(2)->textAt(2), QLatin1String("test"));
}

// **********************************************************
// ********************** size changes  *********************
// **********************************************************
void SpreadsheetTest::testInsertRows() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	auto* model = new SpreadsheetModel(sheet);
	int rowsAboutToBeInsertedCounter = 0;
	connect(model, &SpreadsheetModel::rowsAboutToBeInserted, [&rowsAboutToBeInsertedCounter]() {
		rowsAboutToBeInsertedCounter++;
	});
	int rowsInsertedCounter = 0;
	connect(model, &SpreadsheetModel::rowsInserted, [&rowsInsertedCounter]() {
		rowsInsertedCounter++;
	});
	int rowsAboutToBeRemovedCounter = 0;
	connect(model, &SpreadsheetModel::rowsAboutToBeRemoved, [&rowsAboutToBeRemovedCounter]() {
		rowsAboutToBeRemovedCounter++;
	});
	int rowsRemovedCounter = 0;
	connect(model, &SpreadsheetModel::rowsRemoved, [&rowsRemovedCounter]() {
		rowsRemovedCounter++;
	});

	QCOMPARE(sheet->rowCount(), 100);
	QCOMPARE(model->rowCount(), 100);
	sheet->setRowCount(101); // No crash shall happen
	QCOMPARE(sheet->rowCount(), 101);
	QCOMPARE(model->rowCount(), 101);

	sheet->undoStack()->undo();
	QCOMPARE(sheet->rowCount(), 100);
	QCOMPARE(model->rowCount(), 100);
	sheet->undoStack()->redo();
	QCOMPARE(sheet->rowCount(), 101);
	QCOMPARE(model->rowCount(), 101);

	QCOMPARE(rowsAboutToBeInsertedCounter, 2); // set and redo()
	QCOMPARE(rowsInsertedCounter, 2); // set and redo()
	QCOMPARE(rowsAboutToBeRemovedCounter, 1); // undo()
	QCOMPARE(rowsRemovedCounter, 1); // undo()
}

void SpreadsheetTest::testRemoveRows() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	auto* model = new SpreadsheetModel(sheet);
	int rowsAboutToBeInsertedCounter = 0;
	connect(model, &SpreadsheetModel::rowsAboutToBeInserted, [&rowsAboutToBeInsertedCounter]() {
		rowsAboutToBeInsertedCounter++;
	});
	int rowsInsertedCounter = 0;
	connect(model, &SpreadsheetModel::rowsInserted, [&rowsInsertedCounter]() {
		rowsInsertedCounter++;
	});
	int rowsAboutToBeRemovedCounter = 0;
	connect(model, &SpreadsheetModel::rowsAboutToBeRemoved, [&rowsAboutToBeRemovedCounter]() {
		rowsAboutToBeRemovedCounter++;
	});
	int rowsRemovedCounter = 0;
	connect(model, &SpreadsheetModel::rowsRemoved, [&rowsRemovedCounter]() {
		rowsRemovedCounter++;
	});

	QCOMPARE(sheet->rowCount(), 100);
	QCOMPARE(model->rowCount(), 100);
	sheet->setRowCount(10); // No crash shall happen
	QCOMPARE(sheet->rowCount(), 10);
	QCOMPARE(model->rowCount(), 10);

	sheet->undoStack()->undo();
	QCOMPARE(sheet->rowCount(), 100);
	QCOMPARE(model->rowCount(), 100);
	sheet->undoStack()->redo();
	QCOMPARE(sheet->rowCount(), 10);
	QCOMPARE(model->rowCount(), 10);

	QCOMPARE(rowsAboutToBeInsertedCounter, 1); // undo
	QCOMPARE(rowsInsertedCounter, 1); // undo
	QCOMPARE(rowsAboutToBeRemovedCounter, 2); // set and redo()
	QCOMPARE(rowsRemovedCounter, 2); // set and redo()
}

void SpreadsheetTest::testInsertRowsBegin() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	auto* model = new SpreadsheetModel(sheet);
	int rowsAboutToBeInsertedCounter = 0;
	connect(model, &SpreadsheetModel::rowsAboutToBeInserted, [&rowsAboutToBeInsertedCounter]() {
		rowsAboutToBeInsertedCounter++;
	});
	int rowsInsertedCounter = 0;
	connect(model, &SpreadsheetModel::rowsInserted, [&rowsInsertedCounter]() {
		rowsInsertedCounter++;
	});
	int rowsAboutToBeRemovedCounter = 0;
	connect(model, &SpreadsheetModel::rowsAboutToBeRemoved, [&rowsAboutToBeRemovedCounter]() {
		rowsAboutToBeRemovedCounter++;
	});
	int rowsRemovedCounter = 0;
	connect(model, &SpreadsheetModel::rowsRemoved, [&rowsRemovedCounter]() {
		rowsRemovedCounter++;
	});

	QCOMPARE(sheet->rowCount(), 100);
	QCOMPARE(model->rowCount(), 100);
	sheet->insertRows(0, 1); // No crash shall happen
	QCOMPARE(sheet->rowCount(), 101);
	QCOMPARE(model->rowCount(), 101);

	sheet->undoStack()->undo();
	QCOMPARE(sheet->rowCount(), 100);
	QCOMPARE(model->rowCount(), 100);
	sheet->undoStack()->redo();
	QCOMPARE(sheet->rowCount(), 101);
	QCOMPARE(model->rowCount(), 101);

	QCOMPARE(rowsAboutToBeInsertedCounter, 2); // set and redo()
	QCOMPARE(rowsInsertedCounter, 2); // set and redo()
	QCOMPARE(rowsAboutToBeRemovedCounter, 1); // undo()
	QCOMPARE(rowsRemovedCounter, 1); // undo()
}
void SpreadsheetTest::testRemoveRowsBegin() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	auto* model = new SpreadsheetModel(sheet);
	int rowsAboutToBeInsertedCounter = 0;
	connect(model, &SpreadsheetModel::rowsAboutToBeInserted, [&rowsAboutToBeInsertedCounter]() {
		rowsAboutToBeInsertedCounter++;
	});
	int rowsInsertedCounter = 0;
	connect(model, &SpreadsheetModel::rowsInserted, [&rowsInsertedCounter]() {
		rowsInsertedCounter++;
	});
	int rowsAboutToBeRemovedCounter = 0;
	connect(model, &SpreadsheetModel::rowsAboutToBeRemoved, [&rowsAboutToBeRemovedCounter]() {
		rowsAboutToBeRemovedCounter++;
	});
	int rowsRemovedCounter = 0;
	connect(model, &SpreadsheetModel::rowsRemoved, [&rowsRemovedCounter]() {
		rowsRemovedCounter++;
	});

	QCOMPARE(sheet->rowCount(), 100);
	QCOMPARE(model->rowCount(), 100);
	sheet->removeRows(0, 1);
	QCOMPARE(sheet->rowCount(), 99);
	QCOMPARE(model->rowCount(), 99);

	sheet->undoStack()->undo();
	QCOMPARE(sheet->rowCount(), 100);
	QCOMPARE(model->rowCount(), 100);
	sheet->undoStack()->redo();
	QCOMPARE(sheet->rowCount(), 99);
	QCOMPARE(model->rowCount(), 99);

	QCOMPARE(rowsAboutToBeInsertedCounter, 1); // undo
	QCOMPARE(rowsInsertedCounter, 1); // undo
	QCOMPARE(rowsAboutToBeRemovedCounter, 2); // set and redo()
	QCOMPARE(rowsRemovedCounter, 2); // set and redo()
}

void SpreadsheetTest::testInsertColumns() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	auto* model = new SpreadsheetModel(sheet);

	// initial size
	QCOMPARE(sheet->columnCount(), 2);
	QCOMPARE(model->columnCount(), 2);

	// count the number of signals emitted in the underlying data model (implemented in QAbstractItemModel)
	int columnsAboutToBeInsertedCounter = 0;
	connect(model, &SpreadsheetModel::columnsAboutToBeInserted, [&columnsAboutToBeInsertedCounter]() {
		columnsAboutToBeInsertedCounter++;
	});
	int columnsInsertedCounter = 0;
	connect(model, &SpreadsheetModel::columnsInserted, [&columnsInsertedCounter]() {
		columnsInsertedCounter++;
	});
	int columnsAboutToBeRemovedCounter = 0;
	connect(model, &SpreadsheetModel::columnsAboutToBeRemoved, [&columnsAboutToBeRemovedCounter]() {
		columnsAboutToBeRemovedCounter++;
	});
	int columnsRemovedCounter = 0;
	connect(model, &SpreadsheetModel::columnsRemoved, [&columnsRemovedCounter]() {
		columnsRemovedCounter++;
	});

	// set the total number of columns to 5, 3 more columns added, and verify
	sheet->setColumnCount(5);
	QCOMPARE(sheet->columnCount(), 5);
	QCOMPARE(model->columnCount(), 5);

	// undo and verify, should be 2 again
	sheet->undoStack()->undo();
	QCOMPARE(sheet->columnCount(), 2);
	QCOMPARE(model->columnCount(), 2);

	// redo and verify, should be 5 again
	sheet->undoStack()->redo();
	QCOMPARE(sheet->columnCount(), 5);
	QCOMPARE(model->columnCount(), 5);

	// check the number of signals emitted
	QCOMPARE(columnsAboutToBeInsertedCounter, 4); // 1 call after setColumnCount, 3 calls after undo since the signal is emitted for every columns
	QCOMPARE(columnsInsertedCounter, 4); // same as for columnsAboutToBeInsertedCounter
	QCOMPARE(columnsAboutToBeRemovedCounter, 3); // 3 columns removed in undo()
	QCOMPARE(columnsRemovedCounter, 3); // same as for columnsAboutToBeRemovedCounter

	// TODO: improve Spreadsheet::insertColumns() to reduce the number of emits and acitvate the checks below
	/*
	QCOMPARE(columnsAboutToBeInsertedCounter, 2); // set and redo()
	QCOMPARE(columnsInsertedCounter, 2); // set and redo()
	QCOMPARE(columnsRemovedCounter, 1); // undo()
	QCOMPARE(columnsAboutToBeRemovedCounter, 1); // undo()
	*/
}

void SpreadsheetTest::testRemoveColumns() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	auto* model = new SpreadsheetModel(sheet);

	int columnsAboutToBeInsertedCounter = 0;
	connect(model, &SpreadsheetModel::columnsAboutToBeInserted, [&columnsAboutToBeInsertedCounter]() {
		columnsAboutToBeInsertedCounter++;
	});
	int columnsInsertedCounter = 0;
	connect(model, &SpreadsheetModel::columnsInserted, [&columnsInsertedCounter]() {
		columnsInsertedCounter++;
	});
	int columnsAboutToBeRemovedCounter = 0;
	connect(model, &SpreadsheetModel::columnsAboutToBeRemoved, [&columnsAboutToBeRemovedCounter]() {
		columnsAboutToBeRemovedCounter++;
	});
	int columnsRemovedCounter = 0;
	connect(model, &SpreadsheetModel::columnsRemoved, [&columnsRemovedCounter]() {
		columnsRemovedCounter++;
	});

	QCOMPARE(sheet->columnCount(), 2);
	QCOMPARE(model->columnCount(), 2);
	sheet->setColumnCount(1); // No crash shall happen
	QCOMPARE(sheet->columnCount(), 1);
	QCOMPARE(model->columnCount(), 1);

	sheet->undoStack()->undo();
	QCOMPARE(sheet->columnCount(), 2);
	QCOMPARE(model->columnCount(), 2);
	sheet->undoStack()->redo();
	QCOMPARE(sheet->columnCount(), 1);
	QCOMPARE(model->columnCount(), 1);

	QCOMPARE(columnsAboutToBeInsertedCounter, 1); // undo()
	QCOMPARE(columnsInsertedCounter, 1); // undo()
	QCOMPARE(columnsRemovedCounter, 2); // set and redo()
	QCOMPARE(columnsAboutToBeRemovedCounter, 2); // set and redo()
}

/*!
 * \brief testInsertRowsSuppressUpdate
 * It shall not crash
 * Testing if in the model begin and end are used properly
 */
void SpreadsheetTest::testInsertRowsSuppressUpdate() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	auto* model = new SpreadsheetModel(sheet);

	int rowsAboutToBeInsertedCounter = 0;
	connect(model, &SpreadsheetModel::rowsAboutToBeInserted, [&rowsAboutToBeInsertedCounter]() {
		rowsAboutToBeInsertedCounter++;
	});
	int rowsInsertedCounter = 0;
	connect(model, &SpreadsheetModel::rowsInserted, [&rowsInsertedCounter]() {
		rowsInsertedCounter++;
	});
	int rowsAboutToBeRemovedCounter = 0;
	connect(model, &SpreadsheetModel::rowsAboutToBeRemoved, [&rowsAboutToBeRemovedCounter]() {
		rowsAboutToBeRemovedCounter++;
	});
	int rowsRemovedCounter = 0;
	connect(model, &SpreadsheetModel::rowsRemoved, [&rowsRemovedCounter]() {
		rowsRemovedCounter++;
	});

	int modelResetCounter = 0;
	connect(model, &SpreadsheetModel::modelReset, [&modelResetCounter]() {
		modelResetCounter++;
	});
	int modelAboutToResetCounter = 0;
	connect(model, &SpreadsheetModel::modelAboutToBeReset, [&modelAboutToResetCounter]() {
		modelAboutToResetCounter++;
	});

	model->suppressSignals(true);

	QCOMPARE(sheet->rowCount(), 100);
	sheet->setRowCount(101); // No crash shall happen
	QCOMPARE(sheet->rowCount(), 101);

	sheet->undoStack()->undo();
	QCOMPARE(sheet->rowCount(), 100);
	sheet->undoStack()->redo();
	QCOMPARE(sheet->rowCount(), 101);

	model->suppressSignals(false);

	QCOMPARE(rowsAboutToBeInsertedCounter, 0);
	QCOMPARE(rowsInsertedCounter, 0);
	QCOMPARE(rowsAboutToBeRemovedCounter, 0);
	QCOMPARE(rowsRemovedCounter, 0);
	QCOMPARE(modelResetCounter, 1);
	QCOMPARE(modelAboutToResetCounter, 1);
}

/*!
 * \brief testInsertColumnsSuppressUpdate
 * It shall not crash
 * Testing if in the model begin and end are used properly
 */
void SpreadsheetTest::testInsertColumnsSuppressUpdate() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	auto* model = new SpreadsheetModel(sheet);

	int columnsAboutToBeInsertedCounter = 0;
	connect(model, &SpreadsheetModel::columnsAboutToBeInserted, [&columnsAboutToBeInsertedCounter]() {
		columnsAboutToBeInsertedCounter++;
	});
	int columnsInsertedCounter = 0;
	connect(model, &SpreadsheetModel::columnsInserted, [&columnsInsertedCounter]() {
		columnsInsertedCounter++;
	});
	int columnsAboutToBeRemovedCounter = 0;
	connect(model, &SpreadsheetModel::columnsAboutToBeRemoved, [&columnsAboutToBeRemovedCounter]() {
		columnsAboutToBeRemovedCounter++;
	});
	int columnsRemovedCounter = 0;
	connect(model, &SpreadsheetModel::columnsRemoved, [&columnsRemovedCounter]() {
		columnsRemovedCounter++;
	});

	int modelResetCounter = 0;
	connect(model, &SpreadsheetModel::modelReset, [&modelResetCounter]() {
		modelResetCounter++;
	});
	int modelAboutToResetCounter = 0;
	connect(model, &SpreadsheetModel::modelAboutToBeReset, [&modelAboutToResetCounter]() {
		modelAboutToResetCounter++;
	});

	model->suppressSignals(true);

	QCOMPARE(sheet->columnCount(), 2);
	sheet->setColumnCount(5); // No crash shall happen
	QCOMPARE(sheet->columnCount(), 5);

	sheet->undoStack()->undo();
	QCOMPARE(sheet->columnCount(), 2);
	sheet->undoStack()->redo();
	QCOMPARE(sheet->columnCount(), 5);

	model->suppressSignals(false);

	QCOMPARE(columnsAboutToBeInsertedCounter, 0);
	QCOMPARE(columnsInsertedCounter, 0);
	QCOMPARE(columnsRemovedCounter, 0);
	QCOMPARE(columnsAboutToBeRemovedCounter, 0);
	QCOMPARE(modelResetCounter, 1);
	QCOMPARE(modelAboutToResetCounter, 1);
}

void SpreadsheetTest::testLinkSpreadsheetsUndoRedo() {
#ifdef __FreeBSD__
	return;
#endif
	Project project;
	auto* sheetData = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheetData);
	sheetData->setColumnCount(3);
	sheetData->setRowCount(10);

	auto* sheetData2 = new Spreadsheet(QStringLiteral("data2"), false);
	project.addChild(sheetData2);
	sheetData2->setColumnCount(3);
	sheetData2->setRowCount(100);

	auto* sheetCalculations = new Spreadsheet(QStringLiteral("calculations"), false);
	project.addChild(sheetCalculations);
	sheetCalculations->setColumnCount(3);
	sheetCalculations->setRowCount(2);

	SpreadsheetDock dock(nullptr);
	dock.setSpreadsheets({sheetCalculations});
	auto* modelSheetCalculations = new SpreadsheetModel(sheetCalculations);

	QCOMPARE(dock.ui.cbLinkingEnabled->isChecked(), false);
	QCOMPARE(dock.ui.cbLinkedSpreadsheet->isVisible(), false);
	QCOMPARE(dock.ui.sbRowCount->isEnabled(), true);
	QCOMPARE(dock.m_spreadsheet->linking(), false);

	dock.ui.cbLinkingEnabled->toggled(true);

	// QCOMPARE(dock.ui.cbLinkingEnabled->isChecked(), true); // does not work here. Don't know why
	// QCOMPARE(dock.ui.cbLinkedSpreadsheet->isVisible(), true);
	// QCOMPARE(dock.ui.sbRowCount->isEnabled(), false);
	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), nullptr);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), QLatin1String());
	QCOMPARE(sheetCalculations->rowCount(), 2);
	QCOMPARE(modelSheetCalculations->rowCount(), 2);

	const auto index = dock.aspectModel()->modelIndexOfAspect(sheetData);
	QCOMPARE(index.isValid(), true);
	// dock.ui.cbLinkedSpreadsheet->setCurrentModelIndex(index); // Does not trigger the slot
	sheetCalculations->setLinkedSpreadsheet(sheetData);

	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 10);
	QCOMPARE(modelSheetCalculations->rowCount(), 10);

	sheetCalculations->setLinkedSpreadsheet(sheetData2);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData2);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData2->path());
	QCOMPARE(sheetCalculations->rowCount(), 100);
	QCOMPARE(modelSheetCalculations->rowCount(), 100);

	sheetCalculations->undoStack()->undo();

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 10);
	QCOMPARE(modelSheetCalculations->rowCount(), 10);

	sheetCalculations->undoStack()->redo();

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData2);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData2->path());
	QCOMPARE(sheetCalculations->rowCount(), 100);
	QCOMPARE(modelSheetCalculations->rowCount(), 100);

	sheetCalculations->undoStack()->undo(); // first undo
	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 10);
	QCOMPARE(modelSheetCalculations->rowCount(), 10);

	sheetCalculations->undoStack()->undo();

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), nullptr); // No linked spreadsheet anymore
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), QLatin1String());
	QCOMPARE(sheetCalculations->rowCount(), 2); // Go back to original row count
	QCOMPARE(modelSheetCalculations->rowCount(), 2);

	sheetCalculations->undoStack()->undo();
	QCOMPARE(sheetCalculations->linking(), false);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), nullptr);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), QLatin1String());
	QCOMPARE(sheetCalculations->rowCount(), 2);
	QCOMPARE(modelSheetCalculations->rowCount(), 2);
}

void SpreadsheetTest::testLinkSpreadsheetDeleteAdd() {
#ifdef __FreeBSD__
	return;
#endif
	Project project;
	auto* sheetData = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheetData);
	sheetData->setColumnCount(3);
	sheetData->setRowCount(10);

	auto* sheetCalculations = new Spreadsheet(QStringLiteral("calculations"), false);
	project.addChild(sheetCalculations);
	sheetCalculations->setColumnCount(3);
	sheetCalculations->setRowCount(2);

	SpreadsheetDock dock(nullptr);
	dock.setSpreadsheets({sheetCalculations});

	QCOMPARE(dock.ui.cbLinkingEnabled->isChecked(), false);
	QCOMPARE(dock.ui.cbLinkedSpreadsheet->isVisible(), false);
	QCOMPARE(dock.ui.sbRowCount->isEnabled(), true);
	QCOMPARE(sheetCalculations->linking(), false);

	Q_EMIT dock.ui.cbLinkingEnabled->toggled(true);

	sheetCalculations->setLinkedSpreadsheet(sheetData);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 10);

	sheetData->remove();
	sheetData->setRowCount(100);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), nullptr);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), i18n("Project") + QStringLiteral("/data"));
	QCOMPARE(sheetCalculations->rowCount(), 10); // does not change

	auto* sheetDataNew = new Spreadsheet(QStringLiteral("data"), false);
	sheetDataNew->setColumnCount(3);
	sheetDataNew->setRowCount(12);
	project.addChild(sheetDataNew);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetDataNew);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetDataNew->path());
	QCOMPARE(sheetCalculations->rowCount(), 12);
}

void SpreadsheetTest::testLinkSpreadsheetAddRow() {
#ifdef __FreeBSD__
	return;
#endif
	Project project;
	auto* sheetData = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheetData);
	sheetData->setColumnCount(3);
	sheetData->setRowCount(10);

	auto* sheetCalculations = new Spreadsheet(QStringLiteral("calculations"), false);
	project.addChild(sheetCalculations);
	sheetCalculations->setColumnCount(3);
	sheetCalculations->setRowCount(2);

	SpreadsheetDock dock(nullptr);
	dock.setSpreadsheets({sheetCalculations});

	QCOMPARE(dock.ui.cbLinkingEnabled->isChecked(), false);
	QCOMPARE(dock.ui.cbLinkedSpreadsheet->isVisible(), false);
	QCOMPARE(dock.ui.sbRowCount->isEnabled(), true);
	QCOMPARE(sheetCalculations->linking(), false);

	Q_EMIT dock.ui.cbLinkingEnabled->toggled(true);

	sheetCalculations->setLinkedSpreadsheet(sheetData);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 10);

	new SpreadsheetModel(sheetData); // otherwise emitRowCountChanged will not be called
	sheetData->setRowCount(13);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 13);
}

void SpreadsheetTest::testLinkSpreadsheetRemoveRow() {
#ifdef __FreeBSD__
	return;
#endif
	Project project;
	auto* sheetData = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheetData);
	sheetData->setColumnCount(3);
	sheetData->setRowCount(10);

	auto* sheetCalculations = new Spreadsheet(QStringLiteral("calculations"), false);
	project.addChild(sheetCalculations);
	sheetCalculations->setColumnCount(3);
	sheetCalculations->setRowCount(2);

	SpreadsheetDock dock(nullptr);
	dock.setSpreadsheets({sheetCalculations});

	QCOMPARE(dock.ui.cbLinkingEnabled->isChecked(), false);
	QCOMPARE(dock.ui.cbLinkedSpreadsheet->isVisible(), false);
	QCOMPARE(dock.ui.sbRowCount->isEnabled(), true);
	QCOMPARE(sheetCalculations->linking(), false);

	dock.ui.cbLinkingEnabled->toggled(true);

	sheetCalculations->setLinkedSpreadsheet(sheetData);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 10);

	new SpreadsheetModel(sheetData); // otherwise emitRowCountChanged will not be called
	sheetData->setRowCount(7);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 7);
}

void SpreadsheetTest::testLinkSpreadsheetRecalculate() {
#ifdef __FreeBSD__
	return;
#endif
	Project project;
	auto* sheetData = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheetData);
	sheetData->setColumnCount(2);
	sheetData->setRowCount(10);
	auto* sheetDataColumn0 = sheetData->child<Column>(0);
	sheetDataColumn0->replaceValues(0, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
	QVERIFY(sheetDataColumn0);
	auto* sheetDataColumn1 = sheetData->child<Column>(1);
	QVERIFY(sheetDataColumn1);
	sheetDataColumn1->replaceValues(0, {1, 2, 1, 2, 1, 2, 1, 2, 1, 3});

	auto* sheetCalculations = new Spreadsheet(QStringLiteral("calculations"), false);
	project.addChild(sheetCalculations);
	sheetCalculations->setColumnCount(1);
	sheetCalculations->setRowCount(2);
	auto* sheetCalculationsColumn0 = sheetCalculations->child<Column>(0);
	QVERIFY(sheetCalculationsColumn0);
	sheetCalculationsColumn0->setFormula(QStringLiteral("x + y"), {QStringLiteral("x"), QStringLiteral("y")}, {sheetDataColumn0, sheetDataColumn1}, true);
	sheetCalculationsColumn0->updateFormula();

	{
		QVector<double> ref{2, 4, 4, 6, 6, 8, 8, 10, 10, 13};
		QCOMPARE(sheetCalculationsColumn0->rowCount(), 10); // currently the update() triggers a resize
		for (int i = 0; i < 10; i++)
			VALUES_EQUAL(sheetCalculationsColumn0->doubleAt(i), ref.at(i));
	}
	sheetCalculations->setLinking(true);
	sheetCalculations->setLinkedSpreadsheet(sheetData);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 10);

	sheetData->setRowCount(7);
	sheetDataColumn0->replaceValues(0, {3, 4, 6, 2, 1, 8, 5});
	QCOMPARE(sheetDataColumn0->rowCount(), 7);

	{
		QVector<double> ref{4, 6, 7, 4, 2, 10, 6};
		QCOMPARE(sheetCalculationsColumn0->rowCount(), ref.count());
		for (int i = 0; i < ref.count(); i++)
			VALUES_EQUAL(sheetCalculationsColumn0->doubleAt(i), ref.at(i));
	}
}

void SpreadsheetTest::testLinkSpreadsheetRecalculateRowCountChange() {
#ifdef __FreeBSD__
	return;
#endif
	Project project;
	auto* sheetData = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheetData);
	sheetData->setColumnCount(2);
	sheetData->setRowCount(10);
	auto* sheetDataColumn0 = sheetData->child<Column>(0);
	sheetDataColumn0->setColumnMode(AbstractColumn::ColumnMode::Integer);
	sheetDataColumn0->replaceInteger(0, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
	QVERIFY(sheetDataColumn0);

	auto* sheetCalculations = new Spreadsheet(QStringLiteral("calculations"), false);
	project.addChild(sheetCalculations);
	sheetCalculations->setColumnCount(1);
	sheetCalculations->setRowCount(2);
	auto* sheetCalculationsColumn0 = sheetCalculations->child<Column>(0);
	QVERIFY(sheetCalculationsColumn0);
	sheetCalculationsColumn0->setFormula(QStringLiteral("x"), {QStringLiteral("x")}, {sheetDataColumn0}, true, false);
	sheetCalculationsColumn0->updateFormula();

	{
		QVector<double> ref{1, 2};
		QCOMPARE(sheetCalculationsColumn0->rowCount(), 2);
		for (int i = 0; i < 2; i++)
			VALUES_EQUAL(sheetCalculationsColumn0->doubleAt(i), ref.at(i));
	}
	sheetCalculations->setLinking(true);
	sheetCalculations->setLinkedSpreadsheet(sheetData);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 10);

	new SpreadsheetModel(sheetData); // otherwise emitRowCountChanged will not be called
	sheetData->setRowCount(7);
	sheetDataColumn0->replaceInteger(0, {1, 2, 3, 4, 5, 6, 7});
	QCOMPARE(sheetDataColumn0->rowCount(), 7);

	{
		QCOMPARE(sheetCalculationsColumn0->rowCount(), 7);
		for (int i = 0; i < 7; i++)
			VALUES_EQUAL(sheetCalculationsColumn0->doubleAt(i), i + 1.);
	}

	sheetData->setRowCount(8);

	{
		QCOMPARE(sheetCalculationsColumn0->rowCount(), 8);
		for (int i = 0; i < 8; i++) {
			if (i < 7)
				VALUES_EQUAL(sheetCalculationsColumn0->doubleAt(i), i + 1.);
			else
				VALUES_EQUAL(sheetCalculationsColumn0->doubleAt(i), 0.); // When inserting a row for an integer column, the initial value is zero
		}
	}

	sheetData->setRowCount(7);

	{
		QCOMPARE(sheetCalculationsColumn0->rowCount(), 7);
		for (int i = 0; i < 7; i++)
			VALUES_EQUAL(sheetCalculationsColumn0->doubleAt(i), i + 1.);
	}
}

void SpreadsheetTest::testLinkSpreadsheetSaveLoad() {
#ifdef __FreeBSD__
	return;
#endif
	QString savePath;
	{
		Project project;
		auto model = new AspectTreeModel(&project, this);
		ProjectExplorer pe; // Needed otherwise the state key is missing in the file and then no restorePointers will be called
		pe.setProject(&project);
		pe.setModel(model);
		auto* sheetData = new Spreadsheet(QStringLiteral("data"), false);
		project.addChild(sheetData);
		sheetData->setColumnCount(3);
		sheetData->setRowCount(10);

		auto* sheetCalculations = new Spreadsheet(QStringLiteral("calculations"), false);
		project.addChild(sheetCalculations);
		sheetCalculations->setColumnCount(3);
		sheetCalculations->setRowCount(2);

		SpreadsheetDock dock(nullptr);
		dock.setSpreadsheets({sheetCalculations});

		QCOMPARE(dock.ui.cbLinkingEnabled->isChecked(), false);
		QCOMPARE(dock.ui.cbLinkedSpreadsheet->isVisible(), false);
		QCOMPARE(dock.ui.sbRowCount->isEnabled(), true);
		QCOMPARE(sheetCalculations->linking(), false);

		Q_EMIT dock.ui.cbLinkingEnabled->toggled(true);

		sheetCalculations->setLinkedSpreadsheet(sheetData);

		QCOMPARE(sheetCalculations->linking(), true);
		QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
		QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
		QCOMPARE(sheetCalculations->rowCount(), 10);

		SAVE_PROJECT("testLinkSpreadsheetSaveLoad");
	}

	{
		Project project;
		QCOMPARE(project.load(savePath), true);

		auto sheetData = project.child<Spreadsheet>(0);
		QVERIFY(sheetData);
		auto sheetCalculations = project.child<Spreadsheet>(1);
		QVERIFY(sheetCalculations);

		QCOMPARE(sheetCalculations->linking(), true);
		QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
		QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
		QCOMPARE(sheetCalculations->rowCount(), 10);

		sheetData->setRowCount(11); // Changing shall also update sheetCalculations also after loading

		QCOMPARE(sheetCalculations->linking(), true);
		QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
		QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
		QCOMPARE(sheetCalculations->rowCount(), 11);
	}
}

void SpreadsheetTest::testLinkSpreadsheetsModelDockUpdateCheckRemoveRows() {
#ifdef __FreeBSD__
	return;
#endif
	Project project;
	auto* sheetData = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheetData);
	sheetData->setRowCount(20); // smaller

	auto* sheetCalculations = new Spreadsheet(QStringLiteral("calculations"), false);
	project.addChild(sheetCalculations);
	sheetCalculations->setRowCount(100);

	const auto* model = new SpreadsheetModel(sheetCalculations);
	SpreadsheetDock dock(nullptr);
	dock.setSpreadsheets({sheetCalculations});

	sheetCalculations->setLinking(true);
	sheetCalculations->setLinkedSpreadsheet(sheetData);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 20);
	QCOMPARE(model->rowCount(), 20);
	QCOMPARE(dock.ui.sbRowCount->value(), 20);
}

void SpreadsheetTest::testLinkSpreadsheetsModelDockUpdateCheckInsertRows() {
#ifdef __FreeBSD__
	return;
#endif
	Project project;
	auto* sheetData = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheetData);
	sheetData->setRowCount(100); // larger

	auto* sheetCalculations = new Spreadsheet(QStringLiteral("calculations"), false);
	project.addChild(sheetCalculations);
	sheetCalculations->setRowCount(20);

	const auto* model = new SpreadsheetModel(sheetCalculations);
	SpreadsheetDock dock(nullptr);
	dock.setSpreadsheets({sheetCalculations});

	sheetCalculations->setLinking(true);
	sheetCalculations->setLinkedSpreadsheet(sheetData);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 100);
	QCOMPARE(model->rowCount(), 100);
	QCOMPARE(dock.ui.sbRowCount->value(), 100);
}

// **********************************************************
// **************** statistics spreadsheet  *****************
// **********************************************************
/*!
 * toggle on and off the statistics spreadsheet and check its presence
 */
void SpreadsheetTest::testStatisticsSpreadsheetToggle() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	// initial
	auto children = sheet->children<StatisticsSpreadsheet>();
	QCOMPARE(children.size(), 0);

	// toggle on
	sheet->toggleStatisticsSpreadsheet(true);
	children = sheet->children<StatisticsSpreadsheet>();
	QCOMPARE(children.size(), 1);

	// toggle off
	sheet->toggleStatisticsSpreadsheet(false);
	children = sheet->children<StatisticsSpreadsheet>();
	QCOMPARE(children.size(), 0);
}

/*!
 * change the statistics metrics and check the presense of the corresponding columns
 */
void SpreadsheetTest::testStatisticsSpreadsheetChangeMetrics() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	new SpreadsheetModel(sheet);

	// toggle on the statistics spreadsheet and count the available columns
	sheet->toggleStatisticsSpreadsheet(true);
	auto* statisticsSpreadsheet = sheet->children<StatisticsSpreadsheet>().constFirst();
	auto metrics = statisticsSpreadsheet->metrics();
	int colCount = 1; // column "Column Name" is always available
	for (const auto& metric : statisticsSpreadsheet->m_metricValues) {
		if (metrics.testFlag(metric))
			++colCount;
	}
	QCOMPARE(statisticsSpreadsheet->children<Column>().size(), colCount);

	// deactivate Count and check the available columns again
	metrics.setFlag(StatisticsSpreadsheet::Metric::Count, false);
	statisticsSpreadsheet->setMetrics(metrics);
	QCOMPARE(statisticsSpreadsheet->children<Column>().size(), colCount - 1);
}

/*!
 * check the position of the statistics spreadsheet in the list of children of the parent
 * spreadsheet, it should always be put at the last position.
 */
void SpreadsheetTest::testStatisticsSpreadsheetChildIndex() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	// toggle on the statistics spreadsheet and count the available columns
	sheet->toggleStatisticsSpreadsheet(true);
	auto* statisticsSpreadsheet = sheet->children<StatisticsSpreadsheet>().constFirst();

	// set the column count and check the position of the statistics spreadsheet
	sheet->setColumnCount(2);
	QCOMPARE(sheet->indexOfChild<AbstractAspect>(statisticsSpreadsheet), 2);

	// change the column count and check the position again
	sheet->setColumnCount(3);
	QCOMPARE(sheet->indexOfChild<AbstractAspect>(statisticsSpreadsheet), 3);

	// append one more column and check the position again
	sheet->appendColumn();
	QCOMPARE(sheet->indexOfChild<AbstractAspect>(statisticsSpreadsheet), 4);

	// check also the position when adding new columns via the actions in the view that also
	// take the current selected column into account and have more logic for "append left to"
	// and "append right to" - ensure no columns are added after the statistics spreadhseet
	auto* view = static_cast<SpreadsheetView*>(sheet->view());

	// insert a new column right to the last selected column, it should be placed in front of the statistics spreadsheet
	view->selectColumn(3); // select the last 4th column
	view->insertColumnsRight(1);
	QCOMPARE(sheet->indexOfChild<AbstractAspect>(statisticsSpreadsheet), 5);
}

/*!
 * check the position of the statistics spreadsheet in the list of children of the parent
 * spreadsheet after an undo and redo of a column appen, it should always be put at the last position,
 * and also check  the handling of added and removed childrend in the model in the presense of the
 * statistics spreadsheet.
 */
void SpreadsheetTest::testStatisticsSpreadsheetChildIndexAfterUndoRedo() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	new SpreadsheetModel(sheet);

	// toggle on the statistics spreadsheet and count the available columns
	sheet->toggleStatisticsSpreadsheet(true);
	auto* statisticsSpreadsheet = sheet->children<StatisticsSpreadsheet>().constFirst();

	// set the column count , append a column, undo and redo this change and check the position of the statistics spreadsheet
	sheet->setColumnCount(2);
	sheet->appendColumn();
	QCOMPARE(sheet->model()->columnCount(), 3);
	QCOMPARE(sheet->indexOfChild<AbstractAspect>(statisticsSpreadsheet), 3);

	project.undoStack()->undo();
	QCOMPARE(sheet->model()->columnCount(), 2);
	QCOMPARE(sheet->indexOfChild<AbstractAspect>(statisticsSpreadsheet), 2);

	project.undoStack()->redo();
	QCOMPARE(sheet->model()->columnCount(), 3);
	QCOMPARE(sheet->indexOfChild<AbstractAspect>(statisticsSpreadsheet), 3);
}

#ifdef HAVE_VECTOR_BLF
// Copied from BLFFilterTest.cpp
namespace {
static const std::string PRIMITIVE_DBC =
	R"(VERSION "1.0.0"

NS_ :

BS_:

BU_: DBG DRIVER IO MOTOR SENSOR

)";

void createDBCFile(const QString& filename, const std::string& content) {
	auto* file = std::fopen(filename.toStdString().c_str(), "w");
	QVERIFY(file);
	std::fputs(PRIMITIVE_DBC.c_str(), file);
	std::fputs(content.c_str(), file);
	std::fclose(file);
}

Vector::BLF::CanMessage2* createCANMessage(uint32_t id, uint64_t timestamp, const std::vector<uint8_t>& data) {
	auto* canMessage = new Vector::BLF::CanMessage2();
	canMessage->channel = 1;
	canMessage->flags = 1; // TX
	canMessage->dlc = std::min<uint8_t>(data.size(), 8);
	canMessage->id = id;
	canMessage->objectTimeStamp = timestamp;
	canMessage->objectFlags = Vector::BLF::ObjectHeader::ObjectFlags::TimeOneNans;
	if (canMessage->data.size() < canMessage->dlc)
		canMessage->data.resize(canMessage->dlc);

	for (int i = 0; i < canMessage->dlc; i++) {
		canMessage->data[i] = data.at(i);
	}
	return canMessage;
}

void createBLFFile(const QString& filename, QVector<Vector::BLF::CanMessage2*> messages) {
	Vector::BLF::File blfFile;
	blfFile.open(filename.toStdString().c_str(), std::ios_base::out);
	QVERIFY(blfFile.is_open());

	for (auto msg : messages) {
		blfFile.write(msg);
	}
	// Finish creating files
	blfFile.close();
}
}

void SpreadsheetTest::testLinkSpreadSheetImportBLF() {
	QTemporaryFile blfFileName(QStringLiteral("XXXXXX.blf"));
	QVERIFY(blfFileName.open());
	QVector<Vector::BLF::CanMessage2*> messages{
		createCANMessage(337, 5, {0, 4, 252, 19, 0, 0, 0, 0}),
		createCANMessage(541, 10, {7, 39, 118, 33, 250, 30, 76, 24}), // 99.91, 85.66, 79.3, 22.2
		createCANMessage(337, 15, {47, 4, 60, 29, 0, 0, 0, 0}),
		createCANMessage(337, 20, {57, 4, 250, 29, 0, 0, 0, 0}),
		createCANMessage(541, 25, {7, 39, 118, 33, 250, 30, 76, 24}), // 99.91, 85.66, 79.3, 22.2
	}; // time is in nanoseconds
	createBLFFile(blfFileName.fileName(), messages);

	QTemporaryFile dbcFile(QStringLiteral("XXXXXX.dbc"));
	QVERIFY(dbcFile.open());
	const auto dbcContent = R"(BO_ 337 STATUS: 8 Vector__XXX
 SG_ Value6 : 27|3@1+ (1,0) [0|7] ""  Vector__XXX
 SG_ Value5 : 16|11@1+ (0.1,-102) [-102|102] "%"  Vector__XXX
 SG_ Value2 : 8|2@1+ (1,0) [0|2] ""  Vector__XXX
 SG_ Value3 : 10|1@1+ (1,0) [0|1] ""  Vector__XXX
 SG_ Value7 : 30|2@1+ (1,0) [0|3] ""  Vector__XXX
 SG_ Value4 : 11|4@1+ (1,0) [0|3] ""  Vector__XXX
 SG_ Value1 : 0|8@1+ (1,0) [0|204] "Km/h"  Vector__XXX"
BO_ 541 MSG2: 8 Vector__XXX
 SG_ MSG2Value4 : 48|16@1+ (0.01,-40) [-40|125] "C"  Vector__XXX
 SG_ MSG2Value1 : 0|16@1+ (0.01,0) [0|100] "%"  Vector__XXX
 SG_ MSG2Value3 : 32|16@1+ (0.01,0) [0|100] "%"  Vector__XXX
 SG_ MSG2Value2 : 16|16@1+ (0.01,0) [0|100] "%"  Vector__XXX
)";
	createDBCFile(dbcFile.fileName(), dbcContent);

	//------------------------------------------------------------------------------------------
	Project project;
	const auto spreadsheetName = blfFileName.fileName().replace(QStringLiteral(".blf"), QStringLiteral(""));
	auto* sheetData = new Spreadsheet(spreadsheetName, false);
	project.addChild(sheetData);
	sheetData->setColumnCount(2);
	sheetData->setRowCount(10);
	auto* sheetDataColumn0 = sheetData->child<Column>(0);
	sheetDataColumn0->setName(QStringLiteral("Value6_"));
	sheetDataColumn0->replaceValues(0, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
	QVERIFY(sheetDataColumn0);
	auto* sheetDataColumn1 = sheetData->child<Column>(1);
	sheetDataColumn1->setName(QStringLiteral("Value5_%"));
	QVERIFY(sheetDataColumn1);
	sheetDataColumn1->replaceValues(0, {1, 2, 1, 2, 1, 2, 1, 2, 1, 3});

	auto* sheetCalculations = new Spreadsheet(QStringLiteral("calculations"), false);
	project.addChild(sheetCalculations);
	sheetCalculations->setColumnCount(2);
	sheetCalculations->setRowCount(2);

	auto* sheetCalculationsColumn0 = sheetCalculations->child<Column>(0);
	QVERIFY(sheetCalculationsColumn0);
	sheetCalculationsColumn0->setFormula(QStringLiteral("2*x"), {QStringLiteral("x")}, {sheetDataColumn0}, true);
	sheetCalculationsColumn0->updateFormula();

	auto* sheetCalculationsColumn1 = sheetCalculations->child<Column>(1);
	QVERIFY(sheetCalculationsColumn1);
	sheetCalculationsColumn1->setFormula(QStringLiteral("2*x"), {QStringLiteral("x")}, {sheetDataColumn1}, true);
	sheetCalculationsColumn1->updateFormula();

	{
		QVector<double> ref{2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
		QCOMPARE(sheetCalculationsColumn0->rowCount(), 10);
		for (int i = 0; i < 10; i++)
			VALUES_EQUAL(sheetCalculationsColumn0->doubleAt(i), ref.at(i));
	}

	{
		QVector<double> ref{2, 4, 2, 4, 2, 4, 2, 4, 2, 6};
		QCOMPARE(sheetCalculationsColumn1->rowCount(), 10);
		for (int i = 0; i < 10; i++)
			VALUES_EQUAL(sheetCalculationsColumn1->doubleAt(i), ref.at(i));
	}

	sheetCalculations->setLinking(true);
	sheetCalculations->setLinkedSpreadsheet(sheetData);

	QCOMPARE(sheetCalculations->linking(), true);
	QCOMPARE(sheetCalculations->linkedSpreadsheet(), sheetData);
	QCOMPARE(sheetCalculations->linkedSpreadsheetPath(), sheetData->path());
	QCOMPARE(sheetCalculations->rowCount(), 10);

	new SpreadsheetModel(sheetData); // otherwise emitRowCountChanged will not be called

	VectorBLFFilter filter;
	filter.setConvertTimeToSeconds(true);
	filter.setTimeHandlingMode(CANFilter::TimeHandling::ConcatPrevious);
	QCOMPARE(filter.isValid(blfFileName.fileName()), true);

	// Valid blf and valid dbc
	filter.setDBCFile(dbcFile.fileName());
	filter.readDataFromFile(blfFileName.fileName(), sheetData);
	QCOMPARE(sheetData->columnCount(), 12);

	QCOMPARE(sheetData->rowCount(), 5);
	QCOMPARE(sheetDataColumn0->rowCount(), 5);
	QCOMPARE(sheetDataColumn1->rowCount(), 5);

	const auto* sheetDataColumn6 = sheetData->child<Column>(1);
	QCOMPARE(sheetDataColumn6->name(), QStringLiteral("Value6_"));
	const auto* sheetDataColumn5 = sheetData->child<Column>(2);
	QCOMPARE(sheetDataColumn5->name(), QStringLiteral("Value5_%"));

	{
		QVector<double> ref{4., 4., 6., 6., 6.};
		const auto* sheetCalculationsColumn = sheetCalculations->child<Column>(0);
		QCOMPARE(sheetCalculationsColumn->formulaData().at(0).column(), sheetDataColumn6);
		QCOMPARE(sheetCalculationsColumn->rowCount(), ref.count());
		for (int i = 0; i < ref.count(); i++)
			VALUES_EQUAL(sheetCalculationsColumn->doubleAt(i), ref.at(i));
	}

	{
		QVector<double> ref{0., 0., 64., 102., 102.};
		const auto* sheetCalculationsColumn = sheetCalculations->child<Column>(1);
		QCOMPARE(sheetCalculationsColumn->formulaData().at(0).column(), sheetDataColumn5);
		QCOMPARE(sheetCalculationsColumn->rowCount(), ref.count());
		for (int i = 0; i < ref.count(); i++)
			VALUES_EQUAL(sheetCalculationsColumn->doubleAt(i), ref.at(i));
	}
}
#endif // HAVE_VECTOR_BLF

/*!
 * Columns are named in the correct number order
 */
void SpreadsheetTest::testNaming() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	new SpreadsheetModel(sheet);

	QCOMPARE(sheet->columnCount(), 2);
	QCOMPARE(sheet->column(0)->name(), QStringLiteral("1"));
	QCOMPARE(sheet->column(1)->name(), QStringLiteral("2"));

	sheet->setColumnCount(10);
	QCOMPARE(sheet->columnCount(), 10);
	for (int i = 0; i < 10; i++) {
		QCOMPARE(sheet->column(i)->name(), QString::number(i + 1));
	}
}

void SpreadsheetTest::testClearColumns() {
	Project project;
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);

	new SpreadsheetModel(sheet);

	sheet->setColumnCount(3);
	QCOMPARE(sheet->columnCount(), 3);
	sheet->setRowCount(10);

	auto* c0 = sheet->column(0);
	auto* c1 = sheet->column(1);
	auto* c2 = sheet->column(2);

	for (int i = 0; i < 10; i++) {
		c0->setValueAt(i, i);
		c1->setValueAt(i, 2. * i + 3.);
		c2->setValueAt(i, pow(i, 3.));
	}

	for (int i = 0; i < 10; i++) {
		QCOMPARE(c0->valueAt(i), i);
		QCOMPARE(c1->valueAt(i), 2. * i + 3.);
		QCOMPARE(c2->valueAt(i), pow(i, 3.));
	}

	sheet->clear({c0, c2});

	for (int i = 0; i < 10; i++) {
		QCOMPARE(c0->valueAt(i), NAN);
		QCOMPARE(c1->valueAt(i), 2. * i + 3.);
		QCOMPARE(c2->valueAt(i), NAN);
	}

	sheet->undoStack()->undo();

	for (int i = 0; i < 10; i++) {
		QCOMPARE(c0->valueAt(i), i);
		QCOMPARE(c1->valueAt(i), 2. * i + 3.);
		QCOMPARE(c2->valueAt(i), pow(i, 3.));
	}
}

QTEST_MAIN(SpreadsheetTest)
