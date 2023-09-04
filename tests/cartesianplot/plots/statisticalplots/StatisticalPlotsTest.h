/*
	File                 : BarPlotTest.h
	Project              : LabPlot
	Description          : Tests for statistical plots like Q-Q plot, KDE plot, etc.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATISTICALPLOTSTEST_H
#define STATISTICALPLOTSTEST_H

#include "CommonTest.h"

class StatisticalPlotsTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	// KDE plot
	void testKDEPlotInit();

	// Q-Q plot
	void testQQPlotInit();
	void testQQPlotRange();
};

#endif
