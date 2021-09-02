/*
    File                 : ReadStatFilterTest.h
    Project              : LabPlot
    Description          : Tests for the ReadStat I/O-filter.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef READSTATFILTERTEST_H
#define READSTATFILTERTEST_H

#include "../../CommonTest.h"
#include <QtTest>

class ReadStatFilterTest : public CommonTest {
	Q_OBJECT

private slots:
	void testDTAImport();
	void testSASImport();
	void testSAVImport();
	void testPORImport();
	void testXPTImport();
};


#endif
