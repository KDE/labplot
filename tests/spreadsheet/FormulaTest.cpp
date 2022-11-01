/*
	File                 : FormulaTest.cpp
	Project              : LabPlot
	Description          : Tests for the Formula
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FormulaTest.h"
#include "backend/core/Project.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "kdefrontend/spreadsheet/FlattenColumnsDialog.h"

void FormulaTest::initTestCase() {
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
	QLocale::setDefault(QLocale(QLocale::C));
}


//**********************************************************
//********** Check different formulas ***********
//**********************************************************
/*!
   formula "1"
*/
void FormulaTest::formula1() {
	Spreadsheet sheet("test", false);
	const int cols = 2;
	const int rows = 100;

	sheet.setColumnCount(cols);
	sheet.setRowCount(rows);

	SpreadsheetView view(&sheet, false);
	view.selectColumn(0);
	view.fillWithRowNumbers();
	
	QStringList variableNames;
	variableNames << QLatin1String("x");
	QVector<Column*> variableColumns;
	variableColumns << sheet.column(0);
	sheet.column(1)->setFormulaVariableColumn(sheet.column(0));
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

QTEST_MAIN(FormulaTest)
