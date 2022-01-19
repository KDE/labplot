/*
    File                 : MultiRangeTest.h
    Project              : LabPlot
    Description          : Tests for project imports
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MULTIRANGETEST_H
#define MULTIRANGETEST_H

#include <QtTest>
#include "tests/CommonTest.h"

class MultiRangeTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void applyActionToSelection_CurveSelected_ZoomSelection();

	// zoom selection tests
	void zoomXSelection_AllRanges();
	void zoomXSelection_SingleRange();
	void zoomYSelection_AllRanges();
	void zoomYSelection_SingleRange();
	void zoomSelection_AllRanges();
	void zoomSelection_SingleRange();

	// zoom tests (including auto scale)
	void zoomInX_SingleRange();
	void zoomInX_AllRanges();
	void zoomInY_SingleRange();
	void zoomInY_AllRanges();
	void zoomOutX_SingleRange();
	void zoomOutX_AllRanges();
	void zoomOutY_SingleRange();
	void zoomOutY_AllRanges();

	// shift tests (including auto scale)
	void shiftLeft_SingleRange();
	void shiftLeft_AllRanges();
	void shiftRight_SingleRange();
	void shiftRight_AllRanges();
	void shiftUp_SingleRange();
	void shiftDown_SingleRange();
	void shiftUp_AllRanges();
	void shiftDown_AllRanges();
};
#endif
