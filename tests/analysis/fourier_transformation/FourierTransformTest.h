/*
	File                 : DifferentiationTest.h
	Project              : LabPlot
	Description          : Tests for numerical differentiation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FOURIERTRANSFORM_TEST
#define FOURIERTRANSFORM_TEST

#include <../AnalysisTest.h>

class FourierTransformTest : public AnalysisTest {
	Q_OBJECT

private Q_SLOTS:
	void fft();
};
#endif // FOURIERTRANSFORM_TEST
