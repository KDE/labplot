/*
    File                 : NSLDiffTest.h
    Project              : LabPlot
    Description          : NSL Tests for numerical differentiation
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLDIFFTEST_H
#define NSLDIFFTEST_H

#include "../NSLTest.h"

class NSLDiffTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	// first derivative
	void testFirst_order2();
	void testFirst_order4();
	void testFirst_avg();
	// second derivative
	void testSecond_order1();
	void testSecond_order2();
	void testSecond_order3();
	// higher derivative
	void testThird_order2();
	void testFourth_order1();
	void testFourth_order3();
	void testFifth_order2();
	void testSixth_order1();
	// performance
	void testPerformance_first();
	void testPerformance_second();
	void testPerformance_third();
private:
	QString m_dataDir;
};
#endif
