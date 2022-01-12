/*
    File                 : CartesianPlotTest.cpp
    Project              : LabPlot
    Description          : Tests for cartesian plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianPlotTest.h"

#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/matrix/Matrix.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QAction>

void CartesianPlotTest::initTestCase() {
//	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
//	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

//##############################################################################
//#####################  import of LabPlot projects ############################
//##############################################################################

#define LOAD_PROJECT_DATA_CHANGE \
	Project project; \
	project.load(QFINDTESTDATA(QLatin1String("data/TestDataChange.lml"))); \
	/* check the project tree for the imported project */ \
\
	/* Spreadsheet */ \
	auto* aspect = project.child<AbstractAspect>(0); \
	QVERIFY(aspect != nullptr); \
	if (aspect) \
		QCOMPARE(aspect->name(), QLatin1String("Spreadsheet")); \
	QVERIFY(aspect->type() == AspectType::Spreadsheet); \
	auto s = dynamic_cast<Spreadsheet*>(aspect); \
	if (!s) return; \
	auto c1 = static_cast<Column*>(s->child<Column>(0)); \
	QVERIFY(c1 != nullptr); \
	QCOMPARE(c1->name(), QLatin1String("1")); \
	QVERIFY(c1->columnMode() == AbstractColumn::ColumnMode::Double); \
	QVERIFY(c1->rowCount() == 100); \
	QVERIFY(c1->valueAt(0) == 1.); \
	QVERIFY(c1->valueAt(1) == 2.); \
	auto c2 = static_cast<Column*>(s->child<Column>(1)); \
	QVERIFY(c2 != nullptr); \
	QCOMPARE(c2->name(), QLatin1String("2")); \
	QVERIFY(c2->columnMode() == AbstractColumn::ColumnMode::Double); \
	QVERIFY(c2->rowCount() == 100); \
	QVERIFY(c2->valueAt(0) == 1.); \
	QVERIFY(c2->valueAt(1) == 2.); \
\
	/* Worksheet */ \
	aspect = project.child<AbstractAspect>(1); \
	QVERIFY(aspect != nullptr); \
	if (aspect) \
		QCOMPARE(aspect->name(), QLatin1String("Worksheet - Spreadsheet")); \
	QVERIFY(aspect->type() == AspectType::Worksheet); \
	auto w = dynamic_cast<Worksheet*>(aspect); \
	if (!w) return; \
 \
	auto plot = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(0)); \
	QVERIFY(plot != nullptr); \
	if (!plot) return; \
\
	/* curve */ \
	auto curve = dynamic_cast<XYCurve*>(plot->child<XYCurve>(0)); \
	QVERIFY(curve != nullptr); \
	if (!curve) return; \
	QCOMPARE(curve->name(), "2"); \
\
	CHECK_RANGE(plot, curve, x, 1, 2); \
	CHECK_RANGE(plot, curve, y, 1, 2); \
\
	auto xAxis = static_cast<Axis*>(plot->child<Axis>(0)); \
	QVERIFY(xAxis != nullptr); \
	QCOMPARE(xAxis->orientation() == Axis::Orientation::Horizontal, true); \
	\
	auto yAxis = static_cast<Axis*>(plot->child<Axis>(2)); \
	QVERIFY(yAxis != nullptr); \
	QCOMPARE(yAxis->orientation() == Axis::Orientation::Vertical, true);

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

/*!
 * \brief CartesianPlotTest::changeData1: add data point 
 *
 */

void CartesianPlotTest::changeData1() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c1->setValueAt(2, 3.);
	c2->setValueAt(2, 3.);

	QVERIFY(c1->valueAt(2) == 3.);
	QVERIFY(c2->valueAt(2) == 3.);

	CHECK_RANGE(plot, curve, x, 1, 3);
	CHECK_RANGE(plot, curve, y, 1, 3);
}

void CartesianPlotTest::changeData2() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c1->setValueAt(2, 3.);
	c2->setValueAt(2, 2.);

	QVERIFY(c1->valueAt(2) == 3.);
	QVERIFY(c2->valueAt(2) == 2.);

	CHECK_RANGE(plot, curve, x, 1, 3);
	CHECK_RANGE(plot, curve, y, 1, 2);

	DEBUG_RANGE(plot, curve);
}

void CartesianPlotTest::changeData3() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c1->setValueAt(2, 2.);
	c2->setValueAt(2, 3.);

	QVERIFY(c1->valueAt(2) == 2.);
	QVERIFY(c2->valueAt(2) == 3.);

	CHECK_RANGE(plot, curve, x, 1, 2);
	CHECK_RANGE(plot, curve, y, 1, 3);
}

void CartesianPlotTest::changeData4() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c2->setValueAt(2, 3.);
	c1->setValueAt(2, 3.);

	QVERIFY(c1->valueAt(2) == 3.);
	QVERIFY(c2->valueAt(2) == 3.);

	CHECK_RANGE(plot, curve, x, 1, 3);
	CHECK_RANGE(plot, curve, y, 1, 3);
}

void CartesianPlotTest::changeData5() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c2->setValueAt(2, 2.);
	c1->setValueAt(2, 3.);

	QVERIFY(c1->valueAt(2) == 3.);
	QVERIFY(c2->valueAt(2) == 2.);

	CHECK_RANGE(plot, curve, x, 1, 3);
	CHECK_RANGE(plot, curve, y, 1, 2);
}

void CartesianPlotTest::changeData6() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c2->setValueAt(2, 3.);
	c1->setValueAt(2, 2.);

	QVERIFY(c1->valueAt(2) == 2.);
	QVERIFY(c2->valueAt(2) == 3.);

	CHECK_RANGE(plot, curve, x, 1, 2);
	CHECK_RANGE(plot, curve, y, 1, 3);
}

QTEST_MAIN(CartesianPlotTest)
