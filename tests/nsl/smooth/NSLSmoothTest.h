/*
    File                 : NSLDiffTest.h
    Project              : LabPlot
    Description          : NSL Tests for smoothing
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach (stefan.gerlach@uni.kn)

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef NSLSMOOTHTEST_H
#define NSLSMOOTHTEST_H

#include "../NSLTest.h"

class NSLSmoothTest : public NSLTest {
	Q_OBJECT

private slots:
	// moving average tests
	void testMA_padnone();
	void testMA_padmirror();
	void testMA_padnearest();
	void testMA_padconstant();
	void testMA_padperiodic();
	// lagged moving average tests
	void testMAL_padnone();
	void testMAL_padmirror();
	void testMAL_padnearest();
	void testMAL_padconstant();
	void testMAL_padperiodic();
	// percentile tests
	void testPercentile_padnone();
	void testPercentile_padmirror();
	void testPercentile_padnearest();
	void testPercentile_padconstant();
	void testPercentile_padperiodic();
	// Savivitzky-Golay coeff tests
	void testSG_coeff31();
	void testSG_coeff51();
	void testSG_coeff53();
	void testSG_coeff73();
	void testSG_coeff74();
	void testSG_coeff92();
	void testSG_coeff94();

	// Savivitzky-Golay modes
	void testSG_mode_interp();
	void testSG_mode_mirror();
	void testSG_mode_nearest();
	void testSG_mode_constant();
	void testSG_mode_periodic();

	// performance
	void testPerformance_interp();
	void testPerformance_mirror();
	void testPerformance_nearest();
	void testPerformance_constant();
	void testPerformance_periodic();
private:
	QString m_dataDir;
};
#endif
