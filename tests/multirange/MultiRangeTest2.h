/*
	File                 : MultiRangeTest2.h
	Project              : LabPlot
	Description          : Tests for multi ranges, part 2
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MULTIRANGETEST2_H
#define MULTIRANGETEST2_H

#include "tests/CommonTest.h"

class MultiRangeTest2 : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void autoScaleYAfterZoomInX();
	void autoScaleXAfterZoomInY();

	void mouseWheelXAxisApplyToAllX();
	void mouseWheelTanCurveApplyToAllX();
	void mouseWheelXAxisApplyToSelected();

	void axisMouseMoveApplyToAllX();
	void axisMouseMoveApplyToSelection();
};
#endif
