/*
	File                 : BarPlotTest.h
	Project              : LabPlot
	Description          : Tests for BarPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HISTOGRAMTEST_H
#define HISTOGRAMTEST_H

#include "../../CommonTest.h"

class HistogramTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void testColumnRemoved();
};

#endif // HISTOGRAMTEST_H
