/*
    File                 : NSLStatsTest.h
    Project              : LabPlot
    Description          : NSL Tests for statistical functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLSTATSTEST_H
#define NSLSTATSTEST_H

#include "../NSLTest.h"

class NSLStatsTest : public NSLTest {
	Q_OBJECT

private slots:
	void testQuantile();
	// performance
	//void testPerformance();
private:
	QString m_dataDir;
};
#endif
