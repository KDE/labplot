/*
	File                 : MatrixFormulaTest.cpp
	Project              : LabPlot
	Description          : Tests for formula in matrix
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MatrixFormulaTest.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"

#define INIT_MATRIX Matrix m(QStringLiteral("test matrix"), false);

//**********************************************************
//********** Check different formulas **********************
//**********************************************************
/*!
   formula "1"
*/
void MatrixFormulaTest::formula1(){
	INIT_MATRIX

	// m.setFormula(QLatin1String("1"), variableNames, variableColumns, true);
	// m.updateFormula();

	// QCOMPARE(m.columnCount(), cols);
	// QCOMPARE(m.rowCount(), rows);

	// values
	/*for (int i = 0; i < rows; i++) {
	for (int j = 0; j < cols; j++) {
		QCOMPARE(m[row][col]->valu11);
	}
	}*/
} QTEST_MAIN(MatrixFormulaTest)
