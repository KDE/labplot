/*
	File                 : NSLStatsTest.h
	Project              : LabPlot
	Description          : NSL Tests for statistical functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Kuntal Bar <stefan.gerlach@uni.kn>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSLSTATISTICALTESTTEST_H
#define NSLSTATISTICALTESTTEST_H
#include "../NSLTest.h"

class NSLStatisticalTestTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void testMannWhitney();
	void testAnovaOneWay();
	void testKruskalWallis();
	void testLogRankTest();
	void testIndependentT();
	void testOneSampleT();
	void testAnovaOneWayRepeated();
	void testWelchT();
	void testWilcoxon();
	void testFriedman();
	void testChisqIndependence();
	void testChisqGoodnessOfFit();
};
#endif // NSLSTATISTICALTESTTEST_H
