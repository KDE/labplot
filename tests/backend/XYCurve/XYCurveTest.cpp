/*
	File                 : XYCurveTest.cpp
	Project              : LabPlot
	Description          : Tests for XYCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYCurveTest.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"
#include "backend/lib/trace.h"

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

	const int maximumLines = 2 * numberPixel; // number Pixel horizontal lines, number Pixel vertical lines

	{
		x = 0;
		PERFTRACE(QString(Q_FUNC_INFO) + "XYCurve::addUniqueLine");
		for (int i=0; i < count; i++) {
			pixelDiff = abs(qRound(points[i].x() / minLogicalDiffX) - qRound(x / minLogicalDiffX));
			XYCurvePrivate::addUniqueLine(points[i], minY, maxY, lastPoint, pixelDiff, lines);
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



QTEST_MAIN(XYCurveTest)
