/*
    File                 : NSLFilterTest.h
    Project              : LabPlot
    Description          : NSL Tests for filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach (stefan.gerlach@uni.kn)

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLFILTERTEST_H
#define NSLFILTERTEST_H

#include "../NSLTest.h"

class NSLFilterTest : public NSLTest {
	Q_OBJECT

private slots:
	void initTestCase();

	void testForm();
	// performance
	//void testPerformance();
private:
	QString m_dataDir;
};
#endif
