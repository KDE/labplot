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

#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include <QtTest>

#include <gsl/gsl_math.h>

///////////////////////// macros ///////////

// Comparing two values. First a direct comparsion will be done, because for std::nan nsl_math_aproximately_equal does not work
#define VALUES_EQUAL(v1, ref)                                                                                                                                  \
	QVERIFY2(v1 == ref ? true : (std::isnan(ref) ? std::isnan(v1) : nsl_math_approximately_equal(v1, ref) == true),                                            \
			 qPrintable(QStringLiteral("v1:%1, ref:%2").arg((double)v1, 0, 'g', 15, QLatin1Char(' ')).arg((double)ref, 0, 'g', 15, QLatin1Char(' '))))

#define RANGE_CORRECT(range, start_, end_)                                                                                                                     \
	VALUES_EQUAL(range.start(), start_);                                                                                                                       \
	VALUES_EQUAL(range.end(), end_);

/*!
 * Checks the range of dim \p dim from the coordinatesystem with index \p cSystemIndex
 */
#define CHECK_RANGE_CSYSTEMINDEX(plot, cSystemIndex, dim, start_, end_)                                                                                        \
	RANGE_CORRECT(plot->range(dim, plot->coordinateSystem(cSystemIndex)->index(dim)), start_, end_)

/*!
 * Checks the range of dim \p dim from the coordinatesystem assigned to \p aspect
 */
#define CHECK_RANGE(plot, aspect, dim, start_, end_) CHECK_RANGE_CSYSTEMINDEX(plot, aspect->coordinateSystemIndex(), dim, start_, end_)

#define CHECK_SCALE_PLOT(plot, coordinateSystemIndex, dimension, a_ref, b_ref, c_ref)                                                                          \
	do {                                                                                                                                                       \
		QVERIFY(plot);                                                                                                                                         \
		QVERIFY(plot->coordinateSystemCount() > coordinateSystemIndex);                                                                                        \
		const auto scales = plot->coordinateSystem(coordinateSystemIndex)->scales(dimension);                                                                  \
		QCOMPARE(scales.length(), 1);                                                                                                                          \
		QVERIFY(scales.at(0) != nullptr);                                                                                                                      \
		CHECK_SCALE(scales.at(0), a_ref, b_ref, c_ref);                                                                                                        \
	} while (false);

#define CHECK_SCALE(scale, a_ref, b_ref, c_ref)                                                                                                                \
	do {                                                                                                                                                       \
		double a;                                                                                                                                              \
		double b;                                                                                                                                              \
		double c;                                                                                                                                              \
		Range<double> r;                                                                                                                                       \
		scale->getProperties(&r, &a, &b, &c);                                                                                                                  \
		QVERIFY2(nsl_math_approximately_equal(a, a_ref), qPrintable(QStringLiteral("a: v1:%1, ref:%2").arg(a).arg(a_ref)));                                    \
		QVERIFY2(nsl_math_approximately_equal(b, b_ref), qPrintable(QStringLiteral("b: v1:%1, ref:%2").arg(b).arg(b_ref)));                                    \
		QVERIFY2(nsl_math_approximately_equal(c, c_ref), qPrintable(QStringLiteral("c: v1:%1, ref:%2").arg(c).arg(c_ref)));                                    \
	} while (false);

#define DEBUG_RANGE(plot, aspect)                                                                                                                              \
	{                                                                                                                                                          \
		int cSystem = aspect->coordinateSystemIndex();                                                                                                         \
		WARN(Q_FUNC_INFO << ", csystem index = " << cSystem)                                                                                                   \
		int xIndex = plot->coordinateSystem(cSystem)->index(Dimension::X);                                                                                     \
		int yIndex = plot->coordinateSystem(cSystem)->index(Dimension::Y);                                                                                     \
                                                                                                                                                               \
		auto xrange = plot->range(Dimension::X, xIndex);                                                                                                       \
		auto yrange = plot->range(Dimension::Y, yIndex);                                                                                                       \
		WARN(Q_FUNC_INFO << ", x index = " << xIndex << ", range = " << xrange.start() << " .. " << xrange.end())                                              \
		WARN(Q_FUNC_INFO << ", y index = " << yIndex << ", range = " << yrange.start() << " .. " << yrange.end())                                              \
	}

#define COMPARE_DOUBLE_VECTORS_AT_LEAST_LENGTH(res, ref)                                                                                                       \
	do {																																					\
	QVERIFY(res.length() >= ref.length());                                                                                                                     \
	for (int i = 0; i < ref.length(); i++)                                                                                                                     \
		QVERIFY2(qFuzzyCompare(res.at(i), ref.at(i)),                                                                                                          \
				 qPrintable(QStringLiteral("i=") + QString::number(i) + QStringLiteral(", res=") + QString::number(res.at(i)) + QStringLiteral(", ref=")       \
							+ QString::number(ref.at(i)))); \
	} while (false)

#define COMPARE_DOUBLE_VECTORS(res, ref)                                                                                                                       \
	do {\
	QCOMPARE(res.length(), ref.length());                                                                                                                      \
	COMPARE_DOUBLE_VECTORS_AT_LEAST_LENGTH(res, ref);\
	} while (false)

#define COMPARE_STRING_VECTORS(res, ref)                                                                                                                       \
	QCOMPARE(res.length(), ref.length());                                                                                                                      \
	for (int i = 0; i < res.length(); i++)                                                                                                                     \
		QVERIFY2(res.at(i).compare(ref.at(i)) == 0,                                                                                                            \
				 qPrintable(QStringLiteral("i=") + QString::number(i) + QStringLiteral(", res=") + res.at(i) + QStringLiteral(", ref=") + ref.at(i)));

/*!
 * Stores the labplot project to a temporary file
 * The filename is then stored in the savePath variable
 */
#define SAVE_PROJECT(project_name)                                                                                                                             \
	do {                                                                                                                                                       \
		auto* tempFile = new QTemporaryFile(QStringLiteral("XXXXXX_") + QLatin1String(project_name) + QLatin1String(".lml"), this);                            \
		QCOMPARE(tempFile->open(), true);                                                                                                                      \
		savePath = tempFile->fileName();                                                                                                                       \
		QFile file(savePath);                                                                                                                                  \
		QCOMPARE(file.open(QIODevice::WriteOnly), true);                                                                                                       \
                                                                                                                                                               \
		project.setFileName(tempFile->fileName());                                                                                                             \
		QXmlStreamWriter writer(&file);                                                                                                                        \
		QPixmap thumbnail;                                                                                                                                     \
		project.save(thumbnail, &writer);                                                                                                                      \
		file.close();                                                                                                                                          \
		DEBUG(QStringLiteral("File stored as: ").toStdString() << savePath.toStdString());                                                                     \
		QVERIFY(!savePath.isEmpty());                                                                                                                          \
	} while (0);

///////////////////////////////////////////////////////

class CommonTest : public QObject {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

protected:
	// compare floats with given delta
	// delta - relative error
	static inline void FuzzyCompare(double actual, double expected, double delta = 1.e-12) {
		if (std::abs(expected) < delta)
			QVERIFY(std::abs(actual) < delta);
		else {
			DEBUG(std::setprecision(15) << actual - std::abs(actual) * delta << " <= " << expected << " <= " << actual + std::abs(actual) * delta);
			QVERIFY(!gsl_fcmp(actual, expected, delta));
		}
	}
};
#endif
