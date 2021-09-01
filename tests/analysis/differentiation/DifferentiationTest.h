/*
    File                 : DifferentiationTest.h
    Project              : LabPlot
    Description          : Tests for numerical differentiation
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DIFFERENTIATIONTEST_H
#define DIFFERENTIATIONTEST_H

#include <../AnalysisTest.h>

class DifferentiationTest : public AnalysisTest {
	Q_OBJECT

private slots:
	void testLinear();
	void testLinearNonEquidistant();
	void testQuadratic();
	void testQuadraticNonEquidistant();

	// higher order
	void testQuadraticSecondOrder();
	void testCubicSecondOrder();
	void testCubicThirdOrder();

//	void testPerformance();
};
#endif
