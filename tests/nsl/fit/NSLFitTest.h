/*
    File                 : NSLFitTest.h
    Project              : LabPlot
    Description          : NSL Tests for fitting
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLFITTEST_H
#define NSLFITTEST_H

#include "../NSLTest.h"

class NSLFitTest : public NSLTest {
	Q_OBJECT

private slots:
	void testBounds();
	// performance
	//void testPerformance();
private:
	QString m_dataDir;
};
#endif
