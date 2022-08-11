/*
	File                 : XYCurveTest.cpp
	Project              : LabPlot
	Description          : Tests for XYCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "RetransformTest.h"

#include "backend/core/Project.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#include "kdefrontend/MainWin.h"

#include <QAction>

#define COMPARE(actual, expected, message)                                                                                                                     \
	do {                                                                                                                                                       \
		if (!QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__)) {                                                                      \
			qDebug() << message;                                                                                                                               \
			return;                                                                                                                                            \
		}                                                                                                                                                      \
	} while (false)

using Dimension = CartesianCoordinateSystem::Dimension;

void RetransformTest::TestLoadProject() {
	RetransformCallCounter c;
	Project project;

	// Does not work during load.
	// connect(&project, &Project::aspectAdded, &c, &RetransformCallCounter::aspectAdded);

	project.load(QFINDTESTDATA(QLatin1String("data/p1.lml")));

	QHash<QString, int> h = {{"Project/Worksheet/xy-plot", 1},
							 {"Project/Worksheet/xy-plot/x", 1},
							 {"Project/Worksheet/xy-plot/y", 1},
							 {"Project/Worksheet/xy-plot/sin", 1},
							 {"Project/Worksheet/xy-plot/cos", 1},
							 {"Project/Worksheet/xy-plot/tan", 1},
							 {"Project/Worksheet/xy-plot/y-axis", 1},
							 {"Project/Worksheet/xy-plot/legend", 1},
							 {"Project/Worksheet/xy-plot/plotImage", 1},
							 {"Project/Worksheet/xy-plot/plotText", 1},
							 {"Project/Worksheet/Text Label", 1},
							 {"Project/Worksheet/Image", 1},
							 {"Project/Worksheet/plot2", 1},
							 {"Project/Worksheet/plot2/x", 1},
							 {"Project/Worksheet/plot2/y", 1},
							 {"Project/Worksheet/plot2/xy-curve", 1}};

	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);
	for (auto& child : children) {
		int expectedCallCount = 0;
		const QString& path = child->path();
		if (h.contains(path))
			expectedCallCount = h.value(path);
		COMPARE(c.callCount(child), expectedCallCount, path);
	}
}

void RetransformTest::TestResizeWindows() {
	RetransformCallCounter c;
	Project project;
	project.load(QFINDTESTDATA(QLatin1String("data/p1.lml")));

	const auto& worksheets = project.children(AspectType::Worksheet);
	QCOMPARE(worksheets.count(), 1);
	auto worksheet = static_cast<Worksheet*>(worksheets.at(0));
	auto* view = static_cast<WorksheetView*>(worksheet->view());

	view->resize(100, 100);
	view->processResize();

	for (const auto& child : project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive))
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);

	view->resize(1000, 1000);
	view->processResize();

	// Check that every element is exactly called once
	QHash<QString, int> h = {{"Project/Worksheet/xy-plot", 1},
							 {"Project/Worksheet/xy-plot/x", 1},
							 {"Project/Worksheet/xy-plot/y", 1},
							 {"Project/Worksheet/xy-plot/sin", 1},
							 {"Project/Worksheet/xy-plot/cos", 1},
							 {"Project/Worksheet/xy-plot/tan", 1},
							 {"Project/Worksheet/xy-plot/y-axis", 1},
							 {"Project/Worksheet/xy-plot/legend", 1},
							 {"Project/Worksheet/xy-plot/plotImage", 1},
							 {"Project/Worksheet/xy-plot/plotText", 1},
							 //{"Project/Worksheet/Text Label", 1}, // TODO: turn on when fixed
							 //{"Project/Worksheet/Image", 1},  // TODO: turn on when fixed
							 {"Project/Worksheet/plot2", 1},
							 {"Project/Worksheet/plot2/x", 1},
							 {"Project/Worksheet/plot2/y", 1},
							 {"Project/Worksheet/plot2/xy-curve", 1}};

	QCOMPARE(c.elementLogCount(false), h.count());
	QHash<QString, int>::const_iterator i;
	for (i = h.constBegin(); i != h.constEnd(); ++i)
		QCOMPARE(c.callCount(i.key()), 1);
}

/*!
 * \brief RetransformTest::TestZoomSelectionAutoscale
 * Check that retransform and retransform scale is called correctly during zoom and autoscale. Check
 * also the number of calls of the retransforms
 */
void RetransformTest::TestZoomSelectionAutoscale() {
	RetransformCallCounter c;
	Project project;

	project.load(QFINDTESTDATA(QLatin1String("data/p1.lml")));
	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);

	// Spreadsheet "Spreadsheet"
	// Column "x"
	// Column "sin"
	// Column "cos"
	// Column "tan"
	// Worksheet "Worksheet"
	// CartesianPlot "xy-plot"
	// Axis "x"
	// Axis "y"
	// XYCurve "sin"
	// XYCurve "cos"
	// XYCurve "tan"
	// Axis "y-axis"
	// Legend "legend"
	// TextLabel "plotText"
	// Image "plotImage"
	// TextLabel "Text Label"
	// Image "Image"
	// --- Second plot
	// CartesianPlot "plot2"
	// Axis "x"
	// Axis "y"
	// XYCurve "xy-curve"
	QCOMPARE(children.length(), 22);
	for (const auto& child : children)
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);

	for (const auto& plot : project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive)) {
		connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);
	}

	auto* worksheet = project.child<Worksheet>(0);
	QVERIFY(worksheet);

	auto* view = static_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view);

	auto* plot = worksheet->child<CartesianPlot>(0);
	QVERIFY(plot);
	QCOMPARE(plot->name(), QLatin1String("xy-plot"));

	auto* plot2 = worksheet->child<CartesianPlot>(1);
	QVERIFY(plot2);
	QCOMPARE(plot2->name(), QLatin1String("plot2"));

	QAction a(nullptr);
	a.setData(static_cast<int>(CartesianPlot::MouseMode::ZoomXSelection));
	view->cartesianPlotMouseModeChanged(&a);

	QCOMPARE(c.elementLogCount(false), 0);
	QVERIFY(c.calledExact(0, false));

	emit plot->mousePressZoomSelectionModeSignal(QPointF(0.2, -150));
	emit plot->mouseMoveZoomSelectionModeSignal(QPointF(0.6, 100));
	emit plot->mouseReleaseZoomSelectionModeSignal();

	// x and y are called only once
	QCOMPARE(c.logsXScaleRetransformed.count(), 2); // 2 plots with each one x axis
	QCOMPARE(c.logsXScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsXScaleRetransformed.at(1).plot, plot2);
	QCOMPARE(c.logsXScaleRetransformed.at(1).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.count(),
			 3); // there are two vertical ranges (sin,cos and tan range) for the first plot and one y axis for the second plot
	QCOMPARE(c.logsYScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.at(1).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(1).index, 1);
	QCOMPARE(c.logsYScaleRetransformed.at(2).plot, plot2);
	QCOMPARE(c.logsYScaleRetransformed.at(2).index, 0);

	// Check that every element is exactly called once
	// plot it self does not change so retransform is not called on cartesianplotPrivate
	QStringList list = {
		"Project/Worksheet/xy-plot/x",
		"Project/Worksheet/xy-plot/y",
		"Project/Worksheet/xy-plot/sin",
		"Project/Worksheet/xy-plot/cos",
		"Project/Worksheet/xy-plot/tan",
		"Project/Worksheet/xy-plot/y-axis",
		// not neccesary to retransform legend, but is difficult to
		// distinguish so let it in, because it does not cost that much performance
		"Project/Worksheet/xy-plot/legend",
		"Project/Worksheet/xy-plot/plotText",
		"Project/Worksheet/xy-plot/plotImage",
		/* second plot starting */
		"Project/Worksheet/plot2/x",
		"Project/Worksheet/plot2/y",
		"Project/Worksheet/plot2/xy-curve",
	};
	QCOMPARE(c.elementLogCount(false), list.count());
	for (auto& s : list)
		QCOMPARE(c.callCount(s), 1);

	c.resetRetransformCount();
	view->selectItem(plot->graphicsItem());
	a.setData(static_cast<int>(CartesianPlot::NavigationOperation::ScaleAutoX));
	view->cartesianPlotNavigationChanged(&a);

	QCOMPARE(c.elementLogCount(false), list.count());
	for (auto& s : list)
		QCOMPARE(c.callCount(s), 1);

	// x and y are called only once
	QCOMPARE(c.logsXScaleRetransformed.count(), 2); // 2 plots with each one x axis
	QCOMPARE(c.logsXScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsXScaleRetransformed.at(0).index, 0); // first x axis of first plot
	QCOMPARE(c.logsXScaleRetransformed.at(1).plot, plot2);
	QCOMPARE(c.logsXScaleRetransformed.at(1).index, 0); // first x axis of second plot
	QCOMPARE(c.logsYScaleRetransformed.count(),
			 3); // there are two vertical ranges (sin,cos and tan range) for the first plot and one y axis for the second plot
	QCOMPARE(c.logsYScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0); // first y axis of first plot
	QCOMPARE(c.logsYScaleRetransformed.at(1).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(1).index, 1); // second y axis of first plot
	QCOMPARE(c.logsYScaleRetransformed.at(2).plot, plot2);
	QCOMPARE(c.logsYScaleRetransformed.at(2).index, 0); // first y axis of second plot
}

/*!
 * \brief RetransformTest::TestPadding
 * Check that during a padding change retransform and retransform scale is called
 */
void RetransformTest::TestPadding() {
	RetransformCallCounter c;
	Project project;

	project.load(QFINDTESTDATA(QLatin1String("data/p1.lml")));
	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);

	// Spreadsheet "Spreadsheet"
	// Column "x"
	// Column "sin"
	// Column "cos"
	// Column "tan"
	// Worksheet "Worksheet"
	// CartesianPlot "xy-plot"
	// Axis "x"
	// Axis "y"
	// XYCurve "sin"
	// XYCurve "cos"
	// XYCurve "tan"
	// Axis "y-axis"
	// Legend "legend"
	// TextLabel "plotText"
	// Image "plotImage"
	// TextLabel "Text Label"
	// Image "Image"
	// --- Second plot
	// CartesianPlot "plot2"
	// Axis "x"
	// Axis "y"
	// XYCurve "xy-curve"
	QCOMPARE(children.length(), 22);
	for (const auto& child : children)
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);

	for (const auto& plot : project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive))
		connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);

	auto* worksheet = project.child<Worksheet>(0);
	QVERIFY(worksheet);

	auto* view = static_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view);

	auto* plot = worksheet->child<CartesianPlot>(0);
	QVERIFY(plot);
	QCOMPARE(plot->name(), QLatin1String("xy-plot"));

	// Check that every element is exactly called once
	// TODO: set to 6. legend should not retransform
	// plot it self does not change so retransform is not called on cartesianplotPrivate
	QStringList list = {"Project/Worksheet/xy-plot", // datarect changed, so plot must also be retransformed
						"Project/Worksheet/xy-plot/x",
						"Project/Worksheet/xy-plot/y",
						"Project/Worksheet/xy-plot/sin",
						"Project/Worksheet/xy-plot/cos",
						"Project/Worksheet/xy-plot/tan",
						"Project/Worksheet/xy-plot/y-axis",
						"Project/Worksheet/xy-plot/legend",
						"Project/Worksheet/xy-plot/plotText",
						"Project/Worksheet/xy-plot/plotImage"};
	double hPad = plot->horizontalPadding();
	plot->setHorizontalPadding(hPad + 10);

	QCOMPARE(c.elementLogCount(false), list.count());
	for (auto& s : list)
		QCOMPARE(c.callCount(s), 1);

	// x and y are called only once
	QCOMPARE(c.logsXScaleRetransformed.count(), 1);
	QCOMPARE(c.logsXScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 2); // there are two vertical ranges (sin,cos and tan range)
	QCOMPARE(c.logsYScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.at(1).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(1).index, 1);

	c.resetRetransformCount();

	list = QStringList({// data rect of the plot does not change, so retransforming the
						// plot is not needed
						"Project/Worksheet/xy-plot/x",
						"Project/Worksheet/xy-plot/y",
						"Project/Worksheet/xy-plot/sin",
						"Project/Worksheet/xy-plot/cos",
						"Project/Worksheet/xy-plot/tan",
						"Project/Worksheet/xy-plot/y-axis",
						"Project/Worksheet/xy-plot/legend",
						"Project/Worksheet/xy-plot/plotText",
						"Project/Worksheet/xy-plot/plotImage"});
	plot->navigate(-1, CartesianPlot::NavigationOperation::ScaleAuto);

	QCOMPARE(c.elementLogCount(false), list.count());
	for (auto& s : list)
		QCOMPARE(c.callCount(s), 1);

	// x and y are already scaled due to the change of padding
	QCOMPARE(c.logsXScaleRetransformed.count(), 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 0);
}

void RetransformTest::TestCopyPastePlot() {
	Project project;
	auto* ws = new Worksheet("Worksheet");
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* ws2 = new Worksheet("Worksheet2");
	QVERIFY(ws2 != nullptr);
	project.addChild(ws2);

	RetransformCallCounter c;

	p->copy();
	ws2->paste(true);

	auto plots = ws2->children(AspectType::CartesianPlot);
	QCOMPARE(plots.count(), 1);

	// Check that the plot was retransformed after pasting
	QCOMPARE(c.callCount(plots.at(0)), 1);
}

void RetransformTest::TestAddCurve() {
	Project project;
	auto* ws = new Worksheet("Worksheet");
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot("plot");
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	RetransformCallCounter c;

	p->addEquationCurve();

	// check that plot will be recalculated if a curve will be added
	QCOMPARE(c.callCount(p), 1);

	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);

	// Project/Worksheet
	// Project/Worksheet/plot
	// Project/Worksheet/plot/x
	// Project/Worksheet/plot/y
	// Project/Worksheet/plot/f(x) // equation curve
	QCOMPARE(children.length(), 5);
	for (const auto& child : children)
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);

	for (const auto& plot : project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive))
		connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);

	c.resetRetransformCount();

	auto equationCurves = p->children(AspectType::XYEquationCurve);
	QCOMPARE(equationCurves.count(), 1);
	auto* equationCurve = static_cast<XYEquationCurve*>(equationCurves.at(0));
	XYEquationCurve::EquationData data;
	data.count = 100;
	data.expression1 = "x";
	data.expression2 = "";
	data.min = "0";
	data.max = "10";
	data.type = XYEquationCurve::EquationType::Cartesian;
	equationCurve->setEquationData(data);

	auto list = QStringList({"Project/Worksheet/plot/x", "Project/Worksheet/plot/y", "Project/Worksheet/plot/f(x)"});
	QCOMPARE(c.elementLogCount(false), list.count());
	for (auto& s : list)
		QCOMPARE(c.callCount(s), 1);

	// x and y are called only once
	QCOMPARE(c.logsXScaleRetransformed.count(), 1);
	QCOMPARE(c.logsXScaleRetransformed.at(0).plot, p);
	QCOMPARE(c.logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 1);
	QCOMPARE(c.logsYScaleRetransformed.at(0).plot, p);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0);
}

void RetransformTest::TestBarPlotOrientation() {
	RetransformCallCounter c;
	Project project;

	project.load(QFINDTESTDATA(QLatin1String("data/barplot_test.lml")));
	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);

	// Spreadsheet "Spreadsheet"
	// Column "labels"
	// Column "1"
	// Column "2"
	// Column "3"
	// Column "4"
	// Worksheet "Worksheet"
	// CartesianPlot "xy-plot"
	// Axis "x"
	// Axis "x2"
	// Axis "y"
	// Axis "y2"
	// BarPlot "BarPlot"
	QCOMPARE(children.length(), 13);
	for (const auto& child : children)
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);

	for (const auto& plot : project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive)) {
		connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);
	}

	auto barplots = project.children(AspectType::BarPlot, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(barplots.length(), 1);
	auto barplot = static_cast<BarPlot*>(barplots.at(0));
	QCOMPARE(barplot->name(), "Bar Plot");

	// Trigger retransform
	barplot->setOrientation(BarPlot::Orientation::Horizontal);

	auto* worksheet = project.child<Worksheet>(0);
	QVERIFY(worksheet);
	auto* plot = worksheet->child<CartesianPlot>(0);
	QVERIFY(plot);
	QCOMPARE(plot->name(), QLatin1String("xy-plot"));

	// x and y are called only once
	QCOMPARE(c.logsXScaleRetransformed.count(), 1); // one plot with 2 x-Axes but both are using the same range so 1
	QCOMPARE(c.logsXScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 1); // one plot with 2 x-Axes but both are using the same range so 1
	QCOMPARE(c.logsYScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0);
}

void RetransformTest::TestZoom() {
	RetransformCallCounter c;
	Project project;

	project.load(QFINDTESTDATA(QLatin1String("data/TestZoom.lml")));
	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);

	// Spreadsheet "Spreadsheet"
	// Column "1"
	// Column "2"
	// Worksheet "Worksheet-Spreadsheet"
	// CartesianPlot "Plot-Spreadsheet"
	// Axis "x"
	// Axis "x2"
	// Axis "y"
	// Axis "y2"
	// XYCurve "2"
	QCOMPARE(children.length(), 10);
	for (const auto& child : children)
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);

	auto plots = project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.length(), 1);
	auto* plot = static_cast<CartesianPlot*>(plots[0]);
	connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);

	plot->zoomInX();

	// x and y are called only once
	QCOMPARE(c.logsXScaleRetransformed.count(), 1); // one plot with 2 x-Axes but both are using the same range so 1
	QCOMPARE(c.logsXScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 1); // one plot with 2 x-Axes but both are using the same range so 1
	QCOMPARE(c.logsYScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0);
}

// ############################################################################################
// ############################################################################################
// ############################################################################################

QHash<QString, int> RetransformCallCounter::statistic(bool includeSuppressed) {
	QHash<QString, int> result;
	for (auto& log : logsRetransformed) {
		const auto& path = log.aspect->path();
		if (!includeSuppressed && log.suppressed)
			continue;

		if (!result.contains(path))
			result.insert(path, 1);
		else
			result.insert(path, result.take(path) + 1);
	}
	return result;
}

int RetransformCallCounter::elementLogCount(bool includeSuppressed) {
	return statistic(includeSuppressed).count();
}

bool RetransformCallCounter::calledExact(int requiredCallCount, bool includeSuppressed) {
	const auto& result = statistic(includeSuppressed);
	QHash<QString, int>::const_iterator i;
	for (i = result.constBegin(); i != result.constEnd(); ++i) {
		if (i.value() != requiredCallCount) {
			qDebug() << "Expected CallCount: " << requiredCallCount << ", Current: " << i.value() << ". " << i.key();
			return false;
		}
	}
	return true;
}

int RetransformCallCounter::callCount(const QString& path) {
	const auto& result = statistic(false);
	if (!result.contains(path))
		return 0;

	return result.value(path);
}

int RetransformCallCounter::callCount(const AbstractAspect* aspect) {
	return aspect->retransformCalled();
}

void RetransformCallCounter::resetRetransformCount() {
	logsRetransformed.clear();
	logsXScaleRetransformed.clear();
	logsYScaleRetransformed.clear();
}

void RetransformCallCounter::aspectRetransformed(const AbstractAspect* sender, bool suppressed) {
	logsRetransformed.append({sender, suppressed});
}

void RetransformCallCounter::retransformScaleCalled(const CartesianPlot* plot, const Dimension dim, int index) {
	switch (dim) {
	case Dimension::X:
		logsXScaleRetransformed.append({plot, index});
		break;
	case Dimension::Y:
		logsYScaleRetransformed.append({plot, index});
		break;
	}
}

void RetransformCallCounter::aspectAdded(const AbstractAspect* aspect) {
	connect(aspect, &AbstractAspect::retransformCalledSignal, this, &RetransformCallCounter::aspectRetransformed);
}

// Test change data

QTEST_MAIN(RetransformTest)
