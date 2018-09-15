/***************************************************************************
    File                 : ConvolutionTest.h
    Project              : LabPlot
    Description          : Tests for data convolution
    --------------------------------------------------------------------
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
#ifndef CONVOLUTIONTEST_H
#define CONVOLUTIONTEST_H

#include <QtTest>
#include <backend/lib/macros.h>	// DEBUG()

extern "C" {
#include <gsl/gsl_math.h>
}

class ConvolutionTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();
	// compare floats with given delta (could be useful for other tests too)
	// delta - relative error
	static inline void FuzzyCompare(double actual, double expected, double delta = 1.e-12) {
		if (fabs(expected) < delta)
			QVERIFY(fabs(actual) < delta);
		else {
			DEBUG(std::setprecision(15) << actual - fabs(actual)*delta << " <= " << expected << " <= " << actual + fabs(actual)*delta);
			QVERIFY(!gsl_fcmp(actual, expected, delta));
		}
	}

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
};
#endif
