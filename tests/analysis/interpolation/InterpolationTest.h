/*
	File                 : InterpolationTest.h
	Project              : LabPlot
	Description          : Tests for numerical interpolation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef INTERPOLATIONTEST_H
#define INTERPOLATIONTEST_H

#include <../AnalysisTest.h>

class InterpolationTest : public AnalysisTest {
	Q_OBJECT

private Q_SLOTS:
	void testRanges();
};
#endif
