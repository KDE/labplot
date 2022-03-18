/*
    File                 : CommonTest.h
    Project              : LabPlot
    Description          : General test class
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019-2022 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef COMMONTEST_H
#define COMMONTEST_H

#include <QtTest>
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"

extern "C" {
#include <gsl/gsl_math.h>
}

///////////////////////// macros ///////////

#define VALUES_EQUAL(v1, v2) QCOMPARE(nsl_math_approximately_equal(v1, v2), true)

#define RANGE_CORRECT(range, start_, end_) \
	VALUES_EQUAL(range.start(), start_); \
	VALUES_EQUAL(range.end(), end_);

#define CHECK_RANGE(plot, aspect, xy, start_, end_) \
	RANGE_CORRECT(plot->xy ## Range(plot->coordinateSystem(aspect->coordinateSystemIndex())->xy ## Index()), start_, end_)

#define DEBUG_RANGE(plot, aspect) \
{\
	int cSystem = aspect->coordinateSystemIndex(); \
	WARN(Q_FUNC_INFO << ", csystem index = " << cSystem) \
	int xIndex = plot->coordinateSystem(cSystem)->xIndex(); \
	int yIndex = plot->coordinateSystem(cSystem)->yIndex(); \
\
	auto xrange = plot->xRange(xIndex); \
	auto yrange = plot->yRange(yIndex); \
	WARN(Q_FUNC_INFO << ", x index = " << xIndex << ", range = " << xrange.start() << " .. " << xrange.end()) \
	WARN(Q_FUNC_INFO << ", y index = " << yIndex << ", range = " << yrange.start() << " .. " << yrange.end()) \
}
///////////////////////////////////////////////////////

class CommonTest : public QObject {
	Q_OBJECT

private Q_SLOTS:
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
