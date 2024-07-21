/*
	File                 : XLSXFilterTest.h
	Project              : LabPlot
	Description          : Tests for the XLSX filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef XLSXFILTERTEST_H
#define XLSXFILTERTEST_H

#include "../../CommonMetaTest.h"

class XLSXFilterTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
	void importFile2Cols();
	void importFile3Cols();
	void importFile3ColsStartEndRow();
	void importFile3ColsStartEndColumn();
	void importFileEmptyCells();
	void importFileDatetime();
};
#endif
