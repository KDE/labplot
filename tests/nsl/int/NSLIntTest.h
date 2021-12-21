/*
    File                 : NSLIntTest.h
    Project              : LabPlot
    Description          : NSL Tests for numerical integration
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLINTTEST_H
#define NSLINTTEST_H

#include "../NSLTest.h"

class NSLIntTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	// rules integral/area
	void testRectangle_integral();
	void testRectangle_area();
	void testTrapezoid_integral();
	void testTrapezoid_area();
	void test3Point_integral();
	void test4Point_integral();
	// performance
	void testPerformanceRectangle();
	void testPerformanceTrapezoid();
	void testPerformance3Point();
	void testPerformance4Point();
private:
	QString m_dataDir;
};
#endif
