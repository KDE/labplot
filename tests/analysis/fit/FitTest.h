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
	static inline void FuzzyCompare(double actual, double expected, double delta = 0.000000000001) {
		QVERIFY(actual-delta <= expected && actual+delta >=expected);
	}

	//linear regression (see NIST/linear data)
	void testLinearNorris();
	void testLinearPontius();

	void testLinearWampler1();
	void testLinearWampler2();
	void testLinearWampler3();
	void testLinearWampler4();
	void testLinearWampler5();

	//non-linear regression

	//fits with weights
};
