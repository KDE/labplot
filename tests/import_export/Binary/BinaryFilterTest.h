/*
    File                 : BinaryFilterTest.h
    Project              : LabPlot
    Description          : Tests for the binary filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BINARYFILTERTEST_H
#define BINARYFILTERTEST_H

#include "../../CommonTest.h"
#include <QtTest>

class BinaryFilterTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void importInt8();
	void importInt16BE();
	void importInt32LE();
	void importInt32BE();
	void importInt64BE();

	void importFloatBE();
	void importDoubleBE();
	void importDoubleLE();
	
	void importDoubleMatrixBE();
};
#endif
