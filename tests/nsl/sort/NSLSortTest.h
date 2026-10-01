/*
	File                 : NSLSortTest.h
	Project              : LabPlot
	Description          : Tests for NSL sorting helpers
	--------------------------------------------------------------------
        SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLSORTTEST_H
#define NSLSORTTEST_H

#include "../NSLTest.h"

class NSLSortTest : public NSLTest {
	Q_OBJECT

private Q_SLOTS:
	void testSizeTOrdering();
};

#endif