/*
	File                 : NSLSignalTest.h
	Project              : LabPlot
	Description          : Tests for NSL signal processing algorithms
	--------------------------------------------------------------------
        SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>
        
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLSIGNALTEST_H
#define NSLSIGNALTEST_H

#include "../NSLTest.h"

class NSLSignalTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void testChangePoints();
	void testConvolution();
	void testCorrelation();
	void testHilbertTransform();
};

#endif