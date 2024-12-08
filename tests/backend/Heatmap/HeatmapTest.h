/*
	File                 : HeatmapTest.h
	Project              : LabPlot
	Description          : Tests for HeatmapTest
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HEATMAPTEST_H
#define HEATMAPTEST_H

#include "../../CommonTest.h"

class HeatmapTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void testSetMatrix();
	void testSetSpreadsheetColumn();
	void testSetDataSource();

	void testNumberBins();

	void testFormat();

	void testRepresentationMatrix();
	void testRepresentationSpreadsheet();
	void testRepresentationSpreadsheetDrawEmpty();
	void testRepresentationSpreadsheetDrawZeroes();

	void testColorAutomatic();
	void testColorManual();

	void saveLoad();

	void minMaxMatrix();

	void indicesMinMaxMatrix();

	void testMatrixNumBins();
};

#endif // HEATMAPTEST_H
