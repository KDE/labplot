/*
	File                 : MatrixTest.h
	Project              : LabPlot
	Description          : Tests for the Matrix
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MATRIXTEST_H
#define MATRIXTEST_H

#include "../CommonTest.h"

class MatrixTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:

	void testLoadSaveNoData();
	void testLoadSaveWithData();

	// TODO: see Spreadsheet for things to test
};

#endif
