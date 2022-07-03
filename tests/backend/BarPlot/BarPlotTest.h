/*
	File                 : BarPlotTest.h
	Project              : LabPlot
	Description          : Tests for BarPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BARPLOTTEST_H
#define BARPLOTTEST_H

#include "../../CommonTest.h"

class BarPlotTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void testRange01();
	void testRange02();
};

#endif
