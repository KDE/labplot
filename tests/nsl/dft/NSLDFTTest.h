/*
    File                 : NSLDFTTest.h
    Project              : LabPlot
    Description          : NSL Tests for DFT
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach (stefan.gerlach@uni.kn)

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLDFTTEST_H
#define NSLDFTTEST_H

#include "../NSLTest.h"

class NSLDFTTest : public NSLTest {
	Q_OBJECT

private slots:
	// one sided
	void testOnesided_real();
	void testOnesided_imag();
	void testOnesided_magnitude();
	void testOnesided_amplitude();
	void testOnesided_power();
	void testOnesided_phase();
	void testOnesided_dB();
	void testOnesided_squaremagnitude();
	void testOnesided_squareamplitude();
	void testOnesided_normdB();
	// two sided
	void testTwosided_real();
	void testTwosided_imag();
	void testTwosided_magnitude();
	void testTwosided_amplitude();
	void testTwosided_power();
	void testTwosided_phase();
	void testTwosided_dB();
	void testTwosided_squaremagnitude();
	void testTwosided_squareamplitude();
	void testTwosided_normdB();
	// performance
	void testPerformance_onesided();
	void testPerformance_twosided();
private:
	QString m_dataDir;
};
#endif
