/*
	File                 : NSLPCMTest.h
	Project              : LabPlot
	Description          : Tests for NSL process-control factors
	--------------------------------------------------------------------
        SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>
        
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLPCMTEST_H
#define NSLPCMTEST_H

#include "../NSLTest.h"

class NSLPCMTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void testFactors();
};

#endif