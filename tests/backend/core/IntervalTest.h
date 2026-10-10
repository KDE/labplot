/*
	File                 : IntervalTest.h
	Project              : LabPlot
	Description          : Tests for Interval
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 LabPlot contributors
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERVALTEST_H
#define INTERVALTEST_H

#include "../../CommonMetaTest.h"

class IntervalTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
	void testConstructionAndValidity();
	void testContainmentAndIntersection();
	void testTouchAndMerge();
	void testSubtractAndSplit();
	void testIntervalLists();
};

#endif
