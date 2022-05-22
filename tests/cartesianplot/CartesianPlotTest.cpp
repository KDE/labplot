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
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "commonfrontend/worksheet/WorksheetView.h"

#include <QAction>
#include <QUndoStack>

void CartesianPlotTest::initTestCase() {
	//	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

//##############################################################################
//#####################  import of LabPlot projects ############################
//##############################################################################

#define LOAD_PROJECT_DATA_CHANGE                                                                                                                               \
	Project project;                                                                                                                                           \
	project.load(QFINDTESTDATA(QLatin1String("data/TestDataChange.lml")));                                                                                     \
                                                                                                                                                               \
	/* check the project tree for the imported project */                                                                                                      \
	/* Spreadsheet */                                                                                                                                          \
	auto* aspect = project.child<AbstractAspect>(0);                                                                                                           \
	QVERIFY(aspect != nullptr);                                                                                                                                \
	if (aspect)                                                                                                                                                \
		QCOMPARE(aspect->name(), QLatin1String("Spreadsheet"));                                                                                                \
	QVERIFY(aspect->type() == AspectType::Spreadsheet);                                                                                                        \
	auto s = dynamic_cast<Spreadsheet*>(aspect);                                                                                                               \
	if (!s)                                                                                                                                                    \
		return;                                                                                                                                                \
	auto* c1 = static_cast<Column*>(s->child<Column>(0));                                                                                                      \
	QVERIFY(c1 != nullptr);                                                                                                                                    \
	QCOMPARE(c1->name(), QLatin1String("1"));                                                                                                                  \
	QVERIFY(c1->columnMode() == AbstractColumn::ColumnMode::Double);                                                                                           \
	QVERIFY(c1->rowCount() == 100);                                                                                                                            \
	QVERIFY(c1->valueAt(0) == 1.);                                                                                                                             \
	QVERIFY(c1->valueAt(1) == 2.);                                                                                                                             \
	auto* c2 = static_cast<Column*>(s->child<Column>(1));                                                                                                      \
	QVERIFY(c2 != nullptr);                                                                                                                                    \
	QCOMPARE(c2->name(), QLatin1String("2"));                                                                                                                  \
	QVERIFY(c2->columnMode() == AbstractColumn::ColumnMode::Double);                                                                                           \
	QVERIFY(c2->rowCount() == 100);                                                                                                                            \
	QVERIFY(c2->valueAt(0) == 1.);                                                                                                                             \
	QVERIFY(c2->valueAt(1) == 2.);                                                                                                                             \
                                                                                                                                                               \
	/* Worksheet */                                                                                                                                            \
	aspect = project.child<AbstractAspect>(1);                                                                                                                 \
	QVERIFY(aspect != nullptr);                                                                                                                                \
	if (aspect)                                                                                                                                                \
		QCOMPARE(aspect->name(), QLatin1String("Worksheet - Spreadsheet"));                                                                                    \
	QVERIFY(aspect->type() == AspectType::Worksheet);                                                                                                          \
	auto* w = dynamic_cast<Worksheet*>(aspect);                                                                                                                \
	if (!w)                                                                                                                                                    \
		return;                                                                                                                                                \
                                                                                                                                                               \
	auto* plot = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(0));                                                                                \
	QVERIFY(plot != nullptr);                                                                                                                                  \
	if (!plot)                                                                                                                                                 \
		return;                                                                                                                                                \
                                                                                                                                                               \
	/* curve */                                                                                                                                                \
	auto* curve = dynamic_cast<XYCurve*>(plot->child<XYCurve>(0));                                                                                             \
	QVERIFY(curve != nullptr);                                                                                                                                 \
	if (!curve)                                                                                                                                                \
		return;                                                                                                                                                \
	QCOMPARE(curve->name(), "2");                                                                                                                              \
                                                                                                                                                               \
	CHECK_RANGE(plot, curve, x, 1, 2);                                                                                                                         \
	CHECK_RANGE(plot, curve, y, 1, 2);                                                                                                                         \
                                                                                                                                                               \
	auto* xAxis = static_cast<Axis*>(plot->child<Axis>(0));                                                                                                    \
	QVERIFY(xAxis != nullptr);                                                                                                                                 \
	QCOMPARE(xAxis->orientation() == Axis::Orientation::Horizontal, true);                                                                                     \
                                                                                                                                                               \
	auto* yAxis = static_cast<Axis*>(plot->child<Axis>(2));                                                                                                    \
	QVERIFY(yAxis != nullptr);                                                                                                                                 \
	QCOMPARE(yAxis->orientation() == Axis::Orientation::Vertical, true);

#define LOAD_PROJECT_HISTOGRAM_FIT_CURVE                                                                                                                       \
	Project project;                                                                                                                                           \
	project.load(QFINDTESTDATA(QLatin1String("data/histogram-fit-curve.lml")));                                                                                \
                                                                                                                                                               \
	/* TODO: check the project tree for the imported project */                                                                                                \
	/* Spreadsheet */                                                                                                                                          \
	auto* aspect = project.child<AbstractAspect>(0);                                                                                                           \
	QVERIFY(aspect != nullptr);                                                                                                                                \
	if (aspect)                                                                                                                                                \
		QCOMPARE(aspect->name(), QLatin1String("Spreadsheet"));                                                                                                \
	QVERIFY(aspect->type() == AspectType::Spreadsheet);                                                                                                        \
	auto s = dynamic_cast<Spreadsheet*>(aspect);                                                                                                               \
	if (!s)                                                                                                                                                    \
		return;                                                                                                                                                \
	auto* c1 = static_cast<Column*>(s->child<Column>(0));                                                                                                      \
	QVERIFY(c1 != nullptr);                                                                                                                                    \
	QCOMPARE(c1->name(), QLatin1String("Data"));                                                                                                               \
	QVERIFY(c1->columnMode() == AbstractColumn::ColumnMode::Double);                                                                                           \
	QVERIFY(c1->rowCount() == 10000);                                                                                                                          \
                                                                                                                                                               \
	/* Worksheet */                                                                                                                                            \
	aspect = project.child<AbstractAspect>(1);                                                                                                                 \
	QVERIFY(aspect != nullptr);                                                                                                                                \
	if (aspect)                                                                                                                                                \
		QCOMPARE(aspect->name(), QLatin1String("Worksheet - Spreadsheet"));                                                                                    \
	QVERIFY(aspect->type() == AspectType::Worksheet);                                                                                                          \
	auto* w = dynamic_cast<Worksheet*>(aspect);                                                                                                                \
	if (!w)                                                                                                                                                    \
		return;                                                                                                                                                \
                                                                                                                                                               \
	auto* plot = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(0));                                                                                \
	QVERIFY(plot != nullptr);                                                                                                                                  \
	if (!plot)                                                                                                                                                 \
		return;                                                                                                                                                \
                                                                                                                                                               \
	auto* h = dynamic_cast<Histogram*>(plot->child<Histogram>(0));                                                                                             \
	QVERIFY(h != nullptr);                                                                                                                                     \
	if (!h)                                                                                                                                                    \
		return;                                                                                                                                                \
	QCOMPARE(h->name(), "histogram");                                                                                                                          \
                                                                                                                                                               \
	/* curves */                                                                                                                                               \
	auto* curve1 = dynamic_cast<XYCurve*>(plot->child<XYCurve>(0));                                                                                            \
	QVERIFY(curve1 != nullptr);                                                                                                                                \
	if (!curve1)                                                                                                                                               \
		return;                                                                                                                                                \
	QCOMPARE(curve1->name(), "fit");                                                                                                                           \
	auto* curve2 = dynamic_cast<XYCurve*>(plot->child<XYCurve>(1));                                                                                            \
	QVERIFY(curve2 != nullptr);                                                                                                                                \
	if (!curve2)                                                                                                                                               \
		return;                                                                                                                                                \
	QCOMPARE(curve2->name(), "f(x)");                                                                                                                          \
                                                                                                                                                               \
	CHECK_RANGE(plot, curve1, x, -4, 4);                                                                                                                       \
	CHECK_RANGE(plot, curve1, y, 0, 1);

#define VALUES_EQUAL(v1, v2) QCOMPARE(nsl_math_approximately_equal(v1, v2), true)

#define RANGE_CORRECT(range, start_, end_)                                                                                                                     \
	VALUES_EQUAL(range.start(), start_);                                                                                                                       \
	VALUES_EQUAL(range.end(), end_);

#define CHECK_RANGE(plot, aspect, xy, start_, end_)                                                                                                            \
	RANGE_CORRECT(plot->xy##Range(plot->coordinateSystem(aspect->coordinateSystemIndex())->xy##Index()), start_, end_)

#define DEBUG_RANGE(plot, aspect)                                                                                                                              \
	{                                                                                                                                                          \
		int cSystem = aspect->coordinateSystemIndex();                                                                                                         \
		WARN(Q_FUNC_INFO << ", csystem index = " << cSystem)                                                                                                   \
		int xIndex = plot->coordinateSystem(cSystem)->xIndex();                                                                                                \
		int yIndex = plot->coordinateSystem(cSystem)->yIndex();                                                                                                \
                                                                                                                                                               \
		auto xrange = plot->xRange(xIndex);                                                                                                                    \
		auto yrange = plot->yRange(yIndex);                                                                                                                    \
		WARN(Q_FUNC_INFO << ", x index = " << xIndex << ", range = " << xrange.start() << " .. " << xrange.end())                                              \
		WARN(Q_FUNC_INFO << ", y index = " << yIndex << ", range = " << yrange.start() << " .. " << yrange.end())                                              \
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

// check deleting curve

void CartesianPlotTest::deleteCurveAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE

	// delete curve in plot
	plot->removeChild(curve2);

	CHECK_RANGE(plot, curve1, x, -4, 4);
	CHECK_RANGE(plot, curve1, y, 0, 0.45);
}

void CartesianPlotTest::deleteCurveNoAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());
	plot->enableAutoScaleY(cs->yIndex(), false, false, true);

	CHECK_RANGE(plot, curve1, x, -4, 4);
	CHECK_RANGE(plot, curve1, y, 0, 1);

	// delete curve in plot
	plot->removeChild(curve2);

	CHECK_RANGE(plot, curve1, x, -4, 4);
	CHECK_RANGE(plot, curve1, y, 0, 1);

	QCOMPARE(plot->autoScaleY(cs->yIndex()), false);
}

void CartesianPlotTest::invisibleCurveAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE

	curve2->setVisible(false);

	CHECK_RANGE(plot, curve1, x, -4, 4);
	CHECK_RANGE(plot, curve1, y, 0, 0.45);
}

void CartesianPlotTest::invisibleCurveNoAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());
	plot->enableAutoScaleY(cs->yIndex(), false, false, true);

	CHECK_RANGE(plot, curve1, x, -4, 4);
	CHECK_RANGE(plot, curve1, y, 0, 1);

	curve2->setVisible(false);

	CHECK_RANGE(plot, curve1, x, -4, 4);
	CHECK_RANGE(plot, curve1, y, 0, 1);

	QCOMPARE(plot->autoScaleY(cs->yIndex()), false);
}

void CartesianPlotTest::equationCurveEquationChangedAutoScale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());

	QCOMPARE(curve2->type(), AspectType::XYEquationCurve);
	auto eqc = static_cast<XYEquationCurve*>(curve2);

	auto equationData = eqc->equationData();
	equationData.max = "10";
	eqc->setEquationData(equationData);

	CHECK_RANGE(plot, curve2, x, -5, 10); // NiceExtend Changes the xrange to -5 instead of 4
	CHECK_RANGE(plot, curve2, y, 0, 10);

	QCOMPARE(plot->autoScaleY(cs->yIndex()), true);
}

void CartesianPlotTest::equationCurveEquationChangedNoAutoScale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());
	plot->enableAutoScaleY(cs->yIndex(), false, false, true);

	QCOMPARE(curve2->type(), AspectType::XYEquationCurve);
	auto eqc = static_cast<XYEquationCurve*>(curve2);

	auto equationData = eqc->equationData();
	equationData.max = "10";
	eqc->setEquationData(equationData);

	CHECK_RANGE(plot, curve2, x, -5, 10); // NiceExtend Changes the xrange to -5 instead of 4
	CHECK_RANGE(plot, curve2, y, 0, 1);

	QCOMPARE(plot->autoScaleY(cs->yIndex()), false);
}

void CartesianPlotTest::undoInfoElement() {
	auto* project = new Project();
	auto* worksheet = new Worksheet("ws");
	project->addChild(worksheet);

	auto* plot = new CartesianPlot("plot");
	worksheet->addChild(plot);

	auto* curve = new XYCurve("curve");
	plot->addChild(curve);

	auto* info = new InfoElement("info", plot, curve, 0.);
	plot->addChild(info);

	QCOMPARE(plot->childCount<InfoElement>(), 1);

	//undo the last step
	project->undoStack()->undo();
	QCOMPARE(plot->childCount<InfoElement>(), 0);

	//redo
	project->undoStack()->redo();
	QCOMPARE(plot->childCount<InfoElement>(), 1);
}

QTEST_MAIN(CartesianPlotTest)
