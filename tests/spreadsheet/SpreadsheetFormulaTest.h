/*
	File                 : SpreadsheetFormulaTest.h
	Project              : LabPlot
	Description          : Tests for formula in spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPREADSHEETFORMULATEST_H
#define SPREADSHEETFORMULATEST_H

#include "../CommonTest.h"

class SpreadsheetFormulaTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void formula1();
	void formulax();
	void formulaxp1();

	void formulaCell1();
	void formulaCelli();
	void formulaCellip1();
	void formulaCellim1();
	void formulaCell2i();
	void formulaCellip1im1();
	void formulaCellsqrtip1();

	void formulaCell1_2x();
	void formulaCelli_2x();
	void formulaCelli_xpx();
	void formulaCelli_xp2x();
	void formulaCelli_sqrtx();
	void formulaCelli_xpy();

	void formulaCell2i_xpy();
	void formulaCelli_2xpCelli_2y();

	void formulaLocale();

	// moving statistics
	void formulama();
	void formulamr();
	void formulasma();
	void formulasmr();

	// check updates of columns defined via a formula on changes
	void formulaUpdateAfterCellChange();
	void formulaUpdateAfterPaste();
};

#endif
