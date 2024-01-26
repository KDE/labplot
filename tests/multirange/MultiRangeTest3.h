/*
	File                 : MultiRangeTest3.h
	Project              : LabPlot
	Description          : Third tests for project imports
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MULTIRANGETEST3_H
#define MULTIRANGETEST3_H

#include "tests/CommonTest.h"

class MultiRangeTest3 : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void autoScaleYAfterZoomInX();
	void autoScaleXAfterZoomInY();

	void baseDockSetAspects_NoPlotRangeChange();

	void curveRangeChange();
};
#endif
