/*
    File                 : CommonTest.h
    Project              : LabPlot
    Description          : General test class
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Stefan Gerlach (stefan.gerlach@uni.kn)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
