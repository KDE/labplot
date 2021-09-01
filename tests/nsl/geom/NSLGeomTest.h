/*
    File                 : NSLGeomTest.h
    Project              : LabPlot
    Description          : NSL Tests for geometric functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLGEOMTEST_H
#define NSLGEOMTEST_H

#include "../NSLTest.h"

class NSLGeomTest : public NSLTest {
	Q_OBJECT

private slots:
	void initTestCase();

	void testDist();
	void testLineSim();
	void testLineSimMorse();
	// performance
	//void testPerformance();
private:
	QString m_dataDir;
};
#endif
