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
#include "backend/worksheet/plots/cartesian/CartesianPlotPrivate.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#include "kdefrontend/dockwidgets/XYFitCurveDock.h"

#include <QAction>
#include <QUndoStack>

void CartesianPlotTest::initTestCase() {
	KLocalizedString::setApplicationDomain("labplot2");
	//	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

// ##############################################################################
// #####################  import of LabPlot projects ############################
// ##############################################################################

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
	QCOMPARE(curve->name(), QStringLiteral("2"));                                                                                                              \
                                                                                                                                                               \
	CHECK_RANGE(plot, curve, Dimension::X, 1., 2.);                                                                                                            \
	CHECK_RANGE(plot, curve, Dimension::Y, 1., 2.);                                                                                                            \
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
	QCOMPARE(h->name(), QStringLiteral("histogram"));                                                                                                          \
                                                                                                                                                               \
	/* curves */                                                                                                                                               \
	auto* curve1 = dynamic_cast<XYCurve*>(plot->child<XYCurve>(0));                                                                                            \
	QVERIFY(curve1 != nullptr);                                                                                                                                \
	if (!curve1)                                                                                                                                               \
		return;                                                                                                                                                \
	QCOMPARE(curve1->name(), QStringLiteral("fit"));                                                                                                           \
	auto* curve2 = dynamic_cast<XYCurve*>(plot->child<XYCurve>(1));                                                                                            \
	QVERIFY(curve2 != nullptr);                                                                                                                                \
	if (!curve2)                                                                                                                                               \
		return;                                                                                                                                                \
	QCOMPARE(curve2->name(), QStringLiteral("f(x)"));                                                                                                          \
                                                                                                                                                               \
	CHECK_RANGE(plot, curve1, Dimension::X, -4., 4.);                                                                                                          \
	CHECK_RANGE(plot, curve1, Dimension::Y, 0., 1.);

#define SET_CARTESIAN_MOUSE_MODE(mode)                                                                                                                         \
	QAction a(nullptr);                                                                                                                                        \
	a.setData(static_cast<int>(mode));                                                                                                                         \
	view->cartesianPlotMouseModeChanged(&a);

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

	CHECK_RANGE(plot, curve, Dimension::X, 1., 3.);
	CHECK_RANGE(plot, curve, Dimension::Y, 1., 3.);
}

void CartesianPlotTest::changeData2() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c1->setValueAt(2, 3.);
	c2->setValueAt(2, 2.);

	QVERIFY(c1->valueAt(2) == 3.);
	QVERIFY(c2->valueAt(2) == 2.);

	CHECK_RANGE(plot, curve, Dimension::X, 1., 3.);
	CHECK_RANGE(plot, curve, Dimension::Y, 1., 2.);

	DEBUG_RANGE(plot, curve);
}

void CartesianPlotTest::changeData3() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c1->setValueAt(2, 2.);
	c2->setValueAt(2, 3.);

	QVERIFY(c1->valueAt(2) == 2.);
	QVERIFY(c2->valueAt(2) == 3.);

	CHECK_RANGE(plot, curve, Dimension::X, 1., 2.);
	CHECK_RANGE(plot, curve, Dimension::Y, 1., 3.);
}

void CartesianPlotTest::changeData4() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c2->setValueAt(2, 3.);
	c1->setValueAt(2, 3.);

	QVERIFY(c1->valueAt(2) == 3.);
	QVERIFY(c2->valueAt(2) == 3.);

	CHECK_RANGE(plot, curve, Dimension::X, 1., 3.);
	CHECK_RANGE(plot, curve, Dimension::Y, 1., 3.);
}

void CartesianPlotTest::changeData5() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c2->setValueAt(2, 2.);
	c1->setValueAt(2, 3.);

	QVERIFY(c1->valueAt(2) == 3.);
	QVERIFY(c2->valueAt(2) == 2.);

	CHECK_RANGE(plot, curve, Dimension::X, 1., 3.);
	CHECK_RANGE(plot, curve, Dimension::Y, 1., 2.);
}

void CartesianPlotTest::changeData6() {
	LOAD_PROJECT_DATA_CHANGE

	// insert data
	c2->setValueAt(2, 3.);
	c1->setValueAt(2, 2.);

	QVERIFY(c1->valueAt(2) == 2.);
	QVERIFY(c2->valueAt(2) == 3.);

	CHECK_RANGE(plot, curve, Dimension::X, 1., 2.);
	CHECK_RANGE(plot, curve, Dimension::Y, 1., 3.);
}

// check deleting curve

void CartesianPlotTest::deleteCurveAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE

	// delete curve in plot
	plot->removeChild(curve2);

	CHECK_RANGE(plot, curve1, Dimension::X, -4., 4.);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0., 0.45);
}

void CartesianPlotTest::deleteCurveNoAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());
	plot->enableAutoScale(Dimension::Y, cs->index(Dimension::Y), false, false);

	CHECK_RANGE(plot, curve1, Dimension::X, -4., 4.);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0., 1.);

	// delete curve in plot
	plot->removeChild(curve2);

	CHECK_RANGE(plot, curve1, Dimension::X, -4., 4.);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0., 1.);

	QCOMPARE(plot->autoScale(Dimension::Y, cs->index(Dimension::Y)), false);
}

void CartesianPlotTest::invisibleCurveAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE

	curve2->setVisible(false);

	CHECK_RANGE(plot, curve1, Dimension::X, -4., 4.);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0., 0.45);
}

void CartesianPlotTest::invisibleCurveNoAutoscale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());
	plot->enableAutoScale(Dimension::Y, cs->index(Dimension::Y), false, false);

	CHECK_RANGE(plot, curve1, Dimension::X, -4., 4.);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0., 1.);

	curve2->setVisible(false);

	CHECK_RANGE(plot, curve1, Dimension::X, -4., 4.);
	CHECK_RANGE(plot, curve1, Dimension::Y, 0., 1.);

	QCOMPARE(plot->autoScale(Dimension::Y, cs->index(Dimension::Y)), false);
}

void CartesianPlotTest::equationCurveEquationChangedAutoScale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());

	QCOMPARE(curve2->type(), AspectType::XYEquationCurve);
	auto eqc = static_cast<XYEquationCurve*>(curve2);

	auto equationData = eqc->equationData();
	equationData.max = QStringLiteral("10");
	eqc->setEquationData(equationData);

	CHECK_RANGE(plot, curve2, Dimension::X, -4., 10.);
	CHECK_RANGE(plot, curve2, Dimension::Y, 0., 10.);

	QCOMPARE(plot->autoScale(Dimension::Y, cs->index(Dimension::Y)), true);
}

void CartesianPlotTest::equationCurveEquationChangedNoAutoScale() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE
	const auto cs = plot->coordinateSystem(curve2->coordinateSystemIndex());
	plot->enableAutoScale(Dimension::Y, cs->index(Dimension::Y), false, false);

	QCOMPARE(curve2->type(), AspectType::XYEquationCurve);
	auto eqc = static_cast<XYEquationCurve*>(curve2);

	auto equationData = eqc->equationData();
	equationData.max = QStringLiteral("10");
	eqc->setEquationData(equationData);

	CHECK_RANGE(plot, curve2, Dimension::X, -4., 10.);
	CHECK_RANGE(plot, curve2, Dimension::Y, 0., 1.);

	QCOMPARE(plot->autoScale(Dimension::Y, cs->index(Dimension::Y)), false);
}

void CartesianPlotTest::undoInfoElement() {
	auto* project = new Project();
	auto* worksheet = new Worksheet(QStringLiteral("ws"));
	project->addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);

	auto* curve = new XYCurve(QStringLiteral("curve"));
	plot->addChild(curve);

	auto* info = new InfoElement(QStringLiteral("info"), plot, curve, 0.);
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

		Spreadsheet* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
		project.addChild(sheet);

		sheet->setColumnCount(2);
		sheet->setRowCount(3);

		sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::DateTime);
		sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Integer);

		sheet->column(0)->setDateTimeAt(0, QDateTime::fromString(QStringLiteral("2022-02-03 12:23:00"), Qt::ISODate));
		sheet->column(0)->setDateTimeAt(1, QDateTime::fromString(QStringLiteral("2022-02-04 12:23:00"), Qt::ISODate));
		sheet->column(0)->setDateTimeAt(2, QDateTime::fromString(QStringLiteral("2022-02-05 12:23:00"), Qt::ISODate));

		QCOMPARE(sheet->column(0)->dateTimeAt(0), QDateTime::fromString(QStringLiteral("2022-02-03 12:23:00"), Qt::ISODate));

		sheet->column(1)->setValueAt(0, 0);
		sheet->column(1)->setValueAt(1, 1);
		sheet->column(1)->setValueAt(2, 2);

		auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
		project.addChild(worksheet);

		auto* plot = new CartesianPlot(QStringLiteral("plot"));
		worksheet->addChild(plot);
		plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created

		auto* curve = new XYCurve(QStringLiteral("curve"));
		plot->addChild(curve);

		curve->setXColumn(sheet->column(0));
		curve->setYColumn(sheet->column(1));

		auto* xAxis = static_cast<Axis*>(plot->child<Axis>(0));
		QVERIFY(xAxis);
		QCOMPARE(xAxis->name(), QStringLiteral("x"));

		const auto original = xAxis->labelsDateTimeFormat();
		const auto newFormat = QStringLiteral("yyyy-MM-dd hh:mm");
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
		QCOMPARE(xAxis->name(), QStringLiteral("x"));
		QCOMPARE(xAxis->labelsDateTimeFormat(), QStringLiteral("yyyy-MM-dd hh:mm"));
	}
}

void CartesianPlotTest::shiftLeftAutoScale() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYEquationCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("1");
	data.max = QStringLiteral("2");
	data.count = 1000;
	data.expression1 = QStringLiteral("x");
	curve->setEquationData(data);
	curve->recalculate();

	CHECK_RANGE(p, curve, Dimension::X, 1., 2.);
	CHECK_RANGE(p, curve, Dimension::Y, 1., 2.);

	p->shiftLeftX();

	// Autoscale of the y range was done
	CHECK_RANGE(p, curve, Dimension::X, 1.1, 2.1);
	CHECK_RANGE(p, curve, Dimension::Y, 1.1, 2.); // changed range
}

void CartesianPlotTest::shiftRightAutoScale() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYEquationCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("1");
	data.max = QStringLiteral("2");
	data.count = 1000;
	data.expression1 = QStringLiteral("x");
	curve->setEquationData(data);
	curve->recalculate();

	CHECK_RANGE(p, curve, Dimension::X, 1., 2.);
	CHECK_RANGE(p, curve, Dimension::Y, 1., 2.);

	p->shiftRightX();

	// Autoscale of the y range was done
	CHECK_RANGE(p, curve, Dimension::X, 0.9, 1.9);
	CHECK_RANGE(p, curve, Dimension::Y, 1., 1.9); // changed range
}

void CartesianPlotTest::shiftUpAutoScale() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYEquationCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("1");
	data.max = QStringLiteral("2");
	data.count = 1000;
	data.expression1 = QStringLiteral("x");
	curve->setEquationData(data);
	curve->recalculate();

	CHECK_RANGE(p, curve, Dimension::X, 1., 2.);
	CHECK_RANGE(p, curve, Dimension::Y, 1., 2.);

	p->shiftUpY();

	// Autoscale of the y range was done
	CHECK_RANGE(p, curve, Dimension::X, 1., 1.9);
	CHECK_RANGE(p, curve, Dimension::Y, 0.9, 1.9); // changed range
}

void CartesianPlotTest::shiftDownAutoScale() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYEquationCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("1");
	data.max = QStringLiteral("2");
	data.count = 1000;
	data.expression1 = QStringLiteral("x");
	curve->setEquationData(data);
	curve->recalculate();

	CHECK_RANGE(p, curve, Dimension::X, 1., 2.);
	CHECK_RANGE(p, curve, Dimension::Y, 1., 2.);

	p->shiftDownY();

	// Autoscale of the y range was done
	CHECK_RANGE(p, curve, Dimension::X, 1.1, 2.);
	CHECK_RANGE(p, curve, Dimension::Y, 1.1, 2.1); // changed range
}

void CartesianPlotTest::rangeFormatYDataChanged() {
	Project project;

	Spreadsheet* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	project.addChild(sheet);

	sheet->setColumnCount(2);
	sheet->setRowCount(3);

	sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Integer);

	sheet->column(0)->setDateTimeAt(0, QDateTime::fromString(QStringLiteral("2022-02-03 12:23:00"), Qt::ISODate));
	sheet->column(0)->setDateTimeAt(1, QDateTime::fromString(QStringLiteral("2022-02-04 12:23:00"), Qt::ISODate));
	sheet->column(0)->setDateTimeAt(2, QDateTime::fromString(QStringLiteral("2022-02-05 12:23:00"), Qt::ISODate));

	QCOMPARE(sheet->column(0)->dateTimeAt(0), QDateTime::fromString(QStringLiteral("2022-02-03 12:23:00"), Qt::ISODate));

	sheet->column(1)->setValueAt(0, 0);
	sheet->column(1)->setValueAt(1, 1);
	sheet->column(1)->setValueAt(2, 2);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created

	auto* curve = new XYCurve(QStringLiteral("curve"));
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

	Spreadsheet* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	project.addChild(sheet);

	sheet->setColumnCount(2);
	sheet->setRowCount(3);

	sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Integer);

	sheet->column(0)->setDateTimeAt(0, QDateTime::fromString(QStringLiteral("2022-02-03 12:23:00"), Qt::ISODate));
	sheet->column(0)->setDateTimeAt(1, QDateTime::fromString(QStringLiteral("2022-02-04 12:23:00"), Qt::ISODate));
	sheet->column(0)->setDateTimeAt(2, QDateTime::fromString(QStringLiteral("2022-02-05 12:23:00"), Qt::ISODate));

	QCOMPARE(sheet->column(0)->dateTimeAt(0), QDateTime::fromString(QStringLiteral("2022-02-03 12:23:00"), Qt::ISODate));

	sheet->column(1)->setValueAt(0, 0);
	sheet->column(1)->setValueAt(1, 1);
	sheet->column(1)->setValueAt(2, 2);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created

	auto* curve = new XYCurve(QStringLiteral("curve"));
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

void CartesianPlotTest::rangeFormatNonDefaultRange() {
	Project project;

	Spreadsheet* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	project.addChild(sheet);

	sheet->setColumnCount(4);
	sheet->setRowCount(3);

	sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::Integer);
	sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Integer);
	sheet->column(2)->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	sheet->column(3)->setColumnMode(AbstractColumn::ColumnMode::Integer);

	sheet->column(0)->setValueAt(0, 0);
	sheet->column(0)->setValueAt(1, 1);
	sheet->column(0)->setValueAt(2, 2);

	sheet->column(1)->setValueAt(0, 5);
	sheet->column(1)->setValueAt(1, 6);
	sheet->column(1)->setValueAt(2, 7);

	sheet->column(2)->setDateTimeAt(0, QDateTime::fromString(QStringLiteral("2022-02-03 12:23:00"), Qt::ISODate));
	sheet->column(2)->setDateTimeAt(1, QDateTime::fromString(QStringLiteral("2022-02-04 12:23:00"), Qt::ISODate));
	sheet->column(2)->setDateTimeAt(2, QDateTime::fromString(QStringLiteral("2022-02-05 12:23:00"), Qt::ISODate));

	QCOMPARE(sheet->column(2)->dateTimeAt(0), QDateTime::fromString(QStringLiteral("2022-02-03 12:23:00"), Qt::ISODate));

	sheet->column(3)->setValueAt(0, 8);
	sheet->column(3)->setValueAt(1, 10);
	sheet->column(3)->setValueAt(2, 9);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created

	// Create new cSystem
	Range<double> xRange;
	xRange.setFormat(RangeT::Format::Numeric);
	Range<double> yRange;
	yRange.setFormat(RangeT::Format::Numeric);
	plot->addXRange(xRange);
	plot->addYRange(yRange);
	CartesianCoordinateSystem* cSystem = new CartesianCoordinateSystem(plot);
	cSystem->setIndex(Dimension::X, 1);
	cSystem->setIndex(Dimension::Y, 1);
	plot->addCoordinateSystem(cSystem);

	auto* curve2 = new XYCurve(QStringLiteral("curve2"));
	plot->addChild(curve2);

	curve2->setCoordinateSystemIndex(1);

	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::X, 1), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 1), RangeT::Format::Numeric);

	curve2->setXColumn(sheet->column(2));

	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::X, 1), RangeT::Format::DateTime);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 1), RangeT::Format::Numeric);

	curve2->setYColumn(sheet->column(3));

	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::X, 1), RangeT::Format::DateTime);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 1), RangeT::Format::Numeric);

	plot->dataChanged(curve2, Dimension::X);

	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::X, 1), RangeT::Format::DateTime);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 1), RangeT::Format::Numeric);
}

void CartesianPlotTest::invalidStartValueLogScaling() {
	Project project;

	Spreadsheet* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	project.addChild(sheet);

	sheet->setColumnCount(3);
	sheet->setRowCount(3);

	sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::Double);
	sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Double);
	sheet->column(2)->setColumnMode(AbstractColumn::ColumnMode::Double);

	sheet->column(0)->setValueAt(0, 0.);
	sheet->column(0)->setValueAt(1, 1.);
	sheet->column(0)->setValueAt(2, 2.);

	sheet->column(1)->setValueAt(0, 0.);
	sheet->column(1)->setValueAt(1, 1.);
	sheet->column(1)->setValueAt(2, 2.);

	sheet->column(2)->setValueAt(0, 0.00001);
	sheet->column(2)->setValueAt(1, 0.1);
	sheet->column(2)->setValueAt(2, 1.);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created

	// Create new cSystem
	Range<double> xRange;
	xRange.setFormat(RangeT::Format::Numeric);
	Range<double> yRange;
	yRange.setFormat(RangeT::Format::Numeric);
	yRange.setScale(RangeT::Scale::Log10);
	plot->addXRange(xRange);
	plot->addYRange(yRange);
	CartesianCoordinateSystem* cSystem = new CartesianCoordinateSystem(plot);
	cSystem->setIndex(Dimension::X, 1);
	cSystem->setIndex(Dimension::Y, 1);
	plot->addCoordinateSystem(cSystem);
	plot->setNiceExtend(false);

	auto* curve1 = new XYCurve(QStringLiteral("curve1"));
	plot->addChild(curve1);

	curve1->setXColumn(sheet->column(0));
	curve1->setYColumn(sheet->column(1));
	curve1->setCoordinateSystemIndex(1);

	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::X, 1), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 1), RangeT::Format::Numeric);

	auto* curve2 = new XYCurve(QStringLiteral("curve2"));
	plot->addChild(curve2);

	curve2->setXColumn(sheet->column(0));
	curve2->setYColumn(sheet->column(2));
	curve2->setCoordinateSystemIndex(1);

	plot->zoomOut(1, 1);

	plot->navigate(1, CartesianPlot::NavigationOperation::ScaleAuto);

	// Doesn't matter which curve is used here, because both are using the same cSystem
	CHECK_RANGE(plot, curve1, Dimension::X, 0., 2.);
	// 0 is not valid for log10 scaling, so use the smallest valid values of the curves
	CHECK_RANGE(plot, curve1, Dimension::Y, 0.00001, 2.);
}

/*!
 * \brief CartesianPlotTest::invalidcSystem
 * Plot with 2 CoordinateSystems (with common x range), but the second has invalid start end (0, 0).
 * This scenario shall not destroy the x range when zooming in
 *
 */
void CartesianPlotTest::invalidcSystem() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);
	auto* view = dynamic_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view != nullptr);
	view->initActions(); // needed by SET_CARTESIAN_MOUSE_MODE()

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection) // must be set after the plot was added

	// Create new cSystem
	Range<double> yRange;
	yRange.setFormat(RangeT::Format::Numeric);
	plot->addYRange(yRange);
	plot->addCoordinateSystem();
	QCOMPARE(plot->coordinateSystemCount(), 2);

	auto* cSystem{plot->coordinateSystem(1)};
	cSystem->setIndex(Dimension::Y, 1);
	plot->setRangeDirty(Dimension::Y, 1, true);
	plot->retransformScale(Dimension::Y, 1);

	{
		CHECK_RANGE_CSYSTEMINDEX(plot, 0, Dimension::X, 0., 1.);
		CHECK_RANGE_CSYSTEMINDEX(plot, 0, Dimension::Y, 0., 1.);
		CHECK_RANGE_CSYSTEMINDEX(plot, 1, Dimension::X, 0., 1.);
		CHECK_RANGE_CSYSTEMINDEX(plot, 1, Dimension::Y, 0., 1.);
		const Range<double> plotSceneRangeX = {plot->dataRect().x(), plot->dataRect().x() + plot->dataRect().width()};
		const Range<double> plotSceneRangeY = {plot->dataRect().y() + plot->dataRect().height(), plot->dataRect().y()};

		double bx = plotSceneRangeX.size() / (1 - 0);
		double ax = plotSceneRangeX.start() - bx * 0;

		double by = plotSceneRangeY.size() / (1 - 0);
		double ay = plotSceneRangeY.start() - by * 0;

		CHECK_SCALE_PLOT(plot, 0, Dimension::X, ax, bx, 0);
		CHECK_SCALE_PLOT(plot, 0, Dimension::Y, ay, by, 0);
		CHECK_SCALE_PLOT(plot, 1, Dimension::X, ax, bx, 0);
		CHECK_SCALE_PLOT(plot, 1, Dimension::Y, ay, by, 0);
	}

	// Set range of the unused y range
	Range<double> range;
	range.setStart(0); // Both are set to the same value
	range.setEnd(0); // Both are set to the same value
	range.setFormat(RangeT::Format::Numeric);
	range.setAutoScale(false);
	range.setScale(RangeT::Scale::Linear);

	{
		// plot->setRange(Dimension::Y, 1, range); // does not work
		// Implementation of setRange() must be used, because setRange() uses check to check if
		// the range is valid, which it isn't in this test. To test neverthless and not removing a test
		// use directly the implementation
		int index = 1;
		auto dimension = Dimension::Y;
		auto otherValue = range;
		auto plotPrivate = plot->d_func();
		plotPrivate->setRangeDirty(dimension, index, true);
		auto tmp = plotPrivate->rangeConst(dimension, index);
		plotPrivate->setRange(dimension, index, otherValue);
		otherValue = tmp;
		plotPrivate->retransformScale(dimension, index, true);
		Dimension dim_other = Dimension::Y;
		if (dimension == Dimension::Y)
			dim_other = Dimension::X;

		QVector<int> scaledIndices;
		for (int i = 0; i < plotPrivate->q->coordinateSystemCount(); i++) {
			auto cs = plotPrivate->q->coordinateSystem(i);
			auto index_other = cs->index(dim_other);
			if (cs->index(dimension) == index && scaledIndices.indexOf(index_other) == -1) {
				scaledIndices << index_other;
				if (plotPrivate->q->autoScale(dim_other, index_other) && plotPrivate->q->scaleAuto(dim_other, index_other, false))
					plotPrivate->retransformScale(dim_other, index_other);
			}
		}
		plotPrivate->q->WorksheetElementContainer::retransform();
		Q_EMIT plotPrivate->q->rangeChanged(dimension, index, plotPrivate->rangeConst(dimension, index));
	}

	// Recalculate scales is triggered
	{
		QCOMPARE(plot->coordinateSystemCount(), 2);

		CHECK_RANGE_CSYSTEMINDEX(plot, 0, Dimension::X, 0., 1.);
		CHECK_RANGE_CSYSTEMINDEX(plot, 0, Dimension::Y, 0., 1.);
		CHECK_RANGE_CSYSTEMINDEX(plot, 1, Dimension::X, 0., 1.);
		CHECK_RANGE_CSYSTEMINDEX(plot, 1, Dimension::Y, 0., 0.);
		const Range<double> plotSceneRangeX = {plot->dataRect().x(), plot->dataRect().x() + plot->dataRect().width()};
		const Range<double> plotSceneRangeY = {plot->dataRect().y() + plot->dataRect().height(), plot->dataRect().y()};

		double bx = plotSceneRangeX.size() / (1 - 0);
		double ax = plotSceneRangeX.start() - bx * 0;

		double by = plotSceneRangeY.size() / (1 - 0);
		double ay = plotSceneRangeY.start() - by * 0;

		CHECK_SCALE_PLOT(plot, 0, Dimension::X, ax, bx, 0);
		CHECK_SCALE_PLOT(plot, 0, Dimension::Y, ay, by, 0);
		// Don't care what the second cSystem has, because it is invalid
		// CHECK_SCALE_PLOT(plot, 1, Dimension::X, ax, bx, 0);
		// CHECK_SCALE_PLOT(plot, 1, Dimension::Y, ay, by, 0);
	}

	QCOMPARE(plot->mouseMode(), CartesianPlot::MouseMode::ZoomXSelection);
	plot->mousePressZoomSelectionMode(QPointF(0.2, -150), -1);
	plot->mouseMoveZoomSelectionMode(QPointF(0.6, 100), -1);
	plot->mouseReleaseZoomSelectionMode(-1);

	{
		QCOMPARE(plot->coordinateSystemCount(), 2);

		CHECK_RANGE_CSYSTEMINDEX(plot, 0, Dimension::X, 0.2, 0.6);
		CHECK_RANGE_CSYSTEMINDEX(plot, 0, Dimension::Y, 0., 1.);
		CHECK_RANGE_CSYSTEMINDEX(plot, 1, Dimension::X, 0.2, 0.6);
		CHECK_RANGE_CSYSTEMINDEX(plot, 1, Dimension::Y, 0., 0.);
		const Range<double> plotSceneRangeX = {plot->dataRect().x(), plot->dataRect().x() + plot->dataRect().width()};
		const Range<double> plotSceneRangeY = {plot->dataRect().y() + plot->dataRect().height(), plot->dataRect().y()};

		double bx = plotSceneRangeX.size() / (0.6 - 0.2);
		double ax = plotSceneRangeX.start() - bx * 0.2;

		double by = plotSceneRangeY.size() / (1 - 0);
		double ay = plotSceneRangeY.start() - by * 0;

		CHECK_SCALE_PLOT(plot, 0, Dimension::X, ax, bx, 0);
		CHECK_SCALE_PLOT(plot, 0, Dimension::Y, ay, by, 0);
		// Don't care what the second cSystem has, because it is invalid
		// CHECK_SCALE_PLOT(plot, 1, Dimension::X, ax, bx, 0);
		// CHECK_SCALE_PLOT(plot, 1, Dimension::Y, ay, by, 0);
	}
}

void CartesianPlotTest::autoScaleFitCurveCalculation() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);
	auto* view = dynamic_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view != nullptr);
	view->initActions(); // needed by SET_CARTESIAN_MOUSE_MODE()

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(false);

	auto* equationCurve{new XYEquationCurve(QStringLiteral("f(x)"))};
	equationCurve->setCoordinateSystemIndex(plot->defaultCoordinateSystemIndex());
	plot->addChild(equationCurve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("1");
	data.count = 10;
	data.expression1 = QStringLiteral("x");
	equationCurve->setEquationData(data);
	equationCurve->recalculate();

	CHECK_RANGE(plot, equationCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(plot, equationCurve, Dimension::Y, 0., 1.);

	auto* fitCurve = new XYFitCurve(QStringLiteral("Fit"));
	fitCurve->setCoordinateSystemIndex(plot->defaultCoordinateSystemIndex());
	plot->addChild(fitCurve);

	XYFitCurve::FitData f;
	f.autoRange = false;
	f.fitRange = Range<double>(0, 1);
	f.autoEvalRange = false;
	f.evalRange = Range<double>(0, 3); // larger than fit range
	f.modelCategory = nsl_fit_model_basic;
	f.modelType = nsl_fit_model_polynomial;
	f.degree = 1; // linear
	f.model = QStringLiteral("c0 + c1*x");
	fitCurve->initFitData(f); // Important, otherwise paramNames gets not filled
	fitCurve->setFitData(f);
	fitCurve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
	fitCurve->setDataSourceCurve(equationCurve);
	fitCurve->recalculate();

	CHECK_RANGE(plot, equationCurve, Dimension::X, 0., 3.);
	CHECK_RANGE(plot, equationCurve, Dimension::Y, 0., 3.);
}

void CartesianPlotTest::wheelEventCenterAxes() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);
	auto* view = dynamic_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view != nullptr);
	view->initMenus(); // needed by SET_CARTESIAN_MOUSE_MODE()

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(false);

	auto* equationCurve{new XYEquationCurve(QStringLiteral("f(x)"))};
	equationCurve->setCoordinateSystemIndex(plot->defaultCoordinateSystemIndex());
	plot->addChild(equationCurve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.count = 10;
	data.expression1 = QStringLiteral("x");
	equationCurve->setEquationData(data);
	equationCurve->recalculate();

	CHECK_RANGE(plot, equationCurve, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, equationCurve, Dimension::Y, 0., 10.);

	plot->m_zoomFactor = 2;
	plot->setNiceExtend(false);
	const auto& rect = plot->dataRect();

	int signalEmittedCounter = 0;
	connect(plot,
			&CartesianPlot::wheelEventSignal,
			[&signalEmittedCounter](const QPointF& sceneRelPos, int delta, int xIndex, int yIndex, bool considerDimension, Dimension dim) {
				signalEmittedCounter++;
				if (signalEmittedCounter == 1) {
					QCOMPARE(sceneRelPos.x(), 0.5);
					QCOMPARE(sceneRelPos.y(), 0.5);
					QVERIFY(delta > 0);
					QCOMPARE(xIndex, 0);
					QCOMPARE(yIndex, 0);
					QCOMPARE(considerDimension, true);
					QCOMPARE(dim, Dimension::X);
				} else {
					QCOMPARE(sceneRelPos.x(), 0.5);
					QCOMPARE(sceneRelPos.y(), 0.5);
					QVERIFY(delta < 0);
					QCOMPARE(xIndex, 0);
					QCOMPARE(yIndex, 0);
					QCOMPARE(considerDimension, true);
					QCOMPARE(dim, Dimension::Y);
				}
			});

	const auto& axes = plot->children<Axis>();
	QCOMPARE(axes.length(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* xAxis = axes.at(0);
	auto* yAxis = axes.at(1);

	plot->enableAutoScale(Dimension::X, 0, false);
	plot->enableAutoScale(Dimension::Y, 0, false);

	xAxis->setSelected(true);
	QGraphicsSceneWheelEvent event;
	event.setPos(QPointF(rect.center().x(), rect.center().y()));
	event.setDelta(10); // value not important, only sign
	plot->d_func()->wheelEvent(&event);
	CHECK_RANGE(plot, equationCurve, Dimension::X, 2.5, 7.5);
	CHECK_RANGE(plot, equationCurve, Dimension::Y, 0., 10.);
	QCOMPARE(signalEmittedCounter, 1);

	xAxis->setSelected(false);
	yAxis->setSelected(true);
	event.setPos(QPointF(rect.center().x(), rect.center().y()));
	event.setDelta(-10); // value not important, only sign
	plot->d_func()->wheelEvent(&event);
	CHECK_RANGE(plot, equationCurve, Dimension::X, 2.5, 7.5);
	CHECK_RANGE(plot, equationCurve, Dimension::Y, -5., 15.0);
	QCOMPARE(signalEmittedCounter, 2);
}

void CartesianPlotTest::wheelEventNotCenter() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);
	auto* view = dynamic_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view != nullptr);
	view->initMenus(); // needed by SET_CARTESIAN_MOUSE_MODE()

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(false);

	auto* equationCurve{new XYEquationCurve(QStringLiteral("f(x)"))};
	equationCurve->setCoordinateSystemIndex(plot->defaultCoordinateSystemIndex());
	plot->addChild(equationCurve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.count = 10;
	data.expression1 = QStringLiteral("x");
	equationCurve->setEquationData(data);
	equationCurve->recalculate();

	CHECK_RANGE(plot, equationCurve, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, equationCurve, Dimension::Y, 0., 10.);

	plot->m_zoomFactor = 2;
	plot->setNiceExtend(false);
	const auto& rect = plot->dataRect();

	const auto& axes = plot->children<Axis>();
	QCOMPARE(axes.length(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* xAxis = axes.at(0);
	//	const auto* yAxis = axes.at(1);

	int signalEmittedCounter = 0;
	connect(plot,
			&CartesianPlot::wheelEventSignal,
			[&signalEmittedCounter](const QPointF& sceneRelPos, int delta, int xIndex, int yIndex, bool considerDimension, Dimension dim) {
				signalEmittedCounter++;
				QCOMPARE(sceneRelPos.x(), 0.75);
				QCOMPARE(sceneRelPos.y(), 0.2);
				QVERIFY(delta > 0);
				QCOMPARE(xIndex, 0);
				QCOMPARE(yIndex, 0);
				QCOMPARE(considerDimension, true);
				QCOMPARE(dim, Dimension::X);
			});

	xAxis->setSelected(true);
	QGraphicsSceneWheelEvent event;
	event.setPos(QPointF(rect.left() + rect.width() * 3 / 4, rect.top() + rect.height() * 0.8));
	event.setDelta(10); // value not important, only sign
	plot->d_func()->wheelEvent(&event);
	CHECK_RANGE(plot, equationCurve, Dimension::X, 3.75, 8.75);
	CHECK_RANGE(plot, equationCurve, Dimension::Y, 10. / 3., 8.8888888888888888);
	QCOMPARE(signalEmittedCounter, 1);
}

void CartesianPlotTest::wheelEventOutsideTopLeft() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);
	auto* view = dynamic_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view != nullptr);
	view->initMenus(); // needed by SET_CARTESIAN_MOUSE_MODE()

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(false);
	plot->setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
	plot->setVerticalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
	plot->setRightPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
	plot->setBottomPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

	auto* equationCurve{new XYEquationCurve(QStringLiteral("f(x)"))};
	equationCurve->setCoordinateSystemIndex(plot->defaultCoordinateSystemIndex());
	plot->addChild(equationCurve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.count = 10;
	data.expression1 = QStringLiteral("x");
	equationCurve->setEquationData(data);
	equationCurve->recalculate();

	CHECK_RANGE(plot, equationCurve, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, equationCurve, Dimension::Y, 0., 10.);

	plot->m_zoomFactor = 2;
	plot->setNiceExtend(false);
	const auto& rect = plot->dataRect();

	int signalEmittedCounter = 0;
	connect(plot,
			&CartesianPlot::wheelEventSignal,
			[&signalEmittedCounter](const QPointF& sceneRelPos, int delta, int xIndex, int yIndex, bool considerDimension, Dimension dim) {
				signalEmittedCounter++;
				QCOMPARE(sceneRelPos.x(), -0.2);
				QCOMPARE(sceneRelPos.y(), 1.3);
				QVERIFY(delta > 0);
				Q_UNUSED(xIndex);
				Q_UNUSED(yIndex);
				QCOMPARE(considerDimension, false);
				Q_UNUSED(dim);
			});

	QGraphicsSceneWheelEvent event;
	event.setPos(QPointF(rect.left() - 140, rect.top() - 210));
	event.setDelta(10); // value not important, only sign
	plot->d_func()->wheelEvent(&event);
	CHECK_RANGE(plot, equationCurve, Dimension::X, -1., 4.);
	CHECK_RANGE(plot, equationCurve, Dimension::Y, 6.5, 11.5);
	QCOMPARE(signalEmittedCounter, 1);
}

void CartesianPlotTest::wheelEventOutsideBottomRight() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);
	auto* view = dynamic_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view != nullptr);
	view->initMenus(); // needed by SET_CARTESIAN_MOUSE_MODE()

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(false);
	plot->setHorizontalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
	plot->setVerticalPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
	plot->setRightPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));
	plot->setBottomPadding(Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter));

	auto* equationCurve{new XYEquationCurve(QStringLiteral("f(x)"))};
	equationCurve->setCoordinateSystemIndex(plot->defaultCoordinateSystemIndex());
	plot->addChild(equationCurve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.count = 10;
	data.expression1 = QStringLiteral("x");
	equationCurve->setEquationData(data);
	equationCurve->recalculate();

	CHECK_RANGE(plot, equationCurve, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, equationCurve, Dimension::Y, 0., 10.);

	plot->m_zoomFactor = 2;
	plot->setNiceExtend(false);
	const auto& rect = plot->dataRect();

	int signalEmittedCounter = 0;
	connect(plot,
			&CartesianPlot::wheelEventSignal,
			[&signalEmittedCounter](const QPointF& sceneRelPos, int delta, int xIndex, int yIndex, bool considerDimension, Dimension dim) {
				signalEmittedCounter++;
				QCOMPARE(sceneRelPos.x(), 1.4);
				QCOMPARE(sceneRelPos.y(), -0.7);
				QVERIFY(delta > 0);
				Q_UNUSED(xIndex);
				Q_UNUSED(yIndex);
				QCOMPARE(considerDimension, false);
				Q_UNUSED(dim);
			});

	QGraphicsSceneWheelEvent event;
	event.setPos(QPointF(rect.right() + 280, rect.bottom() + 490));
	event.setDelta(10); // value not important, only sign
	plot->d_func()->wheelEvent(&event);
	CHECK_RANGE(plot, equationCurve, Dimension::X, 7., 12.);
	CHECK_RANGE(plot, equationCurve, Dimension::Y, -3.5, 1.5);
	QCOMPARE(signalEmittedCounter, 1);
}

void CartesianPlotTest::spreadsheetRemoveRows() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(false);

	auto* sheet = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheet);
	sheet->setColumnCount(2);

	const auto& columns = sheet->children<Column>();
	QCOMPARE(columns.count(), 2);
	Column* xColumn = columns.at(0);
	Column* yColumn = columns.at(1);

	xColumn->replaceValues(-1,
						   {
							   1.,
							   2.,
							   3.,
							   4.,
							   5.,
						   });
	yColumn->replaceValues(-1,
						   {
							   0.,
							   4.,
							   6.,
							   8.,
							   10.,
						   });

	auto* curve = new XYCurve(QStringLiteral("curve"));
	plot->addChild(curve);
	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);

	CHECK_RANGE(plot, curve, Dimension::X, 1., 5.);
	CHECK_RANGE(plot, curve, Dimension::Y, 0., 10.);

	sheet->removeRows(4, 1);

	CHECK_RANGE(plot, curve, Dimension::X, 1., 4.);
	CHECK_RANGE(plot, curve, Dimension::Y, 0., 8.);

	project.undoStack()->undo();

	CHECK_RANGE(plot, curve, Dimension::X, 1., 5.);
	CHECK_RANGE(plot, curve, Dimension::Y, 0., 10.);
}

void CartesianPlotTest::spreadsheetInsertRows() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(false);

	auto* sheet = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheet);
	sheet->setColumnCount(2);

	const auto& columns = sheet->children<Column>();
	QCOMPARE(columns.count(), 2);
	Column* xColumn = columns.at(0);
	Column* yColumn = columns.at(1);

	xColumn->replaceValues(-1,
						   {
							   1.,
							   2.,
							   3.,
							   4.,
							   5.,
						   });
	yColumn->replaceValues(-1,
						   {
							   0.,
							   4.,
							   6.,
							   8.,
							   10.,
						   });

	auto* curve = new XYCurve(QStringLiteral("curve"));
	plot->addChild(curve);
	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);

	CHECK_RANGE(plot, curve, Dimension::X, 1., 5.);
	CHECK_RANGE(plot, curve, Dimension::Y, 0., 10.);

	sheet->insertRows(4, 1);
	QCOMPARE(xColumn->rowCount(), 6);
	QCOMPARE(yColumn->rowCount(), 6);

	xColumn->replaceValues(5, {13});
	yColumn->replaceValues(5, {25});

	CHECK_RANGE(plot, curve, Dimension::X, 1., 13.);
	CHECK_RANGE(plot, curve, Dimension::Y, 0., 25.);

	project.undoStack()->undo(); // xColumn replace values
	project.undoStack()->undo(); // yColumn replace values
	project.undoStack()->undo(); // spreadsheet insertRows

	CHECK_RANGE(plot, curve, Dimension::X, 1., 5.);
	CHECK_RANGE(plot, curve, Dimension::Y, 0., 10.);
}

void CartesianPlotTest::columnRemove() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(false);

	auto* sheet = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheet);
	sheet->setColumnCount(2);

	const auto& columns = sheet->children<Column>();
	QCOMPARE(columns.count(), 2);
	const auto* xColumn = columns.at(0);
	const auto* yColumn = columns.at(1);
	const auto& xColumnPath = xColumn->path();
	const auto& yColumnPath = yColumn->path();

	auto* curve = new XYCurve(QStringLiteral("curve"));
	plot->addChild(curve);
	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);

	QSignalSpy xColumnSpy(curve, SIGNAL(xDataChanged()));
	QSignalSpy yColumnSpy(curve, SIGNAL(yDataChanged()));

	// remove the second column in the spreadsheet
	// the x-column in the curve is invalidated, the old path is still valid so we can restore later
	sheet->removeColumns(1, 1);
	QCOMPARE(curve->xColumn(), xColumn);
	QCOMPARE(curve->xColumnPath(), xColumnPath);
	QCOMPARE(xColumnSpy.count(), 0);

	QCOMPARE(curve->yColumn(), nullptr);
	QCOMPARE(curve->yColumnPath(), yColumnPath);
	QCOMPARE(yColumnSpy.count(), 1); // data changed signal has to be emitted to notify the parent plot

	// undo the removal and check again
	project.undoStack()->undo();
	QCOMPARE(curve->xColumn(), xColumn);
	QCOMPARE(curve->xColumnPath(), xColumnPath);
	QCOMPARE(xColumnSpy.count(), 0);

	QCOMPARE(curve->yColumn(), yColumn);
	QCOMPARE(curve->yColumnPath(), yColumnPath);
	QCOMPARE(yColumnSpy.count(), 2);
}

void CartesianPlotTest::spreadsheetRemove() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(false);

	auto* sheet = new Spreadsheet(QStringLiteral("data"), false);
	project.addChild(sheet);
	sheet->setColumnCount(2);

	const auto& columns = sheet->children<Column>();
	QCOMPARE(columns.count(), 2);
	const auto* xColumn = columns.at(0);
	const auto* yColumn = columns.at(1);
	const auto& xColumnPath = xColumn->path();
	const auto& yColumnPath = yColumn->path();

	auto* curve = new XYCurve(QStringLiteral("curve"));
	plot->addChild(curve);
	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);

	QSignalSpy xColumnSpy(curve, SIGNAL(xDataChanged()));
	QSignalSpy yColumnSpy(curve, SIGNAL(yDataChanged()));

	// remove the spreadsheet
	// the x- and y-columns in the curve are invalidated, the old paths are still valid so we can restore later
	project.removeChild(sheet);
	QCOMPARE(curve->xColumn(), nullptr);
	QCOMPARE(curve->xColumnPath(), xColumnPath);
	QCOMPARE(xColumnSpy.count(), 1);

	QCOMPARE(curve->yColumn(), nullptr);
	QCOMPARE(curve->yColumnPath(), yColumnPath);
	QCOMPARE(yColumnSpy.count(), 1);

	// undo the removal and check again
	project.undoStack()->undo();
	QCOMPARE(curve->xColumn(), xColumn);
	QCOMPARE(curve->xColumnPath(), xColumnPath);
	QCOMPARE(xColumnSpy.count(), 2);

	QCOMPARE(curve->yColumn(), yColumn);
	QCOMPARE(curve->yColumnPath(), yColumnPath);
	QCOMPARE(yColumnSpy.count(), 2);
}

/*!
 * tests the handling of z-values on changes in the child hierarchy.
 */
void CartesianPlotTest::zValueAfterAddMoveRemove() {
	LOAD_PROJECT_HISTOGRAM_FIT_CURVE

	// after the objects were added, the order of children is
	// * histogram
	// * curve1
	// * curve2
	int zValueHistogram = h->graphicsItem()->zValue();
	int zValueCurve1 = curve1->graphicsItem()->zValue();
	int zValueCurve2 = curve2->graphicsItem()->zValue();
	QVERIFY(zValueHistogram < zValueCurve1);
	QVERIFY(zValueCurve1 < zValueCurve2);

	// move curve1 up
	plot->moveChild(curve1, -1);

	// after the move, the order of children is
	// * curve1
	// * histogram
	// * curve2
	zValueHistogram = h->graphicsItem()->zValue();
	zValueCurve1 = curve1->graphicsItem()->zValue();
	zValueCurve2 = curve2->graphicsItem()->zValue();
	QVERIFY(zValueCurve1 < zValueHistogram);
	QVERIFY(zValueHistogram < zValueCurve2);

	// move histogram down
	plot->moveChild(h, 1);

	// after the move, the order of children is
	// * curve1
	// * curve2
	// * histogram
	zValueHistogram = h->graphicsItem()->zValue();
	zValueCurve1 = curve1->graphicsItem()->zValue();
	zValueCurve2 = curve2->graphicsItem()->zValue();
	QVERIFY(zValueCurve1 < zValueCurve2);
	QVERIFY(zValueCurve2 < zValueHistogram);

	// remove curve2
	plot->removeChild(curve2);

	// after the remove, the order of children is
	// * curve1
	// * histogram
	zValueHistogram = h->graphicsItem()->zValue();
	zValueCurve1 = curve1->graphicsItem()->zValue();
	QVERIFY(zValueCurve1 < zValueHistogram);
}

QTEST_MAIN(CartesianPlotTest)
