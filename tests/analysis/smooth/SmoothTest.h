/*
    File                 : SmoothTest.h
    Project              : LabPlot
    Description          : Tests for data smoothing
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SMOOTHTEST_H
#define SMOOTHTEST_H

#include <../AnalysisTest.h>

class SmoothTest : public AnalysisTest {
	Q_OBJECT

private Q_SLOTS:
	void testPercentile();

//	void testPerformance();
};
#endif
