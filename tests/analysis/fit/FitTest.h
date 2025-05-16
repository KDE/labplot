/*
	File                 : FitTest.h
	Project              : LabPlot
	Description          : Tests for data fitting
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2018-2025 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FITTEST_H
#define FITTEST_H

#include <../AnalysisTest.h>

class FitTest : public AnalysisTest {
	Q_OBJECT

private:
	void printAndCheck(double value, double exact, double tol);

private Q_SLOTS:
	void addCurve();

	// linear regression (see NIST/linear data)
	void testLinearNorris();
	void testLinearPontius();
	void testLinearNoInt1(); // using custom model
	void testLinearNoInt1_2(); // using polynomial model with fixed parameter
	void testLinearNoInt2(); // using custom model
	void testLinearNoInt2_2(); // using polynomial model with fixed parameter
	void testLinearFilip();

	void testLinearWampler1();
	void testLinearWampler1_custom(); // using custom model
	void testLinearWampler2();
	void testLinearWampler2_custom(); // using custom model
	void testLinearWampler3();
	void testLinearWampler3_custom(); // using custom model
	void testLinearWampler4();
	void testLinearWampler4_custom(); // using custom model
	void testLinearWampler5();
	void testLinearWampler5_custom(); // using custom model

	void testLinearWP_OLS();
	void testLinearR_lm2();

	// non-linear regression
	void testNonLinearMisra1a(); // first set of start values
	void testNonLinearMisra1a_2(); // second set of start values
	void testNonLinearMisra1a_3(); // third set of start values
	void testNonLinearMisra1b(); // first set of start values
	void testNonLinearMisra1b_2(); // second set of start values
	void testNonLinearMisra1b_3(); // third set of start values
	void testNonLinearMisra1c(); // first set of start values
	void testNonLinearMisra1c_2(); // second set of start values
	void testNonLinearMisra1c_3(); // third set of start values
	void testNonLinearMisra1d(); // first set of start values
	void testNonLinearMisra1d_2(); // second set of start values
	void testNonLinearMisra1d_3(); // third set of start values
	void testNonLinearMGH09(); // first set of start values
	void testNonLinearMGH09_2(); // second set of start values
	void testNonLinearMGH09_3(); // exact start values
	void testNonLinearMGH10(); // first set of start values
	void testNonLinearMGH10_2(); // second set of start values
	void testNonLinearMGH10_3(); // third set of start values
	void testNonLinearRat43(); // first set of start values
	void testNonLinearRat43_2(); // second set of start values
	void testNonLinearRat43_3(); // third set of start values
	// more non-linear fits
	void testNonLinearHahn1(); // first set of start values
	void testNonLinearHahn1_2(); // second set of start values
	void testNonLinearHahn1_3(); // exact start values
	void testNonLinearBennett5(); // first set of start values
	void testNonLinearBennett5_2(); // second set of start values
	void testNonLinearBennett5_3(); // exact start values

	void testNonLinearMichaelis_Menten();

	// fits with weights
	void testNonLinearGP_lcdemo();
	void testLinearGP_PY_noerror();
	void testLinearGP_PY_yerror_polynomial();
	void testLinearGP_PY_yerror_custom();
	void testLinearGP_PY_xyerror_polynomial();
	void testLinearGP_PY_xyerror_custom();
	void testLinearGP_PY_xyerror_custom_instrumental_weight();
	void testLinearGP_PY_xyerror_custom_inverse_weight();

	void testNonLinear_yerror_zero_bug408535();

	// histogram fit
	void testHistogramFit();
	void testHistogramGaussianML();
	void testHistogramExponentialML();
	void testHistogramLaplaceML();
	void testHistogramCauchyML();
	void testHistogramLognormalML();
	void testHistogramPoissonML();
	void testHistogramBinomialML();
};
#endif
