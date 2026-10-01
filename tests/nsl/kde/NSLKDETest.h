/*
	File                 : NSLKDETest.h
	Project              : LabPlot
	Description          : Tests for NSL kernel density estimation
	--------------------------------------------------------------------
        SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>
        
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLKDETEST_H
#define NSLKDETEST_H

#include "../NSLTest.h"

class NSLKDETest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void testDensityAndBandwidth();
};

#endif