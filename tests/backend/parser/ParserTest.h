/*
    File                 : ParserTest.h
    Project              : LabPlot
    Description          : Tests for the Parser
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef PARSERTEST_H
#define PARSERTEST_H

#include "../../CommonTest.h"

class ParserTest : public CommonTest {
	Q_OBJECT

private slots:
	void testBasics();
	void testErrors();
	void testVariables();
	void testLocale();

	void testPerformance1();
	void testPerformance2();

};

#endif
