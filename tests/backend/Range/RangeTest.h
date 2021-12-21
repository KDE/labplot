/*
    File                 : RangeTest.h
    Project              : LabPlot
    Description          : Tests for Range
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RANGETEST_H
#define RANGETEST_H

#include "../../CommonTest.h"

class RangeTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void testNiceExtend();
	void testTickCount();

//	void testPerformance();

};

#endif
