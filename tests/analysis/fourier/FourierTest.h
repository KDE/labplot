/*
	File                 : FourierTest.h
	Project              : LabPlot
	Description          : Tests for fourier filtering
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FOURIERTEST_H
#define FOURIERTEST_H

#include <../AnalysisTest.h>

class FourierTest : public AnalysisTest {
	Q_OBJECT

private Q_SLOTS:
	void addCurve();
	void lowPassButterWorth();
};
#endif // FOURIERTEST_H
