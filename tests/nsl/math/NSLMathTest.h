/*
	File                 : NSLMathTest.h
	Project              : LabPlot
	Description          : NSL Tests for math functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLMATHTEST_H
#define NSLMATHTEST_H

#include "../NSLTest.h"

class NSLMathTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void mathMultiple();
	void truncPerformanceComparsion();
	// performance
	// void testPerformance();
};
#endif
