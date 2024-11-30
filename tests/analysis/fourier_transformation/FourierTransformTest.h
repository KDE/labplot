/*
	File                 : FourierTransformTest.h
	Project              : LabPlot
	Description          : Tests for discrete Fourier transformation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FOURIERTRANSFORMTEST_H
#define FOURIERTRANSFORMTEST_H

#include <../AnalysisTest.h>

class FourierTransformTest : public AnalysisTest {
	Q_OBJECT

private Q_SLOTS:
	void fft();
};
#endif
