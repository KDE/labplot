/*
    File                 : ConvolutionTest.h
    Project              : LabPlot
    Description          : Tests for data convolution
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef CONVOLUTIONTEST_H
#define CONVOLUTIONTEST_H

#include <../AnalysisTest.h>

class ConvolutionTest : public AnalysisTest {
	Q_OBJECT

private slots:
	// linear tests
	void testLinear();
	void testLinear2();
	void testLinear_noX();
	void testLinear_swapped();
	void testLinear_swapped_noX();
	void testLinear_norm();
	void testLinear_swapped_norm();
	void testLinear_wrapMax();
	void testLinear_swapped_wrapMax();
	void testLinear_wrapCenter();
	void testLinear_swapped_wrapCenter();

	// circular tests
	void testCircular();
	void testCircular2();
	void testCircular_noX();
	void testCircular_swapped();
	void testCircular_swapped_noX();
	void testCircular_norm();
	void testCircular_swapped_norm();
	void testCircular_wrapMax();
	void testCircular_swapped_wrapMax();
	void testCircular_wrapCenter();
	void testCircular_swapped_wrapCenter();

	// deconvolution tests
	void testLinearDeconv();
	void testLinearDeconv2();
	void testLinearDeconv_swapped();
	void testLinearDeconv2_swapped();
	void testLinearDeconv_norm();
	void testCircularDeconv();
	void testCircularDeconv2();
	void testCircularDeconv_norm();

	void testPerformance();
};
#endif
