/*
	File                 : NSLPeakTest.h
	Project              : LabPlot
	Description          : NSL Tests for peak functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLPEAKTEST_H
#define NSLPEAKTEST_H

#include "../NSLTest.h"

class NSLPeakTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void testPeakSimple();
	// performance
	// void testPerformance();
};
#endif
