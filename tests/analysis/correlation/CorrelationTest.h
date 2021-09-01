/*
    File                 : CorrelationTest.h
    Project              : LabPlot
    Description          : Tests for data correlation
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef CORRELATIONTEST_H
#define CORRELATIONTEST_H

#include <../AnalysisTest.h>

class CorrelationTest : public AnalysisTest {
	Q_OBJECT

private slots:
	// linear tests
	void testLinear();
	void testLinear2();
	void testLinear_noX();
	void testLinear_swapped();

	// circular tests
	void testCircular();
	void testCircular2();

	// norm
	void testLinear_biased();
	void testLinear2_biased();
	void testLinear_unbiased();
	void testLinear2_unbiased();
	void testLinear_coeff();
	void testLinear2_coeff();
	void testCircular_coeff();
	void testCircular2_coeff();

	// sampling interval
	void testLinear_samplingInterval();
	void testLinear2_samplingInterval();
	void testCircular_samplingInterval();
	void testCircular2_samplingInterval();

	void testPerformance();
};
#endif
