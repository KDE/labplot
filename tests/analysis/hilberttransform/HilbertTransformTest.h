/*
    File                 : HilbertTransformTest.h
    Project              : LabPlot
    Description          : Tests for Hilbert transform
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef HILBERTTRANSFORMTEST_H
#define HILBERTTRANSFORMTEST_H

#include <../AnalysisTest.h>

class HilbertTransformTest : public AnalysisTest {
	Q_OBJECT

private slots:
	void test1();
	void test2();
	void test3();
	void test4();

	void testPerformance();
};
#endif
