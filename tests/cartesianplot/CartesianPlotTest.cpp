/*
	File                 : CartesianPlotTest.cpp
	Project              : LabPlot
	Description          : Tests for cartesian plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianPlotTest.h"
#include "tests/CommonTest.h"

#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/lib/macros.h"
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
	CHECK_RANGE(plot, curve, Dimension::X, 1, 2);                                                                                                              \
	CHECK_RANGE(plot, curve, Dimension::Y, 1, 2);                                                                                                              \
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
	CHECK_RANGE(plot, curve1, Dimension::X, -4, 4);                                                                                                            \
	CHECK_RANGE(plot, curve1, Dimension::Y, 0, 1);

#define VALUES_EQUAL(v1, ref) QVERIFY2(nsl_math_approximately_equal(v1, ref) == true, qPrintable(QString("v1:%1, ref:%2").arg(v1).arg(ref)))

#define RANGE_CORRECT(range, start_, end_)                                                                                                                     \
	VALUES_EQUAL(range.start(), start_);                                                                                                                       \
	VALUES_EQUAL(range.end(), end_);

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

	CHECK_RANGE(plot, curve, Dimension::X, 1, 3);
	CHECK_RANGE(plot, curve, Dimension::Y, 1, 3);
}

void CartesianPlotTest::changeData2() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c1->setValueAt(2, 3.);
	c2->setValueAt(2, 2.);

	QVERIFY(c1->valueAt(2) == 3.);
	QVERIFY(c2->valueAt(2) == 2.);

	CHECK_RANGE(plot, curve, Dimension::X, 1, 3);
	CHECK_RANGE(plot, curve, Dimension::Y, 1, 2);

	DEBUG_RANGE(plot, curve);
}

void CartesianPlotTest::changeData3() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c1->setValueAt(2, 2.);
	c2->setValueAt(2, 3.);

	QVERIFY(c1->valueAt(2) == 2.);
	QVERIFY(c2->valueAt(2) == 3.);

	CHECK_RANGE(plot, curve, Dimension::X, 1, 2);
	CHECK_RANGE(plot, curve, Dimension::Y, 1, 3);
}

void CartesianPlotTest::changeData4() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c2->setValueAt(2, 3.);
	c1->setValueAt(2, 3.);

	QVERIFY(c1->valueAt(2) == 3.);
	QVERIFY(c2->valueAt(2) == 3.);

	CHECK_RANGE(plot, curve, Dimension::X, 1, 3);
	CHECK_RANGE(plot, curve, Dimension::Y, 1, 3);
}

void CartesianPlotTest::changeData5() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c2->setValueAt(2, 2.);
	c1->setValueAt(2, 3.);

	QVERIFY(c1->valueAt(2) == 3.);
	QVERIFY(c2->valueAt(2) == 2.);

	CHECK_RANGE(plot, curve, Dimension::X, 1, 3);
	CHECK_RANGE(plot, curve, Dimension::Y, 1, 2);
}

void CartesianPlotTest::changeData6() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c2->setValueAt(2, 3.);
	c1->setValueAt(2, 2.);

	QVERIFY(c1->valueAt(2) == 2.);
	QVERIFY(c2->valueAt(2) == 3.);

	CHECK_RANGE(plot, curve, Dimension::X, 1, 2);
	CHECK_RANGE(plot, curve, Dimension::Y, 1, 3);
}

// check deleting curve

void CartesianPlotTest::deleteCurveAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE

	// delete curve in plot
	plot->removeChild(curve2);

	CHECK_RANGE(plot, curve1, Dimension::X, -4, 4);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0, 0.45);
}

void CartesianPlotTest::deleteCurveNoAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());
	plot->enableAutoScale(Dimension::Y, cs->index(Dimension::Y), false, false);

	CHECK_RANGE(plot, curve1, Dimension::X, -4, 4);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0, 1);

	// delete curve in plot
	plot->removeChild(curve2);

	CHECK_RANGE(plot, curve1, Dimension::X, -4, 4);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0, 1);

	QCOMPARE(plot->autoScale(Dimension::Y, cs->index(Dimension::Y)), false);
}

void CartesianPlotTest::invisibleCurveAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE

	curve2->setVisible(false);

	CHECK_RANGE(plot, curve1, Dimension::X, -4, 4);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0, 0.45);
}

void CartesianPlotTest::invisibleCurveNoAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());
	plot->enableAutoScale(Dimension::Y, cs->index(Dimension::Y), false, false);

	CHECK_RANGE(plot, curve1, Dimension::X, -4, 4);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0, 1);

	curve2->setVisible(false);

	CHECK_RANGE(plot, curve1, Dimension::X, -4, 4);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0, 1);

	QCOMPARE(plot->autoScale(Dimension::Y, cs->index(Dimension::Y)), false);
}

void CartesianPlotTest::equationCurveEquationChangedAutoScale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());

	QCOMPARE(curve2->type(), AspectType::XYEquationCurve);
	auto eqc = static_cast<XYEquationCurve*>(curve2);

	auto equationData = eqc->equationData();
	equationData.max = "10";
	eqc->setEquationData(equationData);

	CHECK_RANGE(plot, curve2, Dimension::X, -5, 10); // NiceExtend Changes the xrange to -5 instead of 4
	CHECK_RANGE(plot, curve2, Dimension::Y, 0, 10);

	QCOMPARE(plot->autoScale(Dimension::Y, cs->index(Dimension::Y)), true);
}

void CartesianPlotTest::equationCurveEquationChangedNoAutoScale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());
	plot->enableAutoScale(Dimension::Y, cs->index(Dimension::Y), false, false);

	QCOMPARE(curve2->type(), AspectType::XYEquationCurve);
	auto eqc = static_cast<XYEquationCurve*>(curve2);

	auto equationData = eqc->equationData();
	equationData.max = "10";
	eqc->setEquationData(equationData);

	CHECK_RANGE(plot, curve2, Dimension::X, -5, 10); // NiceExtend Changes the xrange to -5 instead of 4
	CHECK_RANGE(plot, curve2, Dimension::Y, 0, 1);

	QCOMPARE(plot->autoScale(Dimension::Y, cs->index(Dimension::Y)), false);
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

	// undo the last step
	project->undoStack()->undo();
	QCOMPARE(plot->childCount<InfoElement>(), 0);

	// redo
	project->undoStack()->redo();
	QCOMPARE(plot->childCount<InfoElement>(), 1);
}

void CartesianPlotTest::axisFormat() {
	// testing #74

	QString savePath;
	{
		Project project;

		Spreadsheet* sheet = new Spreadsheet("Spreadsheet", false);
		project.addChild(sheet);

		sheet->setColumnCount(2);
		sheet->setRowCount(3);

		sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::DateTime);
		sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Integer);

		sheet->column(0)->setDateTimeAt(0, QDateTime::fromString("2022-02-03 12:23:00", Qt::ISODate));
		sheet->column(0)->setDateTimeAt(1, QDateTime::fromString("2022-02-04 12:23:00", Qt::ISODate));
		sheet->column(0)->setDateTimeAt(2, QDateTime::fromString("2022-02-05 12:23:00", Qt::ISODate));

		QCOMPARE(sheet->column(0)->dateTimeAt(0), QDateTime::fromString("2022-02-03 12:23:00", Qt::ISODate));

		sheet->column(1)->setValueAt(0, 0);
		sheet->column(1)->setValueAt(1, 1);
		sheet->column(1)->setValueAt(2, 2);

		auto* worksheet = new Worksheet("Worksheet");
		project.addChild(worksheet);

		auto* plot = new CartesianPlot("plot");
		worksheet->addChild(plot);
		plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created

		auto* curve = new XYCurve("curve");
		plot->addChild(curve);

		curve->setXColumn(sheet->column(0));
		curve->setYColumn(sheet->column(1));

		auto* xAxis = static_cast<Axis*>(plot->child<Axis>(0));
		QVERIFY(xAxis);
		QCOMPARE(xAxis->name(), "x");

		const auto original = xAxis->labelsDateTimeFormat();
		const auto newFormat = "yyyy-MM-dd hh:mm";
		QVERIFY(original != newFormat);
		xAxis->setLabelsDateTimeFormat(newFormat);

		SAVE_PROJECT("TestAxisDateTimeFormat.lml");
	}

	{
		Project project;
		project.load(savePath);

		/* check the project tree for the imported project */
		/* Spreadsheet */
		auto* aspect = project.child<AbstractAspect>(0);
		QVERIFY(aspect);
		QCOMPARE(aspect->name(), QLatin1String("Spreadsheet"));
		QVERIFY(aspect->type() == AspectType::Spreadsheet);

		/* Worksheet */
		aspect = project.child<AbstractAspect>(1);
		QVERIFY(aspect);
		QCOMPARE(aspect->name(), QLatin1String("Worksheet"));
		QVERIFY(aspect->type() == AspectType::Worksheet);
		auto* w = dynamic_cast<Worksheet*>(aspect);
		QVERIFY(w);

		auto* plot = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(0));
		QVERIFY(plot);

		auto* xAxis = static_cast<Axis*>(plot->child<Axis>(0));
		QVERIFY(xAxis);
		QCOMPARE(xAxis->name(), "x");
		QCOMPARE(xAxis->labelsDateTimeFormat(), "yyyy-MM-dd hh:mm");
	}
}

void CartesianPlotTest::shiftLeftAutoScale() {
	Project project;
	auto* ws = new Worksheet("worksheet");
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYEquationCurve("f(x)")};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = "1";
	data.max = "2";
	data.count = 1000;
	data.expression1 = "x";
	curve->setEquationData(data);
	curve->recalculate();

	CHECK_RANGE(p, curve, Dimension::X, 1.0, 2.0);
	CHECK_RANGE(p, curve, Dimension::Y, 1.0, 2.0);

	p->shiftLeftX();

	// Autoscale of the y range was done
	CHECK_RANGE(p, curve, Dimension::X, 1.1, 2.1);
	CHECK_RANGE(p, curve, Dimension::Y, 1.1, 2.0); // changed range
}

void CartesianPlotTest::shiftRightAutoScale() {
	Project project;
	auto* ws = new Worksheet("worksheet");
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYEquationCurve("f(x)")};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = "1";
	data.max = "2";
	data.count = 1000;
	data.expression1 = "x";
	curve->setEquationData(data);
	curve->recalculate();

	CHECK_RANGE(p, curve, Dimension::X, 1.0, 2.0);
	CHECK_RANGE(p, curve, Dimension::Y, 1.0, 2.0);

	p->shiftRightX();

	// Autoscale of the y range was done
	CHECK_RANGE(p, curve, Dimension::X, 0.9, 1.9);
	CHECK_RANGE(p, curve, Dimension::Y, 1.0, 1.9); // changed range
}

void CartesianPlotTest::shiftUpAutoScale() {
	Project project;
	auto* ws = new Worksheet("worksheet");
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYEquationCurve("f(x)")};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = "1";
	data.max = "2";
	data.count = 1000;
	data.expression1 = "x";
	curve->setEquationData(data);
	curve->recalculate();

	CHECK_RANGE(p, curve, Dimension::X, 1.0, 2.0);
	CHECK_RANGE(p, curve, Dimension::Y, 1.0, 2.0);

	p->shiftUpY();

	// Autoscale of the y range was done
	CHECK_RANGE(p, curve, Dimension::X, 1.0, 1.9);
	CHECK_RANGE(p, curve, Dimension::Y, 0.9, 1.9); // changed range
}

void CartesianPlotTest::shiftDownAutoScale() {
	Project project;
	auto* ws = new Worksheet("worksheet");
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYEquationCurve("f(x)")};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = "1";
	data.max = "2";
	data.count = 1000;
	data.expression1 = "x";
	curve->setEquationData(data);
	curve->recalculate();

	CHECK_RANGE(p, curve, Dimension::X, 1.0, 2.0);
	CHECK_RANGE(p, curve, Dimension::Y, 1.0, 2.0);

	p->shiftDownY();

	// Autoscale of the y range was done
	CHECK_RANGE(p, curve, Dimension::X, 1.1, 2.0);
	CHECK_RANGE(p, curve, Dimension::Y, 1.1, 2.1); // changed range
}

void CartesianPlotTest::rangeFormatYDataChanged() {
	Project project;

	Spreadsheet* sheet = new Spreadsheet("Spreadsheet", false);
	project.addChild(sheet);

	sheet->setColumnCount(2);
	sheet->setRowCount(3);

	sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Integer);

	sheet->column(0)->setDateTimeAt(0, QDateTime::fromString("2022-02-03 12:23:00", Qt::ISODate));
	sheet->column(0)->setDateTimeAt(1, QDateTime::fromString("2022-02-04 12:23:00", Qt::ISODate));
	sheet->column(0)->setDateTimeAt(2, QDateTime::fromString("2022-02-05 12:23:00", Qt::ISODate));

	QCOMPARE(sheet->column(0)->dateTimeAt(0), QDateTime::fromString("2022-02-03 12:23:00", Qt::ISODate));

	sheet->column(1)->setValueAt(0, 0);
	sheet->column(1)->setValueAt(1, 1);
	sheet->column(1)->setValueAt(2, 2);

	auto* worksheet = new Worksheet("Worksheet");
	project.addChild(worksheet);

	auto* plot = new CartesianPlot("plot");
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created

	auto* curve = new XYCurve("curve");
	plot->addChild(curve);

	curve->setXColumn(sheet->column(0));
	curve->setYColumn(sheet->column(1));

	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::DateTime);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::Numeric);

	// simulate data change
	plot->dataChanged(curve, Dimension::Y);

	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::DateTime);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::Numeric);
}

void CartesianPlotTest::rangeFormatXDataChanged() {
	Project project;

	Spreadsheet* sheet = new Spreadsheet("Spreadsheet", false);
	project.addChild(sheet);

	sheet->setColumnCount(2);
	sheet->setRowCount(3);

	sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Integer);

	sheet->column(0)->setDateTimeAt(0, QDateTime::fromString("2022-02-03 12:23:00", Qt::ISODate));
	sheet->column(0)->setDateTimeAt(1, QDateTime::fromString("2022-02-04 12:23:00", Qt::ISODate));
	sheet->column(0)->setDateTimeAt(2, QDateTime::fromString("2022-02-05 12:23:00", Qt::ISODate));

	QCOMPARE(sheet->column(0)->dateTimeAt(0), QDateTime::fromString("2022-02-03 12:23:00", Qt::ISODate));

	sheet->column(1)->setValueAt(0, 0);
	sheet->column(1)->setValueAt(1, 1);
	sheet->column(1)->setValueAt(2, 2);

	auto* worksheet = new Worksheet("Worksheet");
	project.addChild(worksheet);

	auto* plot = new CartesianPlot("plot");
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created

	auto* curve = new XYCurve("curve");
	plot->addChild(curve);

	curve->setXColumn(sheet->column(1));
	curve->setYColumn(sheet->column(0));

	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::DateTime);

	// simulate data change
	plot->dataChanged(curve, Dimension::X);

	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::DateTime);
}

QTEST_MAIN(CartesianPlotTest)
