/*
	File                 : BarPlotTest.h
	Project              : LabPlot
	Description          : Tests for BarPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BARPLOTTEST_H
#define BARPLOTTEST_H

#include "../../CommonTest.h"

class BarPlotTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	// bar plot
	void testRange01();
	void testRange02();
	void testRange03();
	void testRange04();
	void testRange05();

	// lollipop plot
	void testRangeLollipopPlot01();
	void testRangeLollipopPlot02();
};

#endif
