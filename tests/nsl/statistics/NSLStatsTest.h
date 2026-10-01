/*
	File                 : NSLStatsTest.h
	Project              : LabPlot
	Description          : NSL Tests for statistical functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLSTATSTEST_H
#define NSLSTATSTEST_H

#include "../NSLTest.h"

class NSLStatsTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void testQuantile();
	void testNaNInfHandling();
	void testRemainingStatistics();
	// performance
	// void testPerformance();
};
#endif
