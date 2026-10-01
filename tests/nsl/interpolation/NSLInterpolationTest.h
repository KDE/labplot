/*
	File                 : NSLInterpolationTest.h
	Project              : LabPlot
	Description          : Tests for NSL rational interpolation
	--------------------------------------------------------------------
		SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLINTERPOLATIONTEST_H
#define NSLINTERPOLATIONTEST_H

#include "../NSLTest.h"

class NSLInterpolationTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void testRationalInterpolation();
};

#endif