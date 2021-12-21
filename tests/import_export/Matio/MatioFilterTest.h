/*
    File                 : MatioFilterTest.h
    Project              : LabPlot
    Description          : Tests for the Matio I/O-filter.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MATIOFILTERTEST_H
#define MATIOFILTERTEST_H

#include "../../CommonTest.h"
#include <QtTest>

class MatioFilterTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void testImportDouble();
	void testImportSpreadsheet();
	void testImportSpreadsheetPortion();
	void testImportMatrix();

	void testImportSparse();
	void testImportLogicalSparse();
	void testImportLogicalSparsePortion();
	void testImportSparseComplex();
	void testImportStruct();
	void testImportStructPortion();
	void testImportCell();
	void testImportCellPortion();
	void testImportEmptyCell();

	void testImportMultipleVars();
};


#endif
