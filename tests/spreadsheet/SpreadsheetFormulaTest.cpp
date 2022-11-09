/*
	File                 : SpreadsheetFormulaTest.cpp
	Project              : LabPlot
	Description          : Tests for formula in spreadsheets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetFormulaTest.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"

#define INIT_SPREADSHEET                                                                                                                                       \
	Spreadsheet sheet(QStringLiteral("test"), false);                                                                                                          \
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

	sheet.column(1)->setFormula(QLatin1String("cell(1, x)"), variableNames, variableColumns, true);
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

	sheet.column(1)->setFormula(QLatin1String("cell(i, x)"), variableNames, variableColumns, true);
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

	sheet.column(1)->setFormula(QLatin1String("cell(i + 1, x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i < rows - 1)
			QCOMPARE(sheet.column(1)->valueAt(i), i + 2);
		else
			QCOMPARE(sheet.column(1)->valueAt(i), qQNaN());
	}
}
/*!
   formula "cell(i - 1, x)"
*/
void SpreadsheetFormulaTest::formulaCellim1() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("cell(i - 1, x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i > 0)
			QCOMPARE(sheet.column(1)->valueAt(i), i);
		else
			QCOMPARE(sheet.column(1)->valueAt(i), qQNaN());
	}
}
/*!
   formula "cell(2*i, x)"
*/
void SpreadsheetFormulaTest::formulaCell2i() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("cell(2*i, x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i < rows / 2)
			QCOMPARE(sheet.column(1)->valueAt(i), 2 * (i + 1));
		else
			QCOMPARE(sheet.column(1)->valueAt(i), qQNaN());
	}
}
/*!
   formula "cell(i+1, x) - cell(i-1, x)"
*/
void SpreadsheetFormulaTest::formulaCellip1im1() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("cell(i+1, x) - cell(i-1, x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i > 0 && i < rows - 1)
			QCOMPARE(sheet.column(1)->valueAt(i), 2);
		else
			QCOMPARE(sheet.column(1)->valueAt(i), qQNaN());
	}
}
/*!
   formula "sqrt(cell(i+1, x))"
*/
void SpreadsheetFormulaTest::formulaCellsqrtip1() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("sqrt(cell(i+1, x))"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		if (i < rows - 1)
			QCOMPARE(sheet.column(1)->valueAt(i), std::sqrt(i + 2));
		else
			QCOMPARE(sheet.column(1)->valueAt(i), qQNaN());
	}
}

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
			QCOMPARE(sheet.column(1)->valueAt(i), qQNaN());
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
			QCOMPARE(sheet.column(1)->valueAt(i), qQNaN());
	}
}
/*!
   formula "sma(n, x)"
*/
void SpreadsheetFormulaTest::formulasma() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("sma(4, x)"), variableNames, variableColumns, true);
	sheet.column(1)->updateFormula();

	// values
	const int N = 4;
	for (int i = 0; i < rows; i++) {
		QCOMPARE(sheet.column(0)->valueAt(i), i + 1);
		double value = 0.;
		for (int index = qMax(0, i - N + 1); index <= i; index++)
			value += sheet.column(0)->valueAt(index);
		QCOMPARE(sheet.column(1)->valueAt(i), value / N);
	}
}
/*!
   formula "smr(n, x)"
*/
void SpreadsheetFormulaTest::formulasmr() {
	INIT_SPREADSHEET

	sheet.column(1)->setFormula(QLatin1String("smr(4, x)"), variableNames, variableColumns, true);
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

QTEST_MAIN(SpreadsheetFormulaTest)
