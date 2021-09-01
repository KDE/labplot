/*
    File                 : IntegrationTest.h
    Project              : LabPlot
    Description          : Tests for numerical integration
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef INTEGRATIONTEST_H
#define INTEGRATIONTEST_H

#include <../AnalysisTest.h>

class IntegrationTest : public AnalysisTest {
	Q_OBJECT

private slots:
	void testLinear();

//	void testPerformance();
};
#endif
