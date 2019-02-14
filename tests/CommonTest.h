/***************************************************************************
    File                 : CommonTest.h
    Project              : LabPlot
    Description          : General test class
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#ifndef COMMONTEST_H
#define COMMONTEST_H

#include <QtTest>
#include <backend/lib/macros.h>	// DEBUG()

extern "C" {
#include <gsl/gsl_math.h>
}

class CommonTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();
protected:
	// compare floats with given delta
	// delta - relative error
	static inline void FuzzyCompare(double actual, double expected, double delta = 1.e-12) {
		if (fabs(expected) < delta)
			QVERIFY(fabs(actual) < delta);
		else {
			DEBUG(std::setprecision(15) << actual - fabs(actual)*delta << " <= " << expected << " <= " << actual + fabs(actual)*delta);
			QVERIFY(!gsl_fcmp(actual, expected, delta));
		}
	}
};
#endif
