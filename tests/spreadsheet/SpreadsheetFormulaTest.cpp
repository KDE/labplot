/*
	File                 : SpreadsheetFormulaTest.cpp
	Project              : LabPlot
	Description          : Tests for formula in spreadsheets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2023-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetFormulaTest.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/spreadsheet/SpreadsheetView.h"

#include <QClipboard>

#define INIT_SPREADSHEET                                                                                                                                       \
	Spreadsheet sheet(QStringLiteral("test 2 cols"), false);                                                                                                   \
	const int cols = 2;                                                                                                                                        \
	const int rows = 100;                                                                                                                                      \
                                                                                                                                                               \
	sheet.setColumnCount(cols);                                                                                                                                \
	sheet.setRowCount(rows);                                                                                                                                   \
                                                                                                                                                               \
	SpreadsheetView view(&sheet, false);                                                                                                                       \
	view.selectColumn(0);                                                                                                                                      \
	view.fillWithRowNumbers();                                                                                                                                 \
                                                                                                                                                               \
	QStringList variableNames;                                                                                                                                 \
	variableNames << QLatin1String("x");                                                                                                                       \
	QVector<Column*> variableColumns;                                                                                                                          \
	variableColumns << sheet.column(0);                                                                                                                        \
	sheet.column(1)->setFormulaVariableColumn(sheet.column(0));

#define INIT_SPREADSHEET2                                                                                                                                      \
	Spreadsheet sheet(QStringLiteral("test 3 cols"), false);                                                                                                   \
	const int cols = 3;                                                                                                                                        \
	const int rows = 100;                                                                                                                                      \
                                                                                                                                                               \
	sheet.setColumnCount(cols);                                                                                                                                \
	sheet.setRowCount(rows);                                                                                                                                   \
                                                                                                                                                               \
	SpreadsheetView view(&sheet, false);                                                                                                                       \
	view.selectColumn(0);                                                                                                                                      \
	view.fillWithRowNumbers();                                                                                                                                 \
	for (int i = 0; i < rows; i++)                                                                                                                             \
		sheet.column(1)->setValueAt(i, 1.);                                                                                                                    \
                                                                                                                                                               \
	QStringList variableNames;                                                                                                                                 \
	variableNames << QStringLiteral("x") << QStringLiteral("y");                                                                                               \
	QVector<Column*> variableColumns;                                                                                                                          \
	variableColumns << sheet.column(0) << sheet.column(1);                                                                                                     \
	sheet.column(2)->setFormulaVariableColumn(sheet.column(0));                                                                                                \
	sheet.column(2)->setFormulaVariableColumn(sheet.column(1));

//**********************************************************
//********** Check different formulas **********************
//**********************************************************
/*!
   formula "1"
*/
void SpreadsheetFormulaTest::formula1() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("1"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// spreadsheet size
	QCOMPARE(sheet.columnCount(), cols);
	QCOMPARE(sheet.rowCount(), rows);

	// column modes
	QCOMPARE(sheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(sheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), 1);
	}
}
/*!
   formula "x"
*/
void SpreadsheetFormulaTest::formulax() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("x"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), i + 1);
	}
}
/*!
   formula "x + 1"
*/
void SpreadsheetFormulaTest::formulaxp1() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("x+1"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), i + 2);
	}
}
//**********************************************************
//********** Check different cell() formulas ***************
//**********************************************************
/*!
   formula "cell(1, x)"
*/
void SpreadsheetFormulaTest::formulaCell1() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("cell(1; x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), 1);
	}
}
/*!
   formula "cell(i, x)"
*/
void SpreadsheetFormulaTest::formulaCelli() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("cell(i; x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), i + 1);
	}
}
/*!
   formula "cell(i + 1, x)"
*/
void SpreadsheetFormulaTest::formulaCellip1() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("cell(i + 1; x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i < rows - 1)
			QCOMPARE(sheet.column(1)->valueAt(i), i + 2);
		else
			QCOMPARE(sheet.column(1)->valueAt(i), NAN);
	}
}
/*!
   formula "cell(i - 1, x)"
*/
void SpreadsheetFormulaTest::formulaCellim1() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("cell(i - 1; x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i > 0)
			QCOMPARE(sheet.column(1)->valueAt(i), i);
		else
			QCOMPARE(sheet.column(1)->valueAt(i), NAN);
	}
}
/*!
   formula "cell(2*i, x)"
*/
void SpreadsheetFormulaTest::formulaCell2i() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("cell(2*i; x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i < rows / 2)
			QCOMPARE(sheet.column(1)->valueAt(i), 2 * (i + 1));
		else
			QCOMPARE(sheet.column(1)->valueAt(i), NAN);
	}
}
/*!
   formula "cell(i+1, x) - cell(i-1, x)"
*/
void SpreadsheetFormulaTest::formulaCellip1im1() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("cell(i+1; x) - cell(i-1; x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i > 0 && i < rows - 1)
			QCOMPARE(sheet.column(1)->valueAt(i), 2);
		else
			QCOMPARE(sheet.column(1)->valueAt(i), NAN);
	}
}
/*!
   formula "sqrt(cell(i+1, x))"
*/
void SpreadsheetFormulaTest::formulaCellsqrtip1() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("sqrt(cell(i+1; x))"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i < rows - 1)
			QCOMPARE(sheet.column(1)->valueAt(i), std::sqrt(i + 2));
		else
			QCOMPARE(sheet.column(1)->valueAt(i), NAN);
	}
}

/*!
   formula "cell(1, 2*x)"
*/
void SpreadsheetFormulaTest::formulaCell1_2x() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("2*cell(1; x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), 2);
	}
}
/*!
   formula "cell(i, 2*x)"
*/
void SpreadsheetFormulaTest::formulaCelli_2x() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("cell(i; x) * 2"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), 2 * (i + 1));
	}
}
/*!
   formula "cell(1, x+x)"
*/
void SpreadsheetFormulaTest::formulaCelli_xpx() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("cell(i; x) * 2"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), 2 * (i + 1));
	}
}
/*!
   formula "cell(i, x + 2*x)"
*/
void SpreadsheetFormulaTest::formulaCelli_xp2x() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("3 * cell(i; x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), 3 * (i + 1));
	}
}
/*!
   formula "cell(i, sqrt(x))"
*/
void SpreadsheetFormulaTest::formulaCelli_sqrtx() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("sqrt(cell(i; x))"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), std::sqrt(i + 1));
	}
}
/*!
   formula "cell(i, x+y)"
*/
void SpreadsheetFormulaTest::formulaCelli_xpy() {
	INIT_SPREADSHEET2

	sheet.column(2)->setFormula(QLatin1String("cell(i; x) + cell(i; y)"), variableNames, variableColumns, true);
	sheet.column(2)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), 1);
		QCOMPARE(sheet.column(2)->valueAt(i), i + 2);
	}
}

/*!
   formula "cell(2*i, x+y)"
*/
void SpreadsheetFormulaTest::formulaCell2i_xpy() {
	INIT_SPREADSHEET2

	sheet.column(2)->setFormula(QLatin1String("cell(2*i; x) + cell(2*i; y)"), variableNames, variableColumns, true);
	sheet.column(2)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), 1);
		if (i < rows / 2)
			QCOMPARE(sheet.column(2)->valueAt(i), sheet.column(0)->valueAt(2 * i + 1) + sheet.column(1)->valueAt(2 * i + 1));
		else
			QCOMPARE(sheet.column(2)->valueAt(i), NAN);
	}
}
/*!
   formula "cell(i, 2*x) + cell (i, 2*y)"
*/
void SpreadsheetFormulaTest::formulaCelli_2xpCelli_2y() {
	INIT_SPREADSHEET2

	sheet.column(2)->setFormula(QLatin1String("2*cell(i; x) + cell(i; y) * 2"), variableNames, variableColumns, true);
	sheet.column(2)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), 1);
		QCOMPARE(sheet.column(2)->valueAt(i), 2 * sheet.column(0)->valueAt(i) + 2 * sheet.column(1)->valueAt(i));
	}
}

///////////////////////// check group separator problem /////////////////////

void SpreadsheetFormulaTest::formulaLocale() {
	Spreadsheet sheet(QStringLiteral("test"), false);
	const int cols = 2;
	const int rows = 3;
	const QVector<double> xData{13000, 14000, 15000};

	sheet.setColumnCount(cols);
	sheet.setRowCount(rows);
	auto* col0{sheet.column(0)};
	col0->replaceValues(0, xData);

	SpreadsheetView view(&sheet, false);
	view.selectColumn(0);

	QStringList variableNames;
	variableNames << QLatin1String("x");
	QVector<Column*> variableColumns;
	variableColumns << sheet.column(0);
	sheet.column(1)->setFormulaVariableColumn(sheet.column(0));

	sheet.column(1)->setFormula(QLatin1String("mean(x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++)
		QCOMPARE(sheet.column(1)->valueAt(i), sheet.column(0)->valueAt(1));
}

///////////////////////// more methods /////////////////////

/*!
   formula "ma(x)"
*/
void SpreadsheetFormulaTest::formulama() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("ma(x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i > 0)
			QCOMPARE(sheet.column(1)->valueAt(i), i + .5);
		else
			QCOMPARE(sheet.column(1)->valueAt(i), NAN);
	}
}
/*!
   formula "mr(x)"
*/
void SpreadsheetFormulaTest::formulamr() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("mr(x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i > 0)
			QCOMPARE(sheet.column(1)->valueAt(i), 1);
		else
			QCOMPARE(sheet.column(1)->valueAt(i), NAN);
	}
}
/*!
   formula "sma(n, x)"
*/
void SpreadsheetFormulaTest::formulasma() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("sma(4; x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	const int N = 4;
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		double value = 0.;
		for (int index = std::max(0, i - N + 1); index <= i; index++)
			value += sheet.column(0)->valueAt(index);
		QCOMPARE(sheet.column(1)->valueAt(i), value / N);
	}
}
/*!
   formula "smr(n, x)"
*/
void SpreadsheetFormulaTest::formulasmr() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("smr(4; x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	QCOMPARE(sheet.column(1)->valueAt(0), 0);
	QCOMPARE(sheet.column(1)->valueAt(1), 1);
	QCOMPARE(sheet.column(1)->valueAt(2), 2);
	for (int i = 3; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), 3);
	}
}

// ##############################################################################
// ######### check updates of columns defined via a formula on changes ##########
// ##############################################################################
void SpreadsheetFormulaTest::formulaUpdateAfterCellChange() {
	INIT_SPREADSHEET

	auto* col1 = sheet.column(0);
	auto* col2 = sheet.column(1);
	col2->setFormula(QLatin1String("x"), variableNames, variableColumns, true);
	col2->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(col1->valueAt(i), i + 1);
		QCOMPARE(col2->valueAt(i), i + 1);
	}

	// modify the first cell in the first column and check the updated values
	col1->setIntegerAt(0, 5);

	// check the first row
	QCOMPARE(col1->valueAt(0), 5);
	QCOMPARE(col2->valueAt(0), 5);

	// check the remaining rows
	for (int i = 1; i < rows; i++) {
		QCOMPARE(col1->valueAt(i), i + 1);
		QCOMPARE(col2->valueAt(i), i + 1);
	}
}

void SpreadsheetFormulaTest::formulaUpdateAfterPaste() {
	INIT_SPREADSHEET

	auto* col1 = sheet.column(0);
	auto* col2 = sheet.column(1);
	col2->setFormula(QLatin1String("x"), variableNames, variableColumns, true);
	col2->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(col1->valueAt(i), i + 1);
		QCOMPARE(col2->valueAt(i), i + 1);
	}

	// paste three values into the first column and check the updated values
	const QString str = QStringLiteral("10\n20\n30");
	QApplication::clipboard()->setText(str);
	SpreadsheetView viewToPaste(&sheet, false);
	viewToPaste.pasteIntoSelection();

	// check the first three rows
	QCOMPARE(col1->valueAt(0), 10);
	QCOMPARE(col2->valueAt(0), 10);
	QCOMPARE(col1->valueAt(1), 20);
	QCOMPARE(col2->valueAt(1), 20);
	QCOMPARE(col1->valueAt(2), 30);
	QCOMPARE(col2->valueAt(2), 30);

	// check the remaining rows
	for (int i = 3; i < rows; i++) {
		QCOMPARE(col1->valueAt(i), i + 1);
		QCOMPARE(col2->valueAt(i), i + 1);
	}
}

void SpreadsheetFormulaTest::formulaUpdateAfterRowRemoval() {
	INIT_SPREADSHEET2

	sheet.column(2)->setFormula(QLatin1String("x + y"), variableNames, variableColumns, true);
	sheet.column(2)->updateFormula();

	// check the initial values
	for (int i = 0; i < sheet.rowCount(); i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		QCOMPARE(sheet.column(1)->valueAt(i), 1);
		QCOMPARE(sheet.column(2)->valueAt(i), i + 2);
	}

	// select the rows from 10 to 15 and delete them
	view.setCellsSelected(9, 0, 14, 2, true);
	view.removeSelectedRows();

	// re-check the values in the calculated column
	QCOMPARE(sheet.rowCount(), 94);
	for (int i = 0; i < sheet.rowCount(); i++)
		QCOMPARE(sheet.column(2)->valueAt(i), sheet.column(0)->valueAt(i) + 1);
}

QTEST_MAIN(SpreadsheetFormulaTest)
