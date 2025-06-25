/*
	File                 : MultiRangeTest3.h
	Project              : LabPlot
	Description          : Tests for multi ranges, part 3
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
	void baseDockSetAspects_NoPlotRangeChange();

	void curveRangeChange();
	void loadLegacyProject();
};
#endif
