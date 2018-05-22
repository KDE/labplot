/***************************************************************************
    File                 : FitTest.h
    Project              : LabPlot
    Description          : Tests for data fitting
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <backend/lib/macros.h>	// DEBUG()

extern "C" {
#include <gsl/gsl_math.h>
}

class FitTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();
	// compare floats with given delta (could be useful for other tests too)
	// delta - relative error (set to 1. if expected == 0.)
	static inline void FuzzyCompare(double actual, double expected, double delta = 1.e-12) {
		DEBUG(std::setprecision(15) << actual - fabs(actual)*delta << " <= " << expected << " <= " << actual + fabs(actual)*delta);
		QVERIFY(!gsl_fcmp(actual, expected, delta));
	}

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
};
