/*
    File                 : NSLSFWindowTest.h
    Project              : LabPlot
    Description          : NSL Tests for special window functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLSFWINDOWTEST_H
#define NSLSFWINDOWTEST_H

#include "../NSLTest.h"

class NSLSFWindowTest : public NSLTest {
	Q_OBJECT

private slots:
	void testWindowTypes();

	void testPerformance_triangle();
	void testPerformance_welch();
	void testPerformance_flat_top();
private:
	QString m_dataDir;
};
#endif
