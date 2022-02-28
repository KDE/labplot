/*
	File                 : XYCurveTest.cpp
	Project              : LabPlot
	Description          : Tests for XYCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "XYCurveTest.h"

// To be able to access the private of the curve
#define private public
#define protected public
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"
#undef private
#undef protected

#include "backend/lib/trace.h"
#include "backend/core/Project.h"

#define GET_CURVE_PRIVATE(plot, child_index, column_name, curve_variable_name) \
	auto* curve_variable_name = plot->child<XYCurve>(child_index); \
	QVERIFY(curve_variable_name != nullptr); \
	QCOMPARE(curve_variable_name->name(), QLatin1String(column_name)); \
	QCOMPARE(curve_variable_name->type(), AspectType::XYCurve); \
	auto* curve_variable_name ## Private = curve_variable_name->d_func();

#define LOAD_PROJECT \
	Project project; \
	project.load(QFINDTESTDATA(QLatin1String("data/TestUpdateLines.lml"))); \
	auto* spreadsheet = project.child<AbstractAspect>(0); \
	QVERIFY(spreadsheet != nullptr); \
	QCOMPARE(spreadsheet->name(), QLatin1String("lastValueInvalid")); \
	QCOMPARE(spreadsheet->type(), AspectType::Spreadsheet); \
 \
	auto* worksheet = project.child<AbstractAspect>(1); \
	QVERIFY(worksheet != nullptr); \
	QCOMPARE(worksheet->name(), QLatin1String("Worksheet")); \
	QCOMPARE(worksheet->type(), AspectType::Worksheet); \
 \
	auto* plot = worksheet->child<CartesianPlot>(0); \
	QVERIFY(plot != nullptr); \
	QCOMPARE(plot->name(), QLatin1String("plot")); \
	/* enable once implemented correctly */ \
	/* QCOMPARE(plot->type(), AspectType::CartesianPlot); */ \
 \
	GET_CURVE_PRIVATE(plot, 0, "lastValueInvalid", lastValueInvalidCurve) \
	GET_CURVE_PRIVATE(plot, 1, "lastVertical", lastVerticalCurve) \
	GET_CURVE_PRIVATE(plot, 2, "withGap", withGapCurve) \
	GET_CURVE_PRIVATE(plot, 3, "withGap2", withGapCurve2)


#define COMPARE_LINES(line1, line2) QVERIFY((line1.p1() == line2.p1() && line1.p2() == line2.p2()) || (line1.p1() == line2.p2() && line1.p2() == line2.p1()))


void addUniqueLine01(QPointF p, double x, double& minY, double& maxY, QPointF& lastPoint, int& pixelDiff, QVector<QLineF>& lines);
void addUniqueLine02(QPointF p, double x, double& minY, double& maxY, QPointF& lastPoint, int& pixelDiff, QVector<QLineF>& lines);

void XYCurveTest::addUniqueLineTest01() {
	// For performance Testing only
	const int count = 100e6;
	QVector<QPointF> points(count);
	const double maxValue = 100;
	for (int i = 0; i < count; i++) {
		points[i].setX(double(i)/count * maxValue);
		points[i].setY(sin(double(i)/count * 2. * 3.1416 * 20.));
	}

	double x = 0;
	double minY = INFINITY;
	double maxY = -INFINITY;
	const int numberPixel = 680;
	const double minLogicalDiffX = maxValue/numberPixel; // amount pixel = count/minLogicalDiffX
	QPointF lastPoint;
	int pixelDiff = 0;
	QVector<QLineF> lines, lines1, lines2;
	bool prevPixelDiffZero = false;

	const int maximumLines = 2 * numberPixel; // number Pixel horizontal lines, number Pixel vertical lines

	{
		x = 0;
		PERFTRACE(QString(Q_FUNC_INFO) + "XYCurve::addUniqueLine");
		for (int i=0; i < count; i++) {
			pixelDiff = abs(qRound(points[i].x() / minLogicalDiffX) - qRound(x / minLogicalDiffX));
			XYCurvePrivate::addUniqueLine(points[i], minY, maxY, lastPoint, pixelDiff, lines, prevPixelDiffZero);
			if (pixelDiff > 0) // set x to next pixel
				x += minLogicalDiffX;
		}
	}

	{
		// No reallocation
		x = 0;
	PERFTRACE(QString(Q_FUNC_INFO) + "addUniqueLine01");
		for (int i=0; i < count; i++) {
			pixelDiff = abs(qRound(points[i].x() / minLogicalDiffX) - qRound(x / minLogicalDiffX));
			addUniqueLine01(points[i], x, minY, maxY, lastPoint, pixelDiff, lines1);
			if (pixelDiff > 0) // set x to next pixel
				x += minLogicalDiffX;
		}
		QTEST_ASSERT(lines1.count() <= maximumLines);
	}

	{
		// Reallocating array
		x = 0;
		lines2.resize(maximumLines);
	PERFTRACE(QString(Q_FUNC_INFO) + "addUniqueLine02");
		for (int i=0; i < count; i++) {
			pixelDiff = abs(qRound(points[i].x() / minLogicalDiffX) - qRound(x / minLogicalDiffX));
			addUniqueLine02(points[i], x, minY, maxY, lastPoint, pixelDiff, lines2);
			if (pixelDiff > 0) // set x to next pixel
				x += minLogicalDiffX;
		}
	}
}

void addUniqueLine01(QPointF p, double x, double& minY, double& maxY, QPointF& lastPoint, int& pixelDiff, QVector<QLineF>& lines) {
	static bool prevPixelDiffZero = false;
	if (pixelDiff == 0) {
		maxY = qMax(p.y(), maxY);
		minY = qMin(p.y(), minY);
		lastPoint = p;	// save last point
		prevPixelDiffZero = true;
	} else {
		if (prevPixelDiffZero) {
			lines.append(QLineF(x, maxY, x, minY));
			lines.append(QLineF(QPointF(x, lastPoint.y()), p));
		} else
			lines.append(QLineF(lastPoint, p));
		prevPixelDiffZero = false;
		minY = p.y();
		maxY = p.y();
		lastPoint = p;
	}
}

void addUniqueLine02(QPointF p, double x, double& minY, double& maxY, QPointF& lastPoint, int& pixelDiff, QVector<QLineF>& lines) {
	static bool prevPixelDiffZero = false;
	static int i = 0;
	if (pixelDiff == 0) {
		maxY = qMax(p.y(), maxY);
		minY = qMin(p.y(), minY);
		lastPoint = p;	// save last point
		prevPixelDiffZero = true;
	} else {
		if (prevPixelDiffZero) {
			lines[i] = QLineF(x, maxY, x, minY);
			i++;
			lines[i] = QLineF(QPointF(x, lastPoint.y()), p);
		} else
			lines[i] = QLineF(lastPoint, p);
		i++;
		prevPixelDiffZero = false;
		minY = p.y();
		maxY = p.y();
		lastPoint = p;
	}
}

void XYCurveTest::updateLinesNoGapDirectConnection() {
	LOAD_PROJECT

	QVector<QLineF> refLines {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(10, 8)),
	};
	QCOMPARE(lastValueInvalidCurvePrivate->m_logicalPoints.size(), refLines.size() + 1); // last row is invalid so it will be ommitted
	auto test_lines = lastValueInvalidCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapStartHorizontal() {
	LOAD_PROJECT

	lastValueInvalidCurve->setLineType(XYCurve::LineType::StartHorizontal);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 1)),
		QLineF(QPointF(2, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 2)),
		QLineF(QPointF(3, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 3)),
		QLineF(QPointF(4, 3), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(5, 7)),
		QLineF(QPointF(5, 7), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(6, 15)),
		QLineF(QPointF(6, 15), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(7, 3)),
		QLineF(QPointF(7, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, -10)),
		QLineF(QPointF(8, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 0)),
		QLineF(QPointF(9, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(10, 5)),
		QLineF(QPointF(10, 5), QPointF(10, 8)),
	};
	auto test_lines = lastValueInvalidCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapStartVertical() {
	LOAD_PROJECT

	lastValueInvalidCurve->setLineType(XYCurve::LineType::StartVertical);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1, 2)),
		QLineF(QPointF(1, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2, 3)),
		QLineF(QPointF(2, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3, 7)),
		QLineF(QPointF(3, 7), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(4, 15)),
		QLineF(QPointF(4, 15), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(5, 3)),
		QLineF(QPointF(5, 3), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(6, -10)),
		QLineF(QPointF(6, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7, 0)),
		QLineF(QPointF(7, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8, 5)),
		QLineF(QPointF(8, 5), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9, 8)),
		QLineF(QPointF(9, 8), QPointF(10, 8)),
	};
	auto test_lines = lastValueInvalidCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapMidPointHorizontal() {
	LOAD_PROJECT

	lastValueInvalidCurve->setLineType(XYCurve::LineType::MidpointHorizontal);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1.5, 1)),
		QLineF(QPointF(1.5, 1), QPointF(1.5, 2)),
		QLineF(QPointF(1.5, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2.5, 2)),
		QLineF(QPointF(2.5, 2), QPointF(2.5, 3)),
		QLineF(QPointF(2.5, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3.5, 3)),
		QLineF(QPointF(3.5, 3), QPointF(3.5, 7)),
		QLineF(QPointF(3.5, 7), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(4.5, 7)),
		QLineF(QPointF(4.5, 7), QPointF(4.5, 15)),
		QLineF(QPointF(4.5, 15), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(5.5, 15)),
		QLineF(QPointF(5.5, 15), QPointF(5.5, 3)),
		QLineF(QPointF(5.5, 3), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(6.5, 3)),
		QLineF(QPointF(6.5, 3), QPointF(6.5, -10)),
		QLineF(QPointF(6.5, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7.5, -10)),
		QLineF(QPointF(7.5, -10), QPointF(7.5, 0)),
		QLineF(QPointF(7.5, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8.5, 0)),
		QLineF(QPointF(8.5, 0), QPointF(8.5, 5)),
		QLineF(QPointF(8.5, 5), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9.5, 5)),
		QLineF(QPointF(9.5, 5), QPointF(9.5, 8)),
		QLineF(QPointF(9.5, 8), QPointF(10, 8)),

	};
	auto test_lines = lastValueInvalidCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapMidPointVertical() {
	LOAD_PROJECT

	lastValueInvalidCurve->setLineType(XYCurve::LineType::MidpointVertical);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1, 1.5)),
		QLineF(QPointF(1, 1.5), QPointF(2, 1.5)),
//		QLineF(QPointF(2, 1.5), QPointF(2, 2)),
//		QLineF(QPointF(2, 2), QPointF(2, 2.5)),
		QLineF(QPointF(2, 1.5), QPointF(2, 2.5)),

		QLineF(QPointF(2, 2.5), QPointF(3, 2.5)),
//		QLineF(QPointF(3, 2.5), QPointF(3, 3)),
//		QLineF(QPointF(3, 3), QPointF(3, 5)),
		QLineF(QPointF(3, 2.5), QPointF(3, 5)),

		QLineF(QPointF(3, 5), QPointF(4, 5)),
//		QLineF(QPointF(4, 5), QPointF(4, 7)),
//		QLineF(QPointF(4, 7), QPointF(4, 11)),
		QLineF(QPointF(4, 5), QPointF(4, 11)),

		QLineF(QPointF(4, 11), QPointF(5, 11)),
//		QLineF(QPointF(5, 11), QPointF(5, 15)),
//		QLineF(QPointF(5, 15), QPointF(5, 9)),
		QLineF(QPointF(5, 9), QPointF(5, 15)),

		QLineF(QPointF(5, 9), QPointF(6, 9)),
//		QLineF(QPointF(6, 9), QPointF(6, 3)),
//		QLineF(QPointF(6, 3), QPointF(6, -3.5)),
		QLineF(QPointF(6, 9), QPointF(6,-3.5)),

		QLineF(QPointF(6, -3.5), QPointF(7, -3.5)),
//		QLineF(QPointF(7, -3.5), QPointF(7, -10)),
//		QLineF(QPointF(7, -10), QPointF(7, -5)),
		QLineF(QPointF(7, -10), QPointF(7, -3.5)),

		QLineF(QPointF(7, -5), QPointF(8, -5)),
//		QLineF(QPointF(8, -5), QPointF(8, 0)),
//		QLineF(QPointF(8, 0), QPointF(8, 2.5)),
		QLineF(QPointF(8, -5), QPointF(8, 2.5)),

		QLineF(QPointF(8, 2.5), QPointF(9, 2.5)),
//		QLineF(QPointF(9, 2.5), QPointF(9, 5)),
//		QLineF(QPointF(9, 5), QPointF(9, 6.5)),
		QLineF(QPointF(9, 2.5), QPointF(9, 6.5)),

		QLineF(QPointF(9, 6.5), QPointF(10, 6.5)),
		QLineF(QPointF(10, 6.5), QPointF(10, 8)),
	};
	auto test_lines = lastValueInvalidCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapSegments2() {
	LOAD_PROJECT

	lastValueInvalidCurve->setLineType(XYCurve::LineType::Segments2);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
//		QLineF(QPointF(2, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 7)),
//		QLineF(QPointF(4, 7), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(6, 3)),
//		QLineF(QPointF(6, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, 0)),
//		QLineF(QPointF(8, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(10, 8)),
	};
	auto test_lines = lastValueInvalidCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapSegments3() {
	LOAD_PROJECT

	lastValueInvalidCurve->setLineType(XYCurve::LineType::Segments3);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 3)),
//		QLineF(QPointF(3, 3), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(6, 3)),
//		QLineF(QPointF(6, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 5)),
//		QLineF(QPointF(9, 5), QPointF(10, 8)),
	};
	auto test_lines = lastValueInvalidCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapDirectConnectionLastVertical() {
	LOAD_PROJECT

	QVector<QLineF> refLines {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9, 8)),
	};
	QCOMPARE(lastVerticalCurvePrivate->m_logicalPoints.size(), refLines.size() + 1); // last row is invalid so it will be ommitted
	auto test_lines = lastVerticalCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapStartHorizontalLastVertical() {
	LOAD_PROJECT

	lastVerticalCurve->setLineType(XYCurve::LineType::StartHorizontal);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 1)),
		QLineF(QPointF(2, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 2)),
		QLineF(QPointF(3, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 3)),
		QLineF(QPointF(4, 3), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(5, 7)),
		QLineF(QPointF(5, 7), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(6, 15)),
		QLineF(QPointF(6, 15), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(7, 3)),
		QLineF(QPointF(7, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, -10)),
		QLineF(QPointF(8, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 0)),
		QLineF(QPointF(9, 0), QPointF(9, 8)),
	};
	auto test_lines = lastVerticalCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapStartVerticalLastVertical(){
	LOAD_PROJECT

	lastVerticalCurve->setLineType(XYCurve::LineType::StartVertical);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1, 2)),
		QLineF(QPointF(1, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2, 3)),
		QLineF(QPointF(2, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3, 7)),
		QLineF(QPointF(3, 7), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(4, 15)),
		QLineF(QPointF(4, 15), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(5, 3)),
		QLineF(QPointF(5, 3), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(6, -10)),
		QLineF(QPointF(6, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7, 0)),
		QLineF(QPointF(7, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8, 5)),
		QLineF(QPointF(8, 5), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9, 8)),
	};
	auto test_lines = lastVerticalCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapMidPointHorizontalLastVertical() {
	LOAD_PROJECT

	lastVerticalCurve->setLineType(XYCurve::LineType::MidpointHorizontal);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1.5, 1)),
		QLineF(QPointF(1.5, 1), QPointF(1.5, 2)),
		QLineF(QPointF(1.5, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2.5, 2)),
		QLineF(QPointF(2.5, 2), QPointF(2.5, 3)),
		QLineF(QPointF(2.5, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3.5, 3)),
		QLineF(QPointF(3.5, 3), QPointF(3.5, 7)),
		QLineF(QPointF(3.5, 7), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(4.5, 7)),
		QLineF(QPointF(4.5, 7), QPointF(4.5, 15)),
		QLineF(QPointF(4.5, 15), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(5.5, 15)),
		QLineF(QPointF(5.5, 15), QPointF(5.5, 3)),
		QLineF(QPointF(5.5, 3), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(6.5, 3)),
		QLineF(QPointF(6.5, 3), QPointF(6.5, -10)),
		QLineF(QPointF(6.5, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7.5, -10)),
		QLineF(QPointF(7.5, -10), QPointF(7.5, 0)),
		QLineF(QPointF(7.5, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8.5, 0)),
		QLineF(QPointF(8.5, 0), QPointF(8.5, 5)),
		QLineF(QPointF(8.5, 5), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9, 8)),
	};
	auto test_lines = lastVerticalCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapMidPointVerticalLastVertical() {
	LOAD_PROJECT

	lastVerticalCurve->setLineType(XYCurve::LineType::MidpointVertical);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1, 1.5)),
		QLineF(QPointF(1, 1.5), QPointF(2, 1.5)),
//		QLineF(QPointF(2, 1.5), QPointF(2, 2)),
//		QLineF(QPointF(2, 2), QPointF(2, 2.5)),
		QLineF(QPointF(2, 1.5), QPointF(2, 2.5)),

		QLineF(QPointF(2, 2.5), QPointF(3, 2.5)),
//		QLineF(QPointF(3, 2.5), QPointF(3, 3)),
//		QLineF(QPointF(3, 3), QPointF(3, 5)),
		QLineF(QPointF(3, 2.5), QPointF(3, 5)),

		QLineF(QPointF(3, 5), QPointF(4, 5)),
//		QLineF(QPointF(4, 5), QPointF(4, 7)),
//		QLineF(QPointF(4, 7), QPointF(4, 11)),
		QLineF(QPointF(4, 5), QPointF(4, 11)),

		QLineF(QPointF(4, 11), QPointF(5, 11)),
//		QLineF(QPointF(5, 11), QPointF(5, 15)),
//		QLineF(QPointF(5, 15), QPointF(5, 9)),
		QLineF(QPointF(5, 9), QPointF(5, 15)),

		QLineF(QPointF(5, 9), QPointF(6, 9)),
//		QLineF(QPointF(6, 9), QPointF(6, 3)),
//		QLineF(QPointF(6, 3), QPointF(6, -3.5)),
		QLineF(QPointF(6, 9), QPointF(6,-3.5)),

		QLineF(QPointF(6, -3.5), QPointF(7, -3.5)),
//		QLineF(QPointF(7, -3.5), QPointF(7, -10)),
//		QLineF(QPointF(7, -10), QPointF(7, -5)),
		QLineF(QPointF(7, -10), QPointF(7, -3.5)),

		QLineF(QPointF(7, -5), QPointF(8, -5)),
//		QLineF(QPointF(8, -5), QPointF(8, 0)),
//		QLineF(QPointF(8, 0), QPointF(8, 2.5)),
		QLineF(QPointF(8, -5), QPointF(8, 2.5)),

		QLineF(QPointF(8, 2.5), QPointF(9, 2.5)),
//		QLineF(QPointF(9, 2.5), QPointF(9, 5)),
//		QLineF(QPointF(9, 5), QPointF(9, 6.5)),
		QLineF(QPointF(9, 2.5), QPointF(9, 8)),
	};
	auto test_lines = lastVerticalCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapSegments2LastVertical() {
	LOAD_PROJECT

	lastVerticalCurve->setLineType(XYCurve::LineType::Segments2);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
//		QLineF(QPointF(2, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 7)),
//		QLineF(QPointF(4, 7), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(6, 3)),
//		QLineF(QPointF(6, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, 0)),
//		QLineF(QPointF(8, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9, 8)),
	};
	auto test_lines = lastVerticalCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesNoGapSegments3LastVertical() {
	LOAD_PROJECT

	lastVerticalCurve->setLineType(XYCurve::LineType::Segments3);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 3)),
//		QLineF(QPointF(3, 3), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(6, 3)),
//		QLineF(QPointF(6, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 5)),
//		QLineF(QPointF(9, 5), QPointF(9, 8)),
	};
	auto test_lines = lastVerticalCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapLineSkipDirectConnection() {
	LOAD_PROJECT

	QVector<QLineF> refLines {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(5, 5)),
		QLineF(QPointF(5, 5), QPointF(6, 6)),
	};
	withGapCurve->setLineSkipGaps(true);
	withGapCurvePrivate->updateLines();
	QCOMPARE(withGapCurvePrivate->m_logicalPoints.size(), refLines.size() + 1);
	auto test_lines = withGapCurvePrivate->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

//######################################################################################
// With Gap, skipGaps = True
//######################################################################################
void XYCurveTest::updateLinesWithGapLineSkipDirectConnection2() {
	LOAD_PROJECT

	QVector<QLineF> refLines {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(10, 8)),
	};
	withGapCurve2->setLineSkipGaps(true);
	QCOMPARE(withGapCurve2Private->m_logicalPoints.size(), refLines.size() + 1); // last row is invalid so it will be ommitted
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapLineSkipStartHorizontal() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::StartHorizontal);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 1)),
		QLineF(QPointF(2, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 2)),
		QLineF(QPointF(3, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 3)),
		QLineF(QPointF(4, 3), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(6, 7)),
		QLineF(QPointF(6, 7), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(7, 3)),
		QLineF(QPointF(7, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, -10)),
		QLineF(QPointF(8, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 0)),
		QLineF(QPointF(9, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(10, 5)),
		QLineF(QPointF(10, 5), QPointF(10, 8)),
	};
	withGapCurve2->setLineSkipGaps(true);
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapLineSkipStartVertical() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::StartVertical);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1, 2)),
		QLineF(QPointF(1, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2, 3)),
		QLineF(QPointF(2, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3, 7)),
		QLineF(QPointF(3, 7), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(4, 3)),
		QLineF(QPointF(4, 3), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(6, -10)),
		QLineF(QPointF(6, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7, 0)),
		QLineF(QPointF(7, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8, 5)),
		QLineF(QPointF(8, 5), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9, 8)),
		QLineF(QPointF(9, 8), QPointF(10, 8)),
	};
	withGapCurve2->setLineSkipGaps(true);
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapLineSkipMidPointHorizontal() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::MidpointHorizontal);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1.5, 1)),
		QLineF(QPointF(1.5, 1), QPointF(1.5, 2)),
		QLineF(QPointF(1.5, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2.5, 2)),
		QLineF(QPointF(2.5, 2), QPointF(2.5, 3)),
		QLineF(QPointF(2.5, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3.5, 3)),
		QLineF(QPointF(3.5, 3), QPointF(3.5, 7)),
		QLineF(QPointF(3.5, 7), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(5, 7)),
		QLineF(QPointF(5, 7), QPointF(5, 3)),
		QLineF(QPointF(5, 3), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(6.5, 3)),
		QLineF(QPointF(6.5, 3), QPointF(6.5, -10)),
		QLineF(QPointF(6.5, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7.5, -10)),
		QLineF(QPointF(7.5, -10), QPointF(7.5, 0)),
		QLineF(QPointF(7.5, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8.5, 0)),
		QLineF(QPointF(8.5, 0), QPointF(8.5, 5)),
		QLineF(QPointF(8.5, 5), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9.5, 5)),
		QLineF(QPointF(9.5, 5), QPointF(9.5, 8)),
		QLineF(QPointF(9.5, 8), QPointF(10, 8)),

	};
	withGapCurve2->setLineSkipGaps(true);
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapLineSkipMidPointVertical() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::MidpointVertical);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1, 1.5)), // vertical
		QLineF(QPointF(1, 1.5), QPointF(2, 1.5)),
//		QLineF(QPointF(2, 1.5), QPointF(2, 2)),
//		QLineF(QPointF(2, 2), QPointF(2, 2.5)),
		QLineF(QPointF(2, 1.5), QPointF(2, 2.5)), // vertical

		QLineF(QPointF(2, 2.5), QPointF(3, 2.5)),
//		QLineF(QPointF(3, 2.5), QPointF(3, 3)),
//		QLineF(QPointF(3, 3), QPointF(3, 5)),
		QLineF(QPointF(3, 2.5), QPointF(3, 5)), // vertical

		QLineF(QPointF(3, 5), QPointF(4, 5)),
//		QLineF(QPointF(4, 5), QPointF(4, 7)),
//		QLineF(QPointF(4, 7), QPointF(4, 5)),
		QLineF(QPointF(4, 5), QPointF(4, 7)), // vertical

		QLineF(QPointF(4, 5), QPointF(6, 5)),
//		QLineF(QPointF(6, 5), QPointF(6, 3)),
//		QLineF(QPointF(6, 3), QPointF(6, -3.5)),
		QLineF(QPointF(6, 5), QPointF(6,-3.5)), // vertical

		QLineF(QPointF(6, -3.5), QPointF(7, -3.5)),
//		QLineF(QPointF(7, -3.5), QPointF(7, -10)),
//		QLineF(QPointF(7, -10), QPointF(7, -5)),
		QLineF(QPointF(7, -10), QPointF(7, -3.5)), // vertical

		QLineF(QPointF(7, -5), QPointF(8, -5)),
//		QLineF(QPointF(8, -5), QPointF(8, 0)),
//		QLineF(QPointF(8, 0), QPointF(8, 2.5)),
		QLineF(QPointF(8, -5), QPointF(8, 2.5)), // vertical

		QLineF(QPointF(8, 2.5), QPointF(9, 2.5)),
//		QLineF(QPointF(9, 2.5), QPointF(9, 5)),
//		QLineF(QPointF(9, 5), QPointF(9, 6.5)),
		QLineF(QPointF(9, 2.5), QPointF(9, 6.5)), // vertical

		QLineF(QPointF(9, 6.5), QPointF(10, 6.5)),
		QLineF(QPointF(10, 6.5), QPointF(10, 8)), // vertical
	};
	withGapCurve2->setLineSkipGaps(true);
	auto test_lines = withGapCurve2Private->m_lines_test;
	//QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		DEBUG(i);
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapLineSkipSegments2() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::Segments2);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
//		QLineF(QPointF(2, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 7)),
//		QLineF(QPointF(4, 7), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(7, -10)),
//		QLineF(QPointF(7, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 5)),
//		QLineF(QPointF(9, 5), QPointF(10, 8)),
	};
	withGapCurve2->setLineSkipGaps(true);
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapLineSkipSegments3() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::Segments3);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 3)),
//		QLineF(QPointF(3, 3), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(7, -10)),
//		QLineF(QPointF(7, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(10, 8)),
	};
	withGapCurve2->setLineSkipGaps(true);
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

//######################################################################################
// With Gap, skipGaps = false
//######################################################################################
void XYCurveTest::updateLinesWithGapDirectConnection() {
	LOAD_PROJECT

	QVector<QLineF> refLines {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 7)),
		QLineF(QPointF(6, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(10, 8)),
	};
	withGapCurve2->setLineSkipGaps(false);
	QCOMPARE(withGapCurve2Private->m_logicalPoints.size() - 2, refLines.size()); // one point will be skipped, so 2 lines less
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapStartHorizontal() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::StartHorizontal);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 1)),
		QLineF(QPointF(2, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 2)),
		QLineF(QPointF(3, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 3)),
		QLineF(QPointF(4, 3), QPointF(4, 7)),
		QLineF(QPointF(6, 3), QPointF(7, 3)),
		QLineF(QPointF(7, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, -10)),
		QLineF(QPointF(8, -10), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(9, 0)),
		QLineF(QPointF(9, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(10, 5)),
		QLineF(QPointF(10, 5), QPointF(10, 8)),
	};
	withGapCurve2->setLineSkipGaps(false);
	auto test_lines = withGapCurve2Private->m_lines_test;
	//QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		DEBUG(i)
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapStartVertical() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::StartVertical);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1, 2)),
		QLineF(QPointF(1, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2, 3)),
		QLineF(QPointF(2, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3, 7)),
		QLineF(QPointF(3, 7), QPointF(4, 7)),
		QLineF(QPointF(6, 3), QPointF(6, -10)),
		QLineF(QPointF(6, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7, 0)),
		QLineF(QPointF(7, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8, 5)),
		QLineF(QPointF(8, 5), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9, 8)),
		QLineF(QPointF(9, 8), QPointF(10, 8)),
	};
	withGapCurve2->setLineSkipGaps(false);
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapMidPointHorizontal() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::MidpointHorizontal);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1.5, 1)),
		QLineF(QPointF(1.5, 1), QPointF(1.5, 2)),
		QLineF(QPointF(1.5, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2.5, 2)),
		QLineF(QPointF(2.5, 2), QPointF(2.5, 3)),
		QLineF(QPointF(2.5, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3.5, 3)),
		QLineF(QPointF(3.5, 3), QPointF(3.5, 7)),
		QLineF(QPointF(3.5, 7), QPointF(4, 7)),
		QLineF(QPointF(6, 3), QPointF(6.5, 3)),
		QLineF(QPointF(6.5, 3), QPointF(6.5, -10)),
		QLineF(QPointF(6.5, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7.5, -10)),
		QLineF(QPointF(7.5, -10), QPointF(7.5, 0)),
		QLineF(QPointF(7.5, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8.5, 0)),
		QLineF(QPointF(8.5, 0), QPointF(8.5, 5)),
		QLineF(QPointF(8.5, 5), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9.5, 5)),
		QLineF(QPointF(9.5, 5), QPointF(9.5, 8)),
		QLineF(QPointF(9.5, 8), QPointF(10, 8)),

	};
	withGapCurve2->setLineSkipGaps(false);
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		DEBUG(i)
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapMidPointVertical() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::MidpointVertical);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(1, 1.5)),
		QLineF(QPointF(1, 1.5), QPointF(2, 1.5)),
//		QLineF(QPointF(2, 1.5), QPointF(2, 2)),
//		QLineF(QPointF(2, 2), QPointF(2, 2.5)),
		QLineF(QPointF(2, 1.5), QPointF(2, 2.5)),

		QLineF(QPointF(2, 2.5), QPointF(3, 2.5)),
//		QLineF(QPointF(3, 2.5), QPointF(3, 3)),
//		QLineF(QPointF(3, 3), QPointF(3, 5)),
		QLineF(QPointF(3, 2.5), QPointF(3, 5)), // vertical

		QLineF(QPointF(3, 5), QPointF(4, 5)),
		QLineF(QPointF(4, 5), QPointF(4, 7)),
		// GAP
		QLineF(QPointF(6, 3), QPointF(6,-3.5)),

		QLineF(QPointF(6, -3.5), QPointF(7, -3.5)),
//		QLineF(QPointF(7, -3.5), QPointF(7, -10)),
//		QLineF(QPointF(7, -10), QPointF(7, -5)),
		QLineF(QPointF(7, -10), QPointF(7, -3.5)),

		QLineF(QPointF(7, -5), QPointF(8, -5)),
//		QLineF(QPointF(8, -5), QPointF(8, 0)),
//		QLineF(QPointF(8, 0), QPointF(8, 2.5)),
		QLineF(QPointF(8, -5), QPointF(8, 2.5)),

		QLineF(QPointF(8, 2.5), QPointF(9, 2.5)),
//		QLineF(QPointF(9, 2.5), QPointF(9, 5)),
//		QLineF(QPointF(9, 5), QPointF(9, 6.5)),
		QLineF(QPointF(9, 2.5), QPointF(9, 6.5)),

		QLineF(QPointF(9, 6.5), QPointF(10, 6.5)),
		QLineF(QPointF(10, 6.5), QPointF(10, 8)),
	};

	withGapCurve2->setLineSkipGaps(false);
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		DEBUG(i)
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapSegments2() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::Segments2);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
//		QLineF(QPointF(2, 2), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(4, 7)),
//		QLineF(QPointF(6, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, 0)),
//		QLineF(QPointF(8, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(10, 8)),
	};
	withGapCurve2->setLineSkipGaps(false);
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		DEBUG(i)
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapSegments3() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::Segments3);
	QVector<QLineF> refLines  = {
		QLineF(QPointF(1, 1), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(3, 3)),
//		QLineF(QPointF(3, 3), QPointF(4, 7)),
		QLineF(QPointF(6, 3), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(8, 0)),
//		QLineF(QPointF(8, 0), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(10, 8)),
	};
	withGapCurve2->setLineSkipGaps(false);
	auto test_lines = withGapCurve2Private->m_lines_test;
	QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

// TODO: create tests for Splines

QTEST_MAIN(XYCurveTest)
