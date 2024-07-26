/*
	File                 : OdsFilterTest.h
	Project              : LabPlot
	Description          : Tests for the Ods filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef ODSFILTERTEST_H
#define ODSFILTERTEST_H

#include "../../CommonMetaTest.h"

class OdsFilterTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
	void importFile3SheetsRangesFormula(); // check types, ranges and formula
	void importFile3SheetsWorkbook(); // check workbook import
	void importFileMatrix(); // check import to matrix
	void importFileSheetStartEndRow(); // check giving start and end row
	void importFileSheetStartEndColumn(); // check giving start and end column
	void importFileSheetWithHeader(); // check importing header from first row
};
#endif
