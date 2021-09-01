/*
    File                 : FitTest.h
    Project              : LabPlot
    Description          : Tests for data fitting
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FITTEST_H
#define FITTEST_H

#include <../AnalysisTest.h>

class FitTest : public AnalysisTest {
	Q_OBJECT

private slots:
	//linear regression (see NIST/linear data)
	void testLinearNorris();
	void testLinearPontius();
	void testLinearNoInt1();	// using custom model
	void testLinearNoInt1_2();	// using polynomial model with fixed parameter
	void testLinearNoInt2();	// using custom model
	void testLinearNoInt2_2();	// using polynomial model with fixed parameter
	void testLinearFilip();

	void testLinearWampler1();
	void testLinearWampler2();
	void testLinearWampler3();
	void testLinearWampler4();
	void testLinearWampler5();

	void testLinearWP_OLS();
	void testLinearR_lm2();

	//non-linear regression
	void testNonLinearMisra1a();
	void testNonLinearMisra1a_2();	// second set of start values
	void testNonLinearMisra1a_3();	// third set of start values
	void testNonLinearMisra1b();
	void testNonLinearMisra1b_2();	// second set of start values
	void testNonLinearMisra1b_3();	// third set of start values
	void testNonLinearMisra1c();
	void testNonLinearMisra1c_2();	// second set of start values
	void testNonLinearMisra1c_3();	// third set of start values
	void testNonLinearMisra1d();
	void testNonLinearMisra1d_2();	// second set of start values
	void testNonLinearMisra1d_3();	// third set of start values
	void testNonLinearMGH09();
	void testNonLinearMGH09_2();	// second set of start values
	void testNonLinearMGH09_3();	// third set of start values
	void testNonLinearMGH10();
	void testNonLinearMGH10_2();	// second set of start values
	void testNonLinearMGH10_3();	// third set of start values
	void testNonLinearRat43();
	void testNonLinearRat43_2();	// second set of start values
	void testNonLinearRat43_3();	// third set of start values

	void testNonLinearMichaelis_Menten();

	//fits with weights
	void testNonLinearGP_lcdemo();
	void testLinearGP_PY_noerror();
	void testLinearGP_PY_yerror_polynomial();
	void testLinearGP_PY_yerror_custom();
	void testLinearGP_PY_xyerror_polynomial();
	void testLinearGP_PY_xyerror_custom();
	void testLinearGP_PY_xyerror_custom_instrumental_weight();
	void testLinearGP_PY_xyerror_custom_inverse_weight();

	void testNonLinear_yerror_zero_bug408535();
};
#endif
