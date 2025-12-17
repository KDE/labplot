/*
File                 : SpreadsheetGenerateDataTest.h
Project              : LabPlot
Description          : Tests for the generation of data in spreadsheet
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPREADSHEETGENERATEDATATEST_H
#define SPREADSHEETGENERATEDATATEST_H

#include "../CommonTest.h"

class SpreadsheetGenerateDataTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase() override;

	// generation of equidistant values

	// fixed number of values
	void testFixedNumberDouble();
	void testFixedNumberInt();
	void testFixedNumberBigInt();
	void testFixedNumberDateTime();
	void testFixedNumberDoubleDateTime();

	// fixed increment between the values
	void testFixedIncrementDouble();
	void testFixedIncrementInt();
	void testFixedIncrementBigInt();
	void testFixedIncrementDateTime();

	// fixed number of values and increment
	void testFixedNumberIncrementDouble();
	void testFixedNumberIncrementInt();
	void testFixedNumberIncrementBigInt();
	void testFixedNumberIncrementDateTime();

	// column mode conversion
	void testFixedNumberIntToBigInt();
	void testFixedNumberIntToDouble();
	void testFixedNumberBigIntToDouble();
};

#endif
