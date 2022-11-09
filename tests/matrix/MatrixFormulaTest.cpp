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

#define INIT_MATRIX                                                                                                                                            \
	Matrix m(QStringLiteral("test matrix"), false);                                                                                                            \
	QStringList variableNames;                                                                                                                                 \
	variableNames << QStringLiteral("x") << QStringLiteral("y");

//**********************************************************
//********** Check different formulas **********************
//**********************************************************
/*!
   formula "1"
*/
void MatrixFormulaTest::formula1() {
	INIT_MATRIX

	m.setFormula(QStringLiteral("1"));
	// TODO: currently only via MatrixFunctionDialog
	// m.updateFormula();

	const int rows = 10;
	const int cols = 10;

	QCOMPARE(m.columnCount(), cols);
	QCOMPARE(m.rowCount(), rows);

	// values
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			QCOMPARE(m.cell<double>(i, j), 0);
		}
	}
}
QTEST_MAIN(MatrixFormulaTest)
