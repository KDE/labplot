/*
	File                 : NSLStatsTest.h
	Project              : LabPlot
	Description          : NSL Tests for statistical functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Kuntal Bar <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>

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

	void testMannKendall01();
	void testMannKendall02();
	void testMannKendall03();
	void testMannKendall04();
	void testMannKendall05();

	void testWaldWolfowitzRuns01();
	void testWaldWolfowitzRuns02();
	void testWaldWolfowitzRuns03();
};
#endif // NSLSTATISTICALTESTTEST_H
