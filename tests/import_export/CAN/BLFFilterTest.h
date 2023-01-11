/*
	File                 : BLFFilterTest.h
	Project              : LabPlot
	Description          : Tests for the BLF filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BLFFILTERTEST_H
#define BLFFILTERTEST_H

#include "../../CommonTest.h"
#include <QtTest>

class BLFFilterTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
    void test();
private:
};
#endif // BLFFILTERTEST_H
