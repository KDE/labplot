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

class FitTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();
	// compare floats with given delta (could be useful for other tests too)
	// delta - relative error (set to 1. if expected == 0.)
	static inline void FuzzyCompare(double actual, double expected, double delta = 1.e-12) {
		qDebug() <<  qSetRealNumberPrecision(15) << actual - fabs(actual)*delta << "<=" << expected << "<=" << actual + fabs(actual)*delta;
		QVERIFY(actual - fabs(actual)*delta <= expected && actual + fabs(actual)*delta >=expected);
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

	//non-linear regression
	void testNonLinearMisra1a();
	void testNonLinearMisra1b();

	//fits with weights
};
