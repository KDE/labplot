/*
    File                 : NSLSFBasicTest.h
    Project              : LabPlot
    Description          : NSL Tests for the basic special functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach (stefan.gerlach@uni.kn)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef NSLSFBASICTEST_H
#define NSLSFBASICTEST_H

#include "../NSLTest.h"

class NSLSFBasicTest : public NSLTest {
	Q_OBJECT

private slots:
	// log2
	void testlog2_int_C99();
	void testlog2_int();
	void testlog2_longlong();
	void testlog2_int2();
	void testlog2_int3();
	void testlog2p1_int();
private:
	QString m_dataDir;
};
#endif
