/*
	File                 : FormulaTest.h
	Project              : LabPlot
	Description          : Tests for formula in spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FORMULATEST_H
#define FORMULATEST_H

#include "../CommonTest.h"

class FormulaTest : public CommonTest {
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

	// moving statistics
	void formulama();
	void formulamr();
	void formulasma();
	void formulasmr();
};

#endif
