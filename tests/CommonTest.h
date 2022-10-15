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

extern "C" {
#include <gsl/gsl_math.h>
}

///////////////////////// macros ///////////

#define VALUES_EQUAL(v1, v2) QCOMPARE(nsl_math_approximately_equal(v1, v2), true);

#define RANGE_CORRECT(range, start_, end_)                                                                                                                     \
	VALUES_EQUAL(range.start(), start_)                                                                                                                        \
	VALUES_EQUAL(range.end(), end_)

#define CHECK_RANGE(plot, aspect, dim, start_, end_)                                                                                                           \
	RANGE_CORRECT(plot->range(dim, plot->coordinateSystem(aspect->coordinateSystemIndex())->index(dim)), start_, end_)

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

#define COMPARE_DOUBLE_VECTORS(res, ref)                                                                                                                       \
	QCOMPARE(res.length(), ref.length());                                                                                                                      \
	for (int i = 0; i < res.length(); i++)                                                                                                                     \
		QVERIFY2(qFuzzyCompare(res.at(i), ref.at(i)),                                                                                                          \
				 qPrintable(QString("i=") + QString::number(i) + ", res=" + QString::number(res.at(i)) + ", ref=" + QString::number(ref.at(i))));

#define COMPARE_STRING_VECTORS(res, ref)                                                                                                                       \
	QCOMPARE(res.length(), ref.length());                                                                                                                      \
	for (int i = 0; i < res.length(); i++)                                                                                                                     \
		QVERIFY2(res.at(i).compare(ref.at(i)) == 0, qPrintable(QString("i=") + QString::number(i) + ", res=" + res.at(i) + ", ref=" + ref.at(i)));

#define SAVE_PROJECT(project_name)                                                                                                                             \
	do {                                                                                                                                                       \
		auto* tempFile = new QTemporaryFile(QString("XXXXXX_") + QString(project_name), this);                                                                 \
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
		DEBUG(QString("File stored as: ").toStdString() << tempFile->fileName().toStdString());                                                                \
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
