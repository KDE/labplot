/*
	File                 : CartesianCoordinateSystemTest.h
	Project              : LabPlot
	Description          : Tests for cartesian coordinate system
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANCOORDINATESYSTEMTEST_H
#define CARTESIANCOORDINATESYSTEMTEST_H

#include "../CommonTest.h"

class CartesianCoordinateSystemTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void testMapLogicalToSceneLines();
	void testMapLogicalToSceneLinesClipping();
	void testMapLogicalToSceneLinesPerformance();
};

#endif // CARTESIANCOORDINATESYSTEMTEST_H
