/*
	File                 : DecompositionTest.h
	Project              : LabPlot
	Description          : Tests for time series decomposition
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DECOMPOSITIONTEST_H
#define DECOMPOSITIONTEST_H

#include "CommonTest.h"

class DecompositionTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void testDecompositionInit();
	void testDecompositionDuplicate();

	void testDecompositionMethodChange();
	void testDecompositionPeriodChange();

	void testDecompositionSaveLoadSTL();
	void testDecompositionSaveLoadMSTL();
};
#endif
