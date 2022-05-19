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
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"
#undef private

#include "backend/core/Project.h"
#include "backend/lib/trace.h"

#include <QFile>

#define GET_CURVE_PRIVATE(plot, child_index, column_name, curve_variable_name)                                                                                 \
	auto* curve_variable_name = plot->child<XYCurve>(child_index);                                                                                             \
	QVERIFY(curve_variable_name != nullptr);                                                                                                                   \
	QCOMPARE(curve_variable_name->name(), QLatin1String(column_name));                                                                                         \
	QCOMPARE(curve_variable_name->type(), AspectType::XYCurve);                                                                                                \
	auto* curve_variable_name##Private = curve_variable_name->d_func();                                                                                        \
	Q_UNUSED(curve_variable_name##Private)

#define LOAD_PROJECT                                                                                                                                           \
	Project project;                                                                                                                                           \
	project.load(QFINDTESTDATA(QLatin1String("data/TestUpdateLines.lml")));                                                                                    \
	auto* spreadsheet = project.child<AbstractAspect>(0);                                                                                                      \
	QVERIFY(spreadsheet != nullptr);                                                                                                                           \
	QCOMPARE(spreadsheet->name(), QLatin1String("lastValueInvalid"));                                                                                          \
	QCOMPARE(spreadsheet->type(), AspectType::Spreadsheet);                                                                                                    \
                                                                                                                                                               \
	auto* worksheet = project.child<AbstractAspect>(1);                                                                                                        \
	QVERIFY(worksheet != nullptr);                                                                                                                             \
	QCOMPARE(worksheet->name(), QLatin1String("Worksheet"));                                                                                                   \
	QCOMPARE(worksheet->type(), AspectType::Worksheet);                                                                                                        \
                                                                                                                                                               \
	auto* plot = worksheet->child<CartesianPlot>(0);                                                                                                           \
	QVERIFY(plot != nullptr);                                                                                                                                  \
	QCOMPARE(plot->name(), QLatin1String("plot"));                                                                                                             \
	/* enable once implemented correctly */                                                                                                                    \
	/* QCOMPARE(plot->type(), AspectType::CartesianPlot); */                                                                                                   \
                                                                                                                                                               \
	GET_CURVE_PRIVATE(plot, 0, "lastValueInvalid", lastValueInvalidCurve)                                                                                      \
	GET_CURVE_PRIVATE(plot, 1, "lastVertical", lastVerticalCurve)                                                                                              \
	GET_CURVE_PRIVATE(plot, 2, "withGap", withGapCurve)                                                                                                        \
	GET_CURVE_PRIVATE(plot, 3, "withGap2", withGapCurve2)

#define LOAD_HOVER_PROJECT                                                                                                                                     \
	Project project;                                                                                                                                           \
	project.load(QFINDTESTDATA(QLatin1String("data/curveHover.lml")));                                                                                         \
	auto* spreadsheet = project.child<AbstractAspect>(0);                                                                                                      \
	QVERIFY(spreadsheet != nullptr);                                                                                                                           \
	QCOMPARE(spreadsheet->name(), QLatin1String("Spreadsheet"));                                                                                               \
	QCOMPARE(spreadsheet->type(), AspectType::Spreadsheet);                                                                                                    \
                                                                                                                                                               \
	auto* worksheet = project.child<AbstractAspect>(1);                                                                                                        \
	QVERIFY(worksheet != nullptr);                                                                                                                             \
	QCOMPARE(worksheet->name(), QLatin1String("Worksheet - Spreadsheet"));                                                                                     \
	QCOMPARE(worksheet->type(), AspectType::Worksheet);                                                                                                        \
                                                                                                                                                               \
	auto* plot = worksheet->child<CartesianPlot>(0);                                                                                                           \
	QVERIFY(plot != nullptr);                                                                                                                                  \
	QCOMPARE(plot->name(), QLatin1String("Plot - Spreadsheet"));                                                                                               \
	/* enable once implemented correctly */                                                                                                                    \
	/* QCOMPARE(plot->type(), AspectType::CartesianPlot); */                                                                                                   \
                                                                                                                                                               \
	GET_CURVE_PRIVATE(plot, 0, "IntegerNonMonotonic", integerNonMonotonic)

#define COMPARE_LINES(line1, line2) QVERIFY((line1.p1() == line2.p1() && line1.p2() == line2.p2()) || (line1.p1() == line2.p2() && line1.p2() == line2.p1()))

void addUniqueLine01(QPointF p, double& minY, double& maxY, QPointF& lastPoint, int& pixelDiff, QVector<QLineF>& lines, bool& prevPixelDiffZero);
void addUniqueLine02(QPointF p, double& minY, double& maxY, QPointF& lastPoint, int& pixelDiff, QVector<QLineF>& lines, bool& prevPixelDiffZero);
void addUniqueLine_double_vector(double* p, double& minY, double& maxY, QPointF& lastPoint, int& pixelDiff, QVector<QLineF>& lines, bool& prevPixelDiffZero);
void addUniqueLine_double_vector_last_point_vector(double* p,
												   double& minY,
												   double& maxY,
												   double* lastPoint,
												   int& pixelDiff,
												   QVector<QLineF>& lines,
												   bool& prevPixelDiffZero);
void addUniqueLine_double_vector_last_point_vector_lines_vector(double* p,
																double& minY,
																double& maxY,
																double* lastPoint,
																int& pixelDiff,
																double* lines,
																bool& prevPixelDiffZero,
																int& numberLines);
void addUniqueLine_double_vector_last_point_vector_lines_vector_add4(double* p,
																	 double& minY,
																	 double& maxY,
																	 double* lastPoint,
																	 int& pixelDiff,
																	 double* lines,
																	 bool& prevPixelDiffZero,
																	 int& numberLines);
void addUniqueLine_double_vector_last_point_vector_lines_vector_add4_dont_copy_last_point(double* p,
																						  double& minY,
																						  double& maxY,
																						  double** lastPoint,
																						  int& pixelDiff,
																						  double* lines,
																						  bool& prevPixelDiffZero,
																						  int& numberLines);

#define doublesToString(v1, v2, v3, v4) QString::number(v1) + "," + QString::number(v2) + "," + QString::number(v3) + "," + QString::number(v4)
#define lineToString(line) doublesToString(line.p1().x(), line.p1().y(), line.p2().x(), line.p2().y())

void XYCurveTest::addUniqueLineTest01() {
	//	// For performance Testing only

	//	/*
	//	 * Summary:
	//	 * - Using .at() is faster than []
	//	 * - Chaching calculations. So (qRound64(x / minLogicalDiffX)) must not be
	//	 * done every time.
	//	 * - Try to get as much as possible calculations away. instead of abs() use !=
	//	 * 0 which is secure for long int
	//	 */
	//	const int count = 100e6;
	//	const double maxValue = 100;
	//	const int numberPixel = 680;
	//	const double minLogicalDiffX = maxValue / numberPixel; // amount pixel = count/minLogicalDiffX
	//	const int maximumLines = 2 * numberPixel; // number Pixel horizontal lines, number Pixel vertical lines

	//	//	{
	//	//		QVector<QLineF> lines;
	//	//		bool prevPixelDiffZero = false;
	//	//		x = 0;
	//	//		PERFTRACE(QString(Q_FUNC_INFO) + "XYCurve::addUniqueLine use array
	//	// access"); 		for (int i=0; i < count; i++) { 			pixelDiff =
	//	// llabs(qRound64(points[i].x() / minLogicalDiffX) - qRound64(x /
	//	// minLogicalDiffX)) > 0; // only relevant if greater zero or not
	//	//			XYCurvePrivate::addUniqueLine(points[i], minY, maxY, lastPoint,
	//	// pixelDiff, lines, prevPixelDiffZero); 			if (pixelDiff > 0) // set x to next
	//	// pixel 				x += minLogicalDiffX;
	//	//		}
	//	//	}

	//	{
	//		double x = 0;
	//		double minY = INFINITY;
	//		double maxY = -INFINITY;
	//		QPointF lastPoint;
	//		int pixelDiff = 0;
	//		QVector<QPointF> points(count);
	//		for (int i = 0; i < count; i++) {
	//			points[i].setX(double(i) / count * maxValue);
	//			points[i].setY(sin(double(i) / count * 2. * 3.1416 * 20.));
	//		}
	//		QVector<QLineF> lines;
	//		{
	//			bool prevPixelDiffZero = false;
	//			x = 0;
	//			PERFTRACE(QString(Q_FUNC_INFO) + "01_XYCurve::addUniqueLine use @access");
	//			for (int i = 0; i < count; i++) {
	//				pixelDiff = llabs(qRound64(points.at(i).x() / minLogicalDiffX) - qRound64(x / minLogicalDiffX)) > 0; // only relevant if greater zero or not
	//				XYCurvePrivate::addUniqueLine(points.at(i), minY, maxY, lastPoint, pixelDiff, lines, prevPixelDiffZero);
	//				if (pixelDiff > 0) // set x to next pixel
	//					x += minLogicalDiffX;
	//			}
	//		}
	//		qDebug() << x;
	//		QFile f("01_addUniqueLine_use_at_access.txt");
	//		f.open(QIODevice::OpenModeFlag::WriteOnly);
	//		for (auto& line : lines) {
	//			QTextStream out(&f);
	//			out << lineToString(line) << "\n";
	//		}
	//		f.close();
	//	}

	//	{
	//		double x = 0;
	//		double minY = INFINITY;
	//		double maxY = -INFINITY;
	//		QPointF lastPoint;
	//		int pixelDiff = 0;
	//		QVector<QPointF> points(count);
	//		for (int i = 0; i < count; i++) {
	//			points[i].setX(double(i) / count * maxValue);
	//			points[i].setY(sin(double(i) / count * 2. * 3.1416 * 20.));
	//		}
	//		QVector<QLineF> lines;
	//		{
	//			qint64 r = 0;
	//			bool prevPixelDiffZero = false;
	//			x = 0;
	//			PERFTRACE(QString(Q_FUNC_INFO) +
	//                "011_XYCurve::addUniqueLine use @access try to simplify "
	//                "calculation, cache x/minLogicalDiffX");
	//			for (int i = 0; i < count; i++) {
	//				pixelDiff = (qRound64(points.at(i).x() / minLogicalDiffX) - r) != 0; // only relevant if greater zero or not
	//				XYCurvePrivate::addUniqueLine(points.at(i), minY, maxY, lastPoint, pixelDiff, lines, prevPixelDiffZero);
	//				if (pixelDiff > 0) { // set x to next pixel
	//					x += minLogicalDiffX;
	//					r = qRound64(x / minLogicalDiffX);
	//				}
	//			}
	//		}
	//		qDebug() << x;
	//		QFile f("011_addUniqueLine_use_at_access.txt");
	//		f.open(QIODevice::OpenModeFlag::WriteOnly);
	//		for (auto& line : lines) {
	//			QTextStream out(&f);
	//			out << lineToString(line) << "\n";
	//		}
	//		f.close();
	//	}

	//	//{
	//	//	double x = 0;
	//	//	double minY = INFINITY;
	//	//	double maxY = -INFINITY;
	//	//	QPointF lastPoint;
	//	//	int pixelDiff = 0;
	//	//	QVector<QPointF> points(count);
	//	//	for (int i = 0; i < count; i++) {
	//	//		points[i].setX(double(i)/count * maxValue);
	//	//		points[i].setY(sin(double(i)/count * 2. * 3.1416 * 20.));
	//	//	}
	//	//	QVector<qint64> pixelDiff_;

	//	//	// just to export pixelDiff
	//	//	QVector<QLineF> lines;
	//	//	{
	//	//		bool prevPixelDiffZero = false;
	//	//		x = 0;
	//	//		for (int i=0; i < count; i++) {
	//	//			qint64 res = llabs(qRound64(points.at(i).x() / minLogicalDiffX)
	//	//- qRound64(x / minLogicalDiffX)); 			pixelDiff_.append(res); 			pixelDiff = res >
	//	// 0; // only relevant if greater zero or not
	//	//			XYCurvePrivate::addUniqueLine(points.at(i), minY, maxY,
	//	// lastPoint, pixelDiff, lines, prevPixelDiffZero); 			if (pixelDiff > 0) // set x
	//	// to next pixel 				x += minLogicalDiffX;
	//	//		}
	//	//	}
	//	//	qDebug() << x;
	//	//	QFile f("PixelDiff.txt");
	//	//	f.open(QIODevice::OpenModeFlag::WriteOnly);
	//	//	for (auto& pd: pixelDiff_) {
	//	//		QTextStream out(&f);
	//	//		out << pd << "\n";
	//	//	}
	//	//	f.close();
	//	//}

	//	{
	//		double x = 0;
	//		double minY = INFINITY;
	//		double maxY = -INFINITY;
	//		QPointF lastPoint;
	//		int pixelDiff = 0;
	//		QPointF* pointerArray = (QPointF*)malloc(count * sizeof(QPointF));
	//		for (int i = 0; i < count; i++) {
	//			pointerArray[i].setX(double(i) / count * maxValue);
	//			pointerArray[i].setY(sin(double(i) / count * 2. * 3.1416 * 20.));
	//		}
	//		QVector<QLineF> lines;
	//		{
	//			bool prevPixelDiffZero = false;
	//			x = 0;
	//			PERFTRACE(QString(Q_FUNC_INFO) + "02_XYCurve::addUniqueLine use raw vector");
	//			for (int i = 0; i < count; i++) {
	// only relevant if greater zero or not
	//				pixelDiff = llabs(qRound64(pointerArray[i].x() / minLogicalDiffX) - qRound64(x / minLogicalDiffX)) > 0;
	//			addUniqueLine01(pointerArray[i], minY, maxY, lastPoint, pixelDiff, lines, prevPixelDiffZero);
	//          if (pixelDiff > 0) // set x to next pixel
	//             x += minLogicalDiffX;
	//			}
	//		}
	//		qDebug() << x;
	//		QFile f("02_addUniqueLine_use_raw_vector.txt");
	//		f.open(QIODevice::OpenModeFlag::WriteOnly);
	//		for (auto& line : lines) {
	//			QTextStream out(&f);
	//			out << lineToString(line) << "\n";
	//		}
	//		f.close();
	//		free(pointerArray);
	//	}

	//	{
	//		double x = 0;
	//		double minY = INFINITY;
	//		double maxY = -INFINITY;
	//		QPointF lastPoint;
	//		int pixelDiff = 0;
	//		double* pointerArray = (double*)malloc(count * 2 * sizeof(double));
	//		for (int i = 0; i < count; i++) {
	//			pointerArray[2 * i] = double(i) / count * maxValue;
	//			pointerArray[2 * i + 1] = sin(double(i) / count * 2. * 3.1416 * 20.);
	//		}
	//		QVector<QLineF> lines;
	//		{
	//			bool prevPixelDiffZero = false;
	//			PERFTRACE(QString(Q_FUNC_INFO) + "03_XYCurve::addUniqueLine use raw double vector");
	//			for (int i = 0; i < count; i++) {
	//				only relevant if greater zero or not
	//				pixelDiff = llabs(qRound64(pointerArray[2 * i] / minLogicalDiffX) - qRound64(x / minLogicalDiffX)) > 0;
	//			    addUniqueLine_double_vector(&pointerArray[2 * i], minY, maxY, lastPoint, pixelDiff, lines, prevPixelDiffZero);
	//				if (pixelDiff > 0) // set x to next pixel
	//  				x += minLogicalDiffX;
	//			}
	//		}
	//		qDebug() << x;
	//		QFile f("03_addUniqueLine_use_raw_double_vector.txt");
	//		f.open(QIODevice::OpenModeFlag::WriteOnly);
	//		for (auto& line : lines) {
	//			QTextStream out(&f);
	//			out << lineToString(line) << "\n";
	//		}
	//		f.close();
	//		free(pointerArray);
	//	}

	//	{
	//		double x = 0;
	//		double minY = INFINITY;
	//		double maxY = -INFINITY;
	//		int pixelDiff = 0;
	//		double* pointerArray = (double*)malloc(count * 2 * sizeof(double));
	//		double lastPoint[2] = {0};
	//		for (int i = 0; i < count; i++) {
	//			pointerArray[2 * i] = double(i) / count * maxValue;
	//			pointerArray[2 * i + 1] = sin(double(i) / count * 2. * 3.1416 * 20.);
	//		}
	//		QVector<QLineF> lines;
	//		{
	//			bool prevPixelDiffZero = false;
	//			PERFTRACE(QString(Q_FUNC_INFO) +
	//                "04_XYCurve::addUniqueLine use raw double vector last point "
	//                "double vector");
	//			for (int i = 0; i < count; i++) {
	//				pixelDiff = llabs(qRound64(pointerArray[2 * i] / minLogicalDiffX) - qRound64(x / minLogicalDiffX)) > 0; // only relevant if greater zero or
	// not 				addUniqueLine_double_vector_last_point_vector(&pointerArray[2 * i], minY, maxY, lastPoint, pixelDiff, lines, prevPixelDiffZero);
	// if (pixelDiff > 0) // set x to next pixel 					x += minLogicalDiffX;
	//			}
	//		}
	//		qDebug() << x;
	//		QFile f("04_addUniqueLine_use_raw_double_vector_last_point_double_vector.txt");
	//		f.open(QIODevice::OpenModeFlag::WriteOnly);
	//		for (auto& line : lines) {
	//			QTextStream out(&f);
	//			out << lineToString(line) << "\n";
	//		}
	//		f.close();
	//		free(pointerArray);
	//	}

	//	{
	//		double x = 0;
	//		double minY = INFINITY;
	//		double maxY = -INFINITY;
	//		int pixelDiff = 0;
	//		double* pointerArray = (double*)malloc(count * 2 * sizeof(double));
	//		double lastPoint[2] = {0};
	//		for (int i = 0; i < count; i++) {
	//			pointerArray[2 * i] = double(i) / count * maxValue;
	//			pointerArray[2 * i + 1] = sin(double(i) / count * 2. * 3.1416 * 20.);
	//		}
	//		QVector<QLineF> lines2;
	//		{
	//			bool prevPixelDiffZero = false;
	//			int number_lines = 0;
	//			double* lines = (double*)malloc(maximumLines * 4 * sizeof(double));
	//			PERFTRACE(QString(Q_FUNC_INFO) +
	//                "05_XYCurve::addUniqueLine use raw double vector "
	//                "last_point_double lines_double_vector");
	//			for (int i = 0; i < count; i++) {
	//				pixelDiff = llabs(qRound64(pointerArray[2 * i] / minLogicalDiffX) - qRound64(x / minLogicalDiffX)) > 0; // only relevant if greater zero or
	// not 				addUniqueLine_double_vector_last_point_vector_lines_vector(&pointerArray[2 * i],
	// minY, maxY, lastPoint, 																		   pixelDiff, lines, prevPixelDiffZero,
	// number_lines); 				if (pixelDiff > 0) // set x to next pixel 					x += minLogicalDiffX;
	//			}

	//			for (int i = 0; i < number_lines; i++)
	//				lines2.append(QLineF(lines[i * 4], lines[i * 4 + 1], lines[i * 4 + 2], lines[i * 4 + 3]));
	//			free(lines);
	//		}
	//		qDebug() << x;
	//		QFile f(
	//			"05_addUniqueLine_use_raw_double_vector_last_point_double_vector_"
	//			"lines_double_vector.txt");
	//		f.open(QIODevice::OpenModeFlag::WriteOnly);
	//		for (auto& line : lines2) {
	//			QTextStream out(&f);
	//			out << lineToString(line) << "\n";
	//		}
	//		f.close();
	//		free(pointerArray);
	//	}

	//	{
	//		double x = 0;
	//		double minY = INFINITY;
	//		double maxY = -INFINITY;
	//		int pixelDiff = 0;
	//		double* pointerArray = (double*)malloc(count * 2 * sizeof(double));
	//		double lastPoint[2] = {0};
	//		for (int i = 0; i < count; i++) {
	//			pointerArray[2 * i] = double(i) / count * maxValue;
	//			pointerArray[2 * i + 1] = sin(double(i) / count * 2. * 3.1416 * 20.);
	//		}
	//		qint64 r = 0;
	//		QVector<QLineF> lines2;
	//		{
	//			bool prevPixelDiffZero = false;
	//			int number_lines = 0;
	//			double* lines = (double*)malloc(maximumLines * 4 * sizeof(double));
	//			PERFTRACE(QString(Q_FUNC_INFO) + "051_XYCurve::addUniqueLine use raw "
	//                                       "double vector cache x/minLogicalDiffX");
	//			for (int i = 0; i < count; i++) {
	//				pixelDiff = llabs(qRound64(pointerArray[2 * i] / minLogicalDiffX) - r) > 0; // only relevant if greater zero or not
	//				addUniqueLine_double_vector_last_point_vector_lines_vector(&pointerArray[2 * i],
	//																		   minY,
	//																		   maxY,
	//																		   lastPoint,
	//																		   pixelDiff,
	//																		   lines,
	//																		   prevPixelDiffZero,
	//																		   number_lines);
	//				if (pixelDiff > 0) { // set x to next pixel
	//					x += minLogicalDiffX;
	//					r = qRound64(x / minLogicalDiffX);
	//				}
	//			}

	//			for (int i = 0; i < number_lines; i++)
	//				lines2.append(QLineF(lines[i * 4], lines[i * 4 + 1], lines[i * 4 + 2], lines[i * 4 + 3]));
	//			free(lines);
	//		}
	//		qDebug() << x;
	//		QFile f(
	//			"051_addUniqueLine_use_raw_double_vector_last_point_double_vector_"
	//			"lines_double_vector_cache_x_minLogicalDiffX.txt");
	//		f.open(QIODevice::OpenModeFlag::WriteOnly);
	//		for (auto& line : lines2) {
	//			QTextStream out(&f);
	//			out << lineToString(line) << "\n";
	//		}
	//		f.close();
	//		free(pointerArray);
	//	}

	//	{
	//		double x = 0;
	//		double minY = INFINITY;
	//		double maxY = -INFINITY;
	//		int pixelDiff = 0;
	//		double* pointerArray = (double*)malloc(count * 2 * sizeof(double));
	//		double lastPoint[2] = {0};
	//		for (int i = 0; i < count; i++) {
	//			pointerArray[2 * i] = double(i) / count * maxValue;
	//			pointerArray[2 * i + 1] = sin(double(i) / count * 2. * 3.1416 * 20.);
	//		}
	//		qint64 r = 0;
	//		QVector<QLineF> lines2;
	//		{
	//			bool prevPixelDiffZero = false;
	//			int number_lines = 0;
	//			double* lines = (double*)malloc(maximumLines * 4 * sizeof(double));
	//			PERFTRACE(QString(Q_FUNC_INFO) +
	//                "0511_XYCurve::addUniqueLine use raw double vector cache "
	//                "x/minLogicalDiffX remove abs");
	//			for (int i = 0; i < count; i++) {
	//				pixelDiff = (qRound64(pointerArray[2 * i] / minLogicalDiffX) - r) != 0; // only relevant if not zero
	//				addUniqueLine_double_vector_last_point_vector_lines_vector(&pointerArray[2 * i],
	//																		   minY,
	//																		   maxY,
	//																		   lastPoint,
	//																		   pixelDiff,
	//																		   lines,
	//																		   prevPixelDiffZero,
	//																		   number_lines);
	//				if (pixelDiff > 0) { // set x to next pixel
	//					x += minLogicalDiffX;
	//					r = qRound64(x / minLogicalDiffX);
	//				}
	//			}

	//			for (int i = 0; i < number_lines; i++)
	//				lines2.append(QLineF(lines[i * 4], lines[i * 4 + 1], lines[i * 4 + 2], lines[i * 4 + 3]));
	//			free(lines);
	//		}
	//		qDebug() << x;
	//		QFile f(
	//			"0511_addUniqueLine_use_raw_double_vector_last_point_double_vector_"
	//			"lines_double_vector_cache_x_minLogicalDiffX.txt");
	//		f.open(QIODevice::OpenModeFlag::WriteOnly);
	//		for (auto& line : lines2) {
	//			QTextStream out(&f);
	//			out << lineToString(line) << "\n";
	//		}
	//		f.close();
	//		free(pointerArray);
	//	}

	//	//	{
	//	//		double x = 0;
	//	//		double minY = INFINITY;
	//	//		double maxY = -INFINITY;
	//	//		int pixelDiff = 0;
	//	//		double* pointerArray = (double*)malloc(count * 2 *
	//	// sizeof(double)); 		double lastPoint[2] = {0}; 		for (int i = 0; i < count; i++)
	//	//{ 			pointerArray[2*i] = double(i)/count * maxValue; 			pointerArray[2*i + 1] =
	//	// sin(double(i)/count * 2. * 3.1416 * 20.);
	//	//		}
	//	//		qint64 r = 0;
	//	//		int number_lines = 0;
	//	//		double* lines = (double*)malloc(maximumLines * 4 *
	//	// sizeof(double));
	//	//		{
	//	//			bool prevPixelDiffZero = false;
	//	//			PERFTRACE(QString(Q_FUNC_INFO) + "052_XYCurve::addUniqueLine use
	//	// raw double vector cache x/minLogicalDiffX without linecopy"); 			for (int i=0;
	//	// i < count; i++) { 				pixelDiff = llabs(qRound64(pointerArray[2*i] /
	//	// minLogicalDiffX) - r) > 0; // only relevant if greater zero or not
	//	//				addUniqueLine_double_vector_last_point_vector_lines_vector(&pointerArray[2*i],
	//	// minY, maxY, lastPoint, pixelDiff, lines, prevPixelDiffZero, number_lines);
	//	//				if (pixelDiff > 0) { // set x to next pixel
	//	//					x += minLogicalDiffX;
	//	//					r = qRound64(x / minLogicalDiffX);
	//	//				}
	//	//			}
	//	//		}
	//	//		qDebug() << x;
	//	//		QFile
	//	// f("052_addUniqueLine_use_raw_double_vector_last_point_double_vector_lines_double_vector_cache_x_minLogicalDiffX_without_line_copy.txt");
	//	//		f.open(QIODevice::OpenModeFlag::WriteOnly);
	//	//		for (int i=0; i < number_lines; i++) {
	//	//			QTextStream out(&f);
	//	//			out << doublesToString(lines[i*4], lines[i*4 + 1], lines[i*4 +
	//	// 2], lines[i*4 + 3]) << "\n";
	//	//		}
	//	//		f.close();
	//	//		free(lines);
	//	//		free(pointerArray);
	//	//	}

	//	{
	//		double x = 0;
	//		double minY = INFINITY;
	//		double maxY = -INFINITY;
	//		int pixelDiff = 0;
	//		double* pointerArray = (double*)malloc(count * 2 * sizeof(double));
	//		double lastPoint[2] = {0};
	//		for (int i = 0; i < count; i++) {
	//			pointerArray[2 * i] = double(i) / count * maxValue;
	//			pointerArray[2 * i + 1] = sin(double(i) / count * 2. * 3.1416 * 20.);
	//		}
	//		qint64 r = 0;
	//		int number_lines = 0;
	//		double* lines = (double*)malloc(maximumLines * 4 * sizeof(double));
	//		{
	//			bool prevPixelDiffZero = false;
	//			PERFTRACE(
	//          QString(Q_FUNC_INFO) +
	//          "053_XYCurve::addUniqueLine use raw double vector cache "
	//          "x/minLogicalDiffX without linecopy number_lines add 4 directly");
	//			for (int i = 0; i < count; i++) {
	//				pixelDiff = llabs(qRound64(pointerArray[2 * i] / minLogicalDiffX) - r) > 0; // only relevant if greater zero or not
	//				addUniqueLine_double_vector_last_point_vector_lines_vector_add4(&pointerArray[2 * i],
	//																				minY,
	//																				maxY,
	//																				lastPoint,
	//																				pixelDiff,
	//																				lines,
	//																				prevPixelDiffZero,
	//																				number_lines);
	//				if (pixelDiff > 0) { // set x to next pixel
	//					x += minLogicalDiffX;
	//					r = qRound64(x / minLogicalDiffX);
	//				}
	//			}
	//		}
	//		qDebug() << x;
	//		QFile f("053.txt");
	//		f.open(QIODevice::OpenModeFlag::WriteOnly);
	//		for (int i = 0; i < number_lines; i++) {
	//			QTextStream out(&f);
	//			out << doublesToString(lines[i * 4], lines[i * 4 + 1], lines[i * 4 + 2], lines[i * 4 + 3]) << "\n";
	//		}
	//		f.close();
	//		free(lines);
	//		free(pointerArray);
	//	}

	//	//	// Does not work. Get segmentation fault
	//	//	{
	//	//		double x = 0;
	//	//		double minY = INFINITY;
	//	//		double maxY = -INFINITY;
	//	//		int pixelDiff = 0;
	//	//		double* pointerArray = (double*)malloc(count * 2 *
	//	// sizeof(double)); 		double *lastPoint = nullptr; 		for (int i = 0; i < count;
	//	// i++) { 			pointerArray[2*i] = double(i)/count * maxValue; 			pointerArray[2*i + 1]
	//	//= sin(double(i)/count * 2. * 3.1416 * 20.);
	//	//		}
	//	//		qint64 r = 0;
	//	//		int number_lines = 0;
	//	//		double* lines = (double*)malloc(maximumLines * 4 *
	//	// sizeof(double));
	//	//		{
	//	//			bool prevPixelDiffZero = false;
	//	//			PERFTRACE(QString(Q_FUNC_INFO) + "06_XYCurve::addUniqueLine use
	//	// raw double vector cache x/minLogicalDiffX without linecopy number_lines add
	//	// 4 directly don't copy last point"); 			for (int i=0; i < count; i++) {
	//	//				pixelDiff = llabs(qRound64(pointerArray[2*i] /
	//	// minLogicalDiffX) - r) > 0; // only relevant if greater zero or not
	//	//				addUniqueLine_double_vector_last_point_vector_lines_vector_add4_dont_copy_last_point(&pointerArray[2*i],
	//	// minY, maxY, &lastPoint, pixelDiff, lines, prevPixelDiffZero, number_lines);
	//	//				if (pixelDiff > 0) { // set x to next pixel
	//	//					x += minLogicalDiffX;
	//	//					r = qRound64(x / minLogicalDiffX);
	//	//				}
	//	//			}
	//	//		}
	//	//		qDebug() << x;
	//	//		QFile f("06.txt");
	//	//		f.open(QIODevice::OpenModeFlag::WriteOnly);
	//	//		for (int i=0; i < number_lines; i++) {
	//	//			QTextStream out(&f);
	//	//			out << doublesToString(lines[i*4], lines[i*4 + 1], lines[i*4 +
	//	// 2], lines[i*4 + 3]) << "\n";
	//	//		}
	//	//		f.close();
	//	//		free(lines);
	//	//		free(pointerArray);
	//	//	}

	//	//	{
	//	//		QVector<QLineF> lines;
	//	//		PERFTRACE(QString(Q_FUNC_INFO) + "Add to line");
	//	//		for (int i=0; i < maximumLines; i++) {
	//	//			lines.append(QLineF(points.at(i), points.at(i+1)));
	//	//		}
	//	//		(void)lines;
	//	//	}

	//	//	{
	//	//		QVector<QLineF> lines;
	//	//		bool prevPixelDiffZero = false;
	//	//		// Reallocating array
	//	//		x = 0;
	//	//		lines.resize(maximumLines);
	//	//	PERFTRACE(QString(Q_FUNC_INFO) + "addUniqueLine PreAllocate");
	//	//		for (int i=0; i < count; i++) {
	//	//			pixelDiff = llabs(qRound64(points.at(i).x() / minLogicalDiffX) -
	//	// qRound64(x / minLogicalDiffX)) > 0; // only relevant if greater zero or not
	//	//			XYCurvePrivate::addUniqueLine(points.at(i), minY, maxY,
	//	// lastPoint, pixelDiff, lines, prevPixelDiffZero); 			if (pixelDiff > 0) // set x
	//	// to next pixel 				x += minLogicalDiffX;
	//	//		}
	//	//	}
}

void addUniqueLine01(QPointF p, double& minY, double& maxY, QPointF& lastPoint, int& pixelDiff, QVector<QLineF>& lines, bool& prevPixelDiffZero) {
	if (pixelDiff == 0) {
		maxY = qMax(p.y(), maxY);
		minY = qMin(p.y(), minY);
		prevPixelDiffZero = true;
		lastPoint = p;
	} else {
		if (prevPixelDiffZero) {
			if (maxY != minY)
				lines.append(QLineF(lastPoint.x(), minY, lastPoint.x(), maxY));
			lines.append(QLineF(lastPoint, p));
		} else if (!isnan(lastPoint.x()) && !isnan(lastPoint.y()))
			lines.append(QLineF(lastPoint, p));
		prevPixelDiffZero = false;
		minY = p.y();
		maxY = p.y();
		lastPoint = p;
	}
}

void addUniqueLine_double_vector(double* p, double& minY, double& maxY, QPointF& lastPoint, int& pixelDiff, QVector<QLineF>& lines, bool& prevPixelDiffZero) {
	if (pixelDiff == 0) {
		maxY = qMax(p[1], maxY);
		minY = qMin(p[1], minY);
		prevPixelDiffZero = true;
		lastPoint = QPointF(p[0], p[1]);
	} else {
		if (prevPixelDiffZero) {
			if (maxY != minY)
				lines.append(QLineF(lastPoint.x(), minY, lastPoint.x(), maxY));
			lines.append(QLineF(lastPoint, QPointF(p[0], p[1])));
		} else if (!isnan(lastPoint.x()) && !isnan(lastPoint.y()))
			lines.append(QLineF(lastPoint, QPointF(p[0], p[1])));
		prevPixelDiffZero = false;
		minY = p[1];
		maxY = p[1];
		lastPoint = QPointF(p[0], p[1]);
	}
}

void addUniqueLine_double_vector_last_point_vector(double* p,
												   double& minY,
												   double& maxY,
												   double* lastPoint,
												   int& pixelDiff,
												   QVector<QLineF>& lines,
												   bool& prevPixelDiffZero) {
	if (pixelDiff == 0) {
		maxY = qMax(p[1], maxY);
		minY = qMin(p[1], minY);
		prevPixelDiffZero = true;
		// lastPoint = QPointF(p[0], p[1]);
		lastPoint[0] = p[0];
		lastPoint[1] = p[1];
	} else {
		if (prevPixelDiffZero) {
			if (maxY != minY)
				lines.append(QLineF(lastPoint[0], minY, lastPoint[0], maxY));
			lines.append(QLineF(QPointF(lastPoint[0], lastPoint[1]), QPointF(p[0], p[1])));
		} else if (!isnan(lastPoint[0]) && !isnan(lastPoint[1]))
			lines.append(QLineF(QPointF(lastPoint[0], lastPoint[1]), QPointF(p[0], p[1])));
		prevPixelDiffZero = false;
		minY = p[1];
		maxY = p[1];
		lastPoint[0] = p[0];
		lastPoint[1] = p[1];
	}
}

void addUniqueLine_double_vector_last_point_vector_lines_vector(double* p,
																double& minY,
																double& maxY,
																double* lastPoint,
																int& pixelDiff,
																double* lines,
																bool& prevPixelDiffZero,
																int& numberLines) {
	if (pixelDiff == 0) {
		maxY = qMax(p[1], maxY);
		minY = qMin(p[1], minY);
		prevPixelDiffZero = true;
		// lastPoint = QPointF(p[0], p[1]);
		lastPoint[0] = p[0];
		lastPoint[1] = p[1];
	} else {
		if (prevPixelDiffZero) {
			if (maxY != minY) {
				lines[4 * numberLines] = lastPoint[0];
				lines[4 * numberLines + 1] = minY;
				lines[4 * numberLines + 2] = lastPoint[0];
				lines[4 * numberLines + 3] = maxY;
				// lines.append(QLineF(lastPoint[0], minY, lastPoint[0], maxY));
				numberLines++;
			}
			lines[4 * numberLines] = lastPoint[0];
			lines[4 * numberLines + 1] = lastPoint[1];
			lines[4 * numberLines + 2] = p[0];
			lines[4 * numberLines + 3] = p[1];
			numberLines++;
			// lines.append(QLineF(QPointF(lastPoint[0], lastPoint[1]), QPointF(p[0],
			// p[1])));
		} else if (!isnan(lastPoint[0]) && !isnan(lastPoint[1])) {
			// lines.append(QLineF(QPointF(lastPoint[0], lastPoint[1]), QPointF(p[0],
			// p[1])));
			lines[4 * numberLines] = lastPoint[0];
			lines[4 * numberLines + 1] = lastPoint[1];
			lines[4 * numberLines + 2] = p[0];
			lines[4 * numberLines + 3] = p[1];
			numberLines++;
		}
		prevPixelDiffZero = false;
		minY = p[1];
		maxY = p[1];
		lastPoint[0] = p[0];
		lastPoint[1] = p[1];
	}
}

void addUniqueLine_double_vector_last_point_vector_lines_vector_add4(double* p,
																	 double& minY,
																	 double& maxY,
																	 double* lastPoint,
																	 int& pixelDiff,
																	 double* lines,
																	 bool& prevPixelDiffZero,
																	 int& numberLines) {
	if (pixelDiff == 0) {
		maxY = qMax(p[1], maxY);
		minY = qMin(p[1], minY);
		prevPixelDiffZero = true;
		// lastPoint = QPointF(p[0], p[1]);
		lastPoint[0] = p[0];
		lastPoint[1] = p[1];
	} else {
		if (prevPixelDiffZero) {
			if (maxY != minY) {
				lines[numberLines] = lastPoint[0];
				lines[numberLines + 1] = minY;
				lines[numberLines + 2] = lastPoint[0];
				lines[numberLines + 3] = maxY;
				// lines.append(QLineF(lastPoint[0], minY, lastPoint[0], maxY));
				numberLines += 4;
			}
			lines[numberLines] = lastPoint[0];
			lines[numberLines + 1] = lastPoint[1];
			lines[numberLines + 2] = p[0];
			lines[numberLines + 3] = p[1];
			numberLines += 4;
			// lines.append(QLineF(QPointF(lastPoint[0], lastPoint[1]), QPointF(p[0],
			// p[1])));
		} else if (!isnan(lastPoint[0]) && !isnan(lastPoint[1])) {
			// lines.append(QLineF(QPointF(lastPoint[0], lastPoint[1]), QPointF(p[0],
			// p[1])));
			lines[numberLines] = lastPoint[0];
			lines[numberLines + 1] = lastPoint[1];
			lines[numberLines + 2] = p[0];
			lines[numberLines + 3] = p[1];
			numberLines += 4;
		}
		prevPixelDiffZero = false;
		minY = p[1];
		maxY = p[1];
		lastPoint[0] = p[0];
		lastPoint[1] = p[1];
	}
}

void addUniqueLine_double_vector_last_point_vector_lines_vector_add4_dont_copy_last_point(double* p,
																						  double& minY,
																						  double& maxY,
																						  double** lastPoint,
																						  int& pixelDiff,
																						  double* lines,
																						  bool& prevPixelDiffZero,
																						  int& numberLines) {
	if (pixelDiff == 0) {
		maxY = qMax(p[1], maxY);
		minY = qMin(p[1], minY);
		prevPixelDiffZero = true;
		// lastPoint = QPointF(p[0], p[1]);
		lastPoint = &p;
	} else {
		if (prevPixelDiffZero) {
			if (maxY != minY) {
				lines[numberLines] = (*lastPoint)[0];
				lines[numberLines + 1] = minY;
				lines[numberLines + 2] = (*lastPoint)[0];
				lines[numberLines + 3] = maxY;
				// lines.append(QLineF(lastPoint[0], minY, lastPoint[0], maxY));
				numberLines += 4;
			}
			lines[numberLines] = (*lastPoint)[0];
			lines[numberLines + 1] = (*lastPoint)[1];
			lines[numberLines + 2] = p[0];
			lines[numberLines + 3] = p[1];
			numberLines += 4;
			// lines.append(QLineF(QPointF(lastPoint[0], lastPoint[1]), QPointF(p[0],
			// p[1])));
		} else if (!isnan((*lastPoint)[0]) && !isnan((*lastPoint)[1])) {
			// lines.append(QLineF(QPointF(lastPoint[0], lastPoint[1]), QPointF(p[0],
			// p[1])));
			lines[numberLines] = (*lastPoint)[0];
			lines[numberLines + 1] = (*lastPoint)[1];
			lines[numberLines + 2] = p[0];
			lines[numberLines + 3] = p[1];
			numberLines += 4;
		}
		prevPixelDiffZero = false;
		minY = p[1];
		maxY = p[1];
		lastPoint = &p;
	}
}

void addUniqueLine02(QPointF p, double x, double& minY, double& maxY, QPointF& lastPoint, int& pixelDiff, QVector<QLineF>& lines) {
	static bool prevPixelDiffZero = false;
	static int i = 0;
	if (pixelDiff == 0) {
		maxY = qMax(p.y(), maxY);
		minY = qMin(p.y(), minY);
		lastPoint = p; // save last point
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

	QVector<QLineF> refLines{
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
	QVector<QLineF> refLines = {
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
	QVector<QLineF> refLines = {
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
	QVector<QLineF> refLines = {
		QLineF(QPointF(1, 1), QPointF(1.5, 1)),		QLineF(QPointF(1.5, 1), QPointF(1.5, 2)),	QLineF(QPointF(1.5, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2.5, 2)),		QLineF(QPointF(2.5, 2), QPointF(2.5, 3)),	QLineF(QPointF(2.5, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3.5, 3)),		QLineF(QPointF(3.5, 3), QPointF(3.5, 7)),	QLineF(QPointF(3.5, 7), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(4.5, 7)),		QLineF(QPointF(4.5, 7), QPointF(4.5, 15)),	QLineF(QPointF(4.5, 15), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(5.5, 15)),	QLineF(QPointF(5.5, 15), QPointF(5.5, 3)),	QLineF(QPointF(5.5, 3), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(6.5, 3)),		QLineF(QPointF(6.5, 3), QPointF(6.5, -10)), QLineF(QPointF(6.5, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7.5, -10)), QLineF(QPointF(7.5, -10), QPointF(7.5, 0)), QLineF(QPointF(7.5, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8.5, 0)),		QLineF(QPointF(8.5, 0), QPointF(8.5, 5)),	QLineF(QPointF(8.5, 5), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9.5, 5)),		QLineF(QPointF(9.5, 5), QPointF(9.5, 8)),	QLineF(QPointF(9.5, 8), QPointF(10, 8)),

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
	QVector<QLineF> refLines = {
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
		QLineF(QPointF(6, 9), QPointF(6, -3.5)),

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
	QVector<QLineF> refLines = {
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
	QVector<QLineF> refLines = {
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

	QVector<QLineF> refLines{
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
	QVector<QLineF> refLines = {
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

void XYCurveTest::updateLinesNoGapStartVerticalLastVertical() {
	LOAD_PROJECT

	lastVerticalCurve->setLineType(XYCurve::LineType::StartVertical);
	QVector<QLineF> refLines = {
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
	QVector<QLineF> refLines = {
		QLineF(QPointF(1, 1), QPointF(1.5, 1)),		QLineF(QPointF(1.5, 1), QPointF(1.5, 2)),	QLineF(QPointF(1.5, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2.5, 2)),		QLineF(QPointF(2.5, 2), QPointF(2.5, 3)),	QLineF(QPointF(2.5, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3.5, 3)),		QLineF(QPointF(3.5, 3), QPointF(3.5, 7)),	QLineF(QPointF(3.5, 7), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(4.5, 7)),		QLineF(QPointF(4.5, 7), QPointF(4.5, 15)),	QLineF(QPointF(4.5, 15), QPointF(5, 15)),
		QLineF(QPointF(5, 15), QPointF(5.5, 15)),	QLineF(QPointF(5.5, 15), QPointF(5.5, 3)),	QLineF(QPointF(5.5, 3), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(6.5, 3)),		QLineF(QPointF(6.5, 3), QPointF(6.5, -10)), QLineF(QPointF(6.5, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7.5, -10)), QLineF(QPointF(7.5, -10), QPointF(7.5, 0)), QLineF(QPointF(7.5, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8.5, 0)),		QLineF(QPointF(8.5, 0), QPointF(8.5, 5)),	QLineF(QPointF(8.5, 5), QPointF(9, 5)),
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
	QVector<QLineF> refLines = {
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
		QLineF(QPointF(6, 9), QPointF(6, -3.5)),

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
	QVector<QLineF> refLines = {
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
	QVector<QLineF> refLines = {
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

	QVector<QLineF> refLines{
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

	QVector<QLineF> refLines{
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
	QVector<QLineF> refLines = {
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
	QVector<QLineF> refLines = {
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
	QVector<QLineF> refLines = {
		QLineF(QPointF(1, 1), QPointF(1.5, 1)),		QLineF(QPointF(1.5, 1), QPointF(1.5, 2)),	QLineF(QPointF(1.5, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2.5, 2)),		QLineF(QPointF(2.5, 2), QPointF(2.5, 3)),	QLineF(QPointF(2.5, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3.5, 3)),		QLineF(QPointF(3.5, 3), QPointF(3.5, 7)),	QLineF(QPointF(3.5, 7), QPointF(4, 7)),
		QLineF(QPointF(4, 7), QPointF(5, 7)),		QLineF(QPointF(5, 7), QPointF(5, 3)),		QLineF(QPointF(5, 3), QPointF(6, 3)),
		QLineF(QPointF(6, 3), QPointF(6.5, 3)),		QLineF(QPointF(6.5, 3), QPointF(6.5, -10)), QLineF(QPointF(6.5, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7.5, -10)), QLineF(QPointF(7.5, -10), QPointF(7.5, 0)), QLineF(QPointF(7.5, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8.5, 0)),		QLineF(QPointF(8.5, 0), QPointF(8.5, 5)),	QLineF(QPointF(8.5, 5), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9.5, 5)),		QLineF(QPointF(9.5, 5), QPointF(9.5, 8)),	QLineF(QPointF(9.5, 8), QPointF(10, 8)),

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
	QVector<QLineF> refLines = {
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
		QLineF(QPointF(6, 5), QPointF(6, -3.5)), // vertical

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
	// QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		DEBUG(i);
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapLineSkipSegments2() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::Segments2);
	QVector<QLineF> refLines = {
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
	QVector<QLineF> refLines = {
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

	QVector<QLineF> refLines{
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
	QVector<QLineF> refLines = {
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
	// QCOMPARE(refLines.size(), test_lines.size());
	for (int i = 0; i < test_lines.size(); i++) {
		DEBUG(i)
		COMPARE_LINES(test_lines.at(i), refLines.at(i));
	}
}

void XYCurveTest::updateLinesWithGapStartVertical() {
	LOAD_PROJECT

	withGapCurve2->setLineType(XYCurve::LineType::StartVertical);
	QVector<QLineF> refLines = {
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
	QVector<QLineF> refLines = {
		QLineF(QPointF(1, 1), QPointF(1.5, 1)),		QLineF(QPointF(1.5, 1), QPointF(1.5, 2)),	QLineF(QPointF(1.5, 2), QPointF(2, 2)),
		QLineF(QPointF(2, 2), QPointF(2.5, 2)),		QLineF(QPointF(2.5, 2), QPointF(2.5, 3)),	QLineF(QPointF(2.5, 3), QPointF(3, 3)),
		QLineF(QPointF(3, 3), QPointF(3.5, 3)),		QLineF(QPointF(3.5, 3), QPointF(3.5, 7)),	QLineF(QPointF(3.5, 7), QPointF(4, 7)),
		QLineF(QPointF(6, 3), QPointF(6.5, 3)),		QLineF(QPointF(6.5, 3), QPointF(6.5, -10)), QLineF(QPointF(6.5, -10), QPointF(7, -10)),
		QLineF(QPointF(7, -10), QPointF(7.5, -10)), QLineF(QPointF(7.5, -10), QPointF(7.5, 0)), QLineF(QPointF(7.5, 0), QPointF(8, 0)),
		QLineF(QPointF(8, 0), QPointF(8.5, 0)),		QLineF(QPointF(8.5, 0), QPointF(8.5, 5)),	QLineF(QPointF(8.5, 5), QPointF(9, 5)),
		QLineF(QPointF(9, 5), QPointF(9.5, 5)),		QLineF(QPointF(9.5, 5), QPointF(9.5, 8)),	QLineF(QPointF(9.5, 8), QPointF(10, 8)),

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
	QVector<QLineF> refLines = {
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
		QLineF(QPointF(6, 3), QPointF(6, -3.5)),

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
	QVector<QLineF> refLines = {
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
	QVector<QLineF> refLines = {
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

//############################################################################
// Hover tests
//############################################################################
void XYCurveTest::hooverCurveIntegerEndingZeros() {
	LOAD_HOVER_PROJECT

	QPointF mouseLogicalPos(13, 29.1); // extracted from the spreadsheet
	bool visible;
	auto mouseScenePos = plot->coordinateSystem(integerNonMonotonic->coordinateSystemIndex())->mapLogicalToScene(mouseLogicalPos, visible);
	QCOMPARE(integerNonMonotonic->activateCurve(mouseScenePos, -1), true);
}

QTEST_MAIN(XYCurveTest)
