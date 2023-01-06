/*
	File                 : NSLBaselineTest.h
	Project              : LabPlot
	Description          : NSL Tests for baseline functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLBASELINETEST_H
#define NSLBASELINETEST_H

#include "../NSLTest.h"

class NSLBaselineTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void testBaselineMinimum();
	void testBaselineMinimum2();
	void testBaselineMaximum();
	void testBaselineMaximum2();
	void testBaselineMean();
	void testBaselineMean2();
	void testBaselineMedian();
	void testBaselineMedian2();
	// performance
	// void testPerformance();
};
#endif
