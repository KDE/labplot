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
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/AxisPrivate.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#include "kdefrontend/dockwidgets/AxisDock.h"
#include "kdefrontend/dockwidgets/CartesianPlotDock.h"
#include "kdefrontend/dockwidgets/XYCurveDock.h"

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

// Problem in this project was that the second axis labels are not loaded. Zooming in/out once shows the correct range
void RetransformTest::TestLoadProject2() {
	RetransformCallCounter c;
	Project project;

	project.load(QFINDTESTDATA(QLatin1String("data/bars_dis_004.lml")));

	QHash<QString, int> h = {{"Project/Worksheet - Spreadsheet/Plot - Spreadsheet", 1},
							 {"Project/Worksheet - Spreadsheet/Plot - Spreadsheet/x", 1},
							 {"Project/Worksheet - Spreadsheet/Plot - Spreadsheet/y", 1},
							 {"Project/Worksheet - Spreadsheet/Plot - Spreadsheet/x2", 1},
							 {"Project/Worksheet - Spreadsheet/Plot - Spreadsheet/y2", 1},
							 {"Project/Worksheet - Spreadsheet/Plot - Spreadsheet/Frequency", 1},
							 {"Project/Worksheet - Spreadsheet/Plot - Spreadsheet/Cumulative", 1},
							 {"Project/Worksheet - Spreadsheet/Plot - Spreadsheet/legend", 1}};

	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);
	for (auto& child : children) {
		int expectedCallCount = 0;
		const QString& path = child->path();
		if (h.contains(path))
			expectedCallCount = h.value(path);
		COMPARE(c.callCount(child), expectedCallCount, path);
	}

	// check axis ranges
	auto axes = project.children(AspectType::Axis, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(axes.length(), 4);
	auto* xAxis = axes.at(0);
	auto* xAxis2 = axes.at(1);
	auto* yAxis1 = axes.at(2);
	auto* yAxis2 = axes.at(3);

	QCOMPARE(xAxis->name(), "x");
	QCOMPARE(xAxis2->name(), "x2");
	QCOMPARE(yAxis1->name(), "y");
	QCOMPARE(yAxis2->name(), "y2");

	// xAxis2 does not have any labels
	QVector<QString> refString = {"161.2", "166.7", "172.2", "177.8", "183.3", "188.8", "194.4"};
	COMPARE_STRING_VECTORS(static_cast<Axis*>(xAxis)->tickLabelStrings(), refString);
	QVector<double> ref = {0, 20, 40, 60, 80, 100};
	COMPARE_DOUBLE_VECTORS(static_cast<Axis*>(yAxis1)->tickLabelValues(), ref);
	ref = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
	COMPARE_DOUBLE_VECTORS(static_cast<Axis*>(yAxis2)->tickLabelValues(), ref);
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

	auto* sheet = new Spreadsheet("Spreadsheet", false);
	sheet->setColumnCount(2);
	sheet->setRowCount(100);

	QVector<int> xData;
	QVector<double> yData;
	for (int i = 0; i < 100; i++) {
		xData.append(i);
		yData.append(i);
	}
	auto* xColumn = sheet->column(0);
	xColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
	xColumn->replaceInteger(0, xData);
	auto* yColumn = sheet->column(1);
	yColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
	yColumn->replaceValues(0, yData);

	project.addChild(sheet);

	auto* worksheet = new Worksheet("Worksheet - Spreadsheet");
	project.addChild(worksheet);

	auto* p = new CartesianPlot("Plot - Spreadsheet");
	p->setType(CartesianPlot::Type::FourAxes); // Otherwise no axis are created
	worksheet->addChild(p);

	auto* curve = new XYCurve("curve");
	p->addChild(curve);
	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);

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
	// XYCurve "curve"
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

void RetransformTest::TestImportCSV() {
	Project project;
	auto* ws = new Worksheet("Worksheet");
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheet = new Spreadsheet("test", false);
	spreadsheet->setColumnCount(2);
	spreadsheet->setRowCount(3);

	auto* xCol = spreadsheet->column(0);
	xCol->replaceValues(0, QVector<double>({1, 2, 3}));

	auto* yCol = spreadsheet->column(1);
	yCol->replaceValues(0, QVector<double>({2, 3, 4}));

	QCOMPARE(spreadsheet->rowCount(), 3);
	QCOMPARE(spreadsheet->columnCount(), 2);

	project.addChild(spreadsheet);
	auto* p = new CartesianPlot("plot");
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve = new XYCurve("xy-curve");
	curve->setXColumn(xCol);
	curve->setYColumn(yCol);
	p->addChild(curve);

	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);

	RetransformCallCounter c;
	// Worksheet "Worksheet"
	// CartesianPlot "plot"
	// Axis "x"
	// Axis "y"
	// XYCurve "xy-curve"
	// Spreadsheet "test"
	// Column "1"
	// Column "2"
	QCOMPARE(children.length(), 8);
	for (const auto& child : children) {
		qDebug() << child->name();
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);
	}

	for (const auto& plot : project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive))
		connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);

	// check axis ranges
	auto axes = p->children(AspectType::Axis, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(axes.length(), 2);
	auto* xAxis = axes.at(0);
	QVector<double> ref = {1, 1.5, 2, 2.5, 3};
	COMPARE_DOUBLE_VECTORS(static_cast<Axis*>(xAxis)->tickLabelValues(), ref);
	auto* yAxis = axes.at(1);
	ref = {2, 2.5, 3, 3.5, 4};
	COMPARE_DOUBLE_VECTORS(static_cast<Axis*>(yAxis)->tickLabelValues(), ref);

	QTemporaryFile file;
	QCOMPARE(file.open(), true);
	QVERIFY(file.write("1,2\n10,10\n20,20\n30,30\n") > 0);
	file.close();

	AsciiFilter filter;
	filter.readDataFromFile(file.fileName(), spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet->rowCount(), 3);
	QCOMPARE(spreadsheet->columnCount(), 2);

	xCol = spreadsheet->column(0);
	QCOMPARE(xCol->name(), "1");
	QCOMPARE(xCol->valueAt(0), 10);
	QCOMPARE(xCol->valueAt(1), 20);
	QCOMPARE(xCol->valueAt(2), 30);

	yCol = spreadsheet->column(1);
	QCOMPARE(yCol->name(), "2");
	QCOMPARE(yCol->valueAt(0), 10);
	QCOMPARE(yCol->valueAt(1), 20);
	QCOMPARE(yCol->valueAt(2), 30);

	ref = {10, 15, 20, 25, 30};
	auto tickLabelValues = static_cast<Axis*>(xAxis)->tickLabelValues();
	COMPARE_DOUBLE_VECTORS(tickLabelValues, ref);
	ref = {10, 15, 20, 25, 30};
	COMPARE_DOUBLE_VECTORS(static_cast<Axis*>(yAxis)->tickLabelValues(), ref);

	// x and y are called only once
	QCOMPARE(c.logsXScaleRetransformed.count(), 1); // one plot with 1 x-Axis
	QCOMPARE(c.logsXScaleRetransformed.at(0).plot, p);
	QCOMPARE(c.logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 1); // one plot with 1 x-Axis
	QCOMPARE(c.logsYScaleRetransformed.at(0).plot, p);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0);

	auto list = QStringList({"Project/Worksheet/plot/x", "Project/Worksheet/plot/y", "Project/Worksheet/plot/xy-curve"});
	QCOMPARE(c.elementLogCount(false), list.count());
	for (auto& s : list) {
		qDebug() << s;
		QCOMPARE(c.callCount(s), 1);
	}
}

void RetransformTest::TestSetScale() {
	RetransformCallCounter c;
	Project project;

	auto* sheet = new Spreadsheet("Spreadsheet", false);
	sheet->setColumnCount(2);
	sheet->setRowCount(100);

	QVector<int> xData;
	QVector<double> yData;
	for (int i = 0; i < 100; i++) {
		xData.append(i);
		yData.append(i);
	}
	auto* xColumn = sheet->column(0);
	xColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
	xColumn->replaceInteger(0, xData);
	auto* yColumn = sheet->column(1);
	yColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
	yColumn->replaceValues(0, yData);

	project.addChild(sheet);

	auto* worksheet = new Worksheet("Worksheet");
	project.addChild(worksheet);

	auto* p = new CartesianPlot("Plot");
	p->setType(CartesianPlot::Type::FourAxes); // Otherwise no axis are created
	worksheet->addChild(p);

	auto* curve = new XYCurve("curve");
	p->addChild(curve);
	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);

	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);

	// Spreadsheet "Spreadsheet"
	// Column "1"
	// Column "2"
	// Worksheet "Worksheet"
	// CartesianPlot "Plot"
	// Axis "x"
	// Axis "x2"
	// Axis "y"
	// Axis "y2"
	// XYCurve "curve"
	QCOMPARE(children.length(), 10);
	for (const auto& child : children)
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);

	auto plots = project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.length(), 1);
	auto* plot = static_cast<CartesianPlot*>(plots[0]);
	connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);

	c.resetRetransformCount();

	auto list = QStringList({// data rect of the plot does not change, so retransforming the
							 // plot is not needed
							 "Project/Worksheet/Plot/x",
							 "Project/Worksheet/Plot/x2",
							 "Project/Worksheet/Plot/y",
							 "Project/Worksheet/Plot/y2",
							 "Project/Worksheet/Plot/curve"});

	plot->setRangeScale(Dimension::X, 0, RangeT::Scale::Log10);

	QCOMPARE(c.elementLogCount(false), list.count());
	for (auto& s : list)
		QCOMPARE(c.callCount(s), 1);

	// x and y are called only once
	QCOMPARE(c.logsXScaleRetransformed.count(), 1); // one plot with 2 x-Axes but both are using the same range so 1
	QCOMPARE(c.logsXScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 0);
}

void RetransformTest::TestChangePlotRange() {
	RetransformCallCounter c;
	Project project;

	auto* sheet = new Spreadsheet("Spreadsheet", false);
	sheet->setColumnCount(2);
	sheet->setRowCount(100);

	QVector<int> xData;
	QVector<double> yData;
	for (int i = 0; i < 100; i++) {
		xData.append(i);
		yData.append(i);
	}
	auto* xColumn = sheet->column(0);
	xColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
	xColumn->replaceInteger(0, xData);
	auto* yColumn = sheet->column(1);
	yColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
	yColumn->replaceValues(0, yData);

	project.addChild(sheet);

	auto* worksheet = new Worksheet("Worksheet");
	project.addChild(worksheet);

	auto* p = new CartesianPlot("Plot");
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	worksheet->addChild(p);

	auto* curve = new XYCurve("curve");
	p->addChild(curve);
	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);

	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);

	// Spreadsheet "Spreadsheet"
	// Column "1"
	// Column "2"
	// Worksheet "Worksheet"
	// CartesianPlot "Plot"
	// Axis "x"
	// Axis "y"
	// XYCurve "curve"
	QCOMPARE(children.length(), 8);
	for (const auto& child : children)
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);

	auto plots = project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.length(), 1);
	auto* plot = static_cast<CartesianPlot*>(plots[0]);
	connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);

	c.resetRetransformCount();

	auto list = QStringList({// data rect of the plot does not change, so retransforming the
							 // plot is not needed
							 "Project/Worksheet/Plot/x",
							 "Project/Worksheet/Plot/y",
							 "Project/Worksheet/Plot/curve"});

	CartesianPlotDock dock(nullptr);
	dock.setPlots({plot});
	dock.addXRange();

	QCOMPARE(plot->rangeCount(Dimension::X), 2);

	dock.autoScaleChanged(Dimension::X, 1, false);
	QCOMPARE(plot->autoScale(Dimension::X, 1), false);

	dock.minChanged(Dimension::X, 1, "10");
	dock.maxChanged(Dimension::X, 1, "20");

	// check axis ranges
	auto axes = project.children(AspectType::Axis, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(axes.length(), 2);
	auto* xAxis = static_cast<Axis*>(axes.at(0));
	auto* yAxis = static_cast<Axis*>(axes.at(1));

	QCOMPARE(xAxis->name(), "x");
	QCOMPARE(yAxis->name(), "y");

	//	QVector<QString> refString = {"0", "20", "20", "177.8", "183.3", "188.8", "194.4"};
	//	COMPARE_STRING_VECTORS(static_cast<Axis*>(xAxis)->tickLabelStrings(), refString);
	QVector<double> ref = {0, 20, 40, 60, 80, 100};
	COMPARE_DOUBLE_VECTORS(xAxis->tickLabelValues(), ref);
	ref = {0, 20, 40, 60, 80, 100};
	COMPARE_DOUBLE_VECTORS(yAxis->tickLabelValues(), ref);

	int linesUpdatedCounter = 0;
	connect(curve, &XYCurve::linesUpdated, [&linesUpdatedCounter](const XYCurve*, const QVector<QLineF> lines) {
		// One point before and one point after is used therefore it is not 10, 20
		// se XYCurvePrivate::updateLines() startIndex--; and endIndex++;
		QCOMPARE(lines.at(0).p1().x(), 9);
		QCOMPARE(lines.last().p2().x(), 21);
		linesUpdatedCounter++;
	});
	// switch in first csystem from the first x range to the second x range
	dock.PlotRangeChanged(0, Dimension::X, 1);

	ref = {10, 12, 14, 16, 18, 20};
	COMPARE_DOUBLE_VECTORS(static_cast<Axis*>(xAxis)->tickLabelValues(), ref);
	ref = {0, 20, 40, 60, 80, 100};
	COMPARE_DOUBLE_VECTORS(static_cast<Axis*>(yAxis)->tickLabelValues(), ref);

	auto dataRect = plot->dataRect();

	// check that lines are starting at the beginning of the datarect
	auto line = xAxis->d_func()->lines.at(0);
	QCOMPARE(line.p1().x(), dataRect.left());
	QCOMPARE(line.p2().x(), dataRect.right());

	auto lines = curve->d_func()->m_lines;
	QCOMPARE(lines.at(0).p1().x(), dataRect.left());
	QCOMPARE(lines.last().p2().x(), dataRect.right());

	QCOMPARE(linesUpdatedCounter, 1);
}

void RetransformTest::TestChangePlotRangeElement() {
	// Change the plotrange of one of the elements

	RetransformCallCounter c;
	Project project;

	auto* sheet = new Spreadsheet("Spreadsheet", false);
	sheet->setColumnCount(2);
	sheet->setRowCount(100);

	QVector<int> xData;
	QVector<double> yData;
	for (int i = 0; i < 100; i++) {
		xData.append(i);
		yData.append(i);
	}
	auto* xColumn = sheet->column(0);
	xColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
	xColumn->replaceInteger(0, xData);
	auto* yColumn = sheet->column(1);
	yColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
	yColumn->replaceValues(0, yData);

	project.addChild(sheet);

	auto* worksheet = new Worksheet("Worksheet");
	project.addChild(worksheet);

	auto* p = new CartesianPlot("Plot");
	p->setType(CartesianPlot::Type::FourAxes); // Otherwise no axis are created
	worksheet->addChild(p);

	auto* curve = new XYCurve("curve");
	p->addChild(curve);
	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);

	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);

	// Spreadsheet "Spreadsheet"
	// Column "1"
	// Column "2"
	// Worksheet "Worksheet"
	// CartesianPlot "Plot"
	// Axis "x"
	// Axis "x2"
	// Axis "y"
	// Axis "y2"
	// XYCurve "curve"
	QCOMPARE(children.length(), 10);
	for (const auto& child : children)
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);

	auto plots = project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.length(), 1);
	auto* plot = static_cast<CartesianPlot*>(plots[0]);
	connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);

	// check axes
	auto axes = project.children(AspectType::Axis, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(axes.length(), 4);
	auto* xAxis1 = static_cast<Axis*>(axes.at(0));
	auto* xAxis2 = static_cast<Axis*>(axes.at(1));
	auto* yAxis1 = static_cast<Axis*>(axes.at(2));
	auto* yAxis2 = static_cast<Axis*>(axes.at(3));
	QCOMPARE(xAxis1->name(), "x");
	QCOMPARE(xAxis2->name(), "x2");
	QCOMPARE(yAxis1->name(), "y");
	QCOMPARE(yAxis2->name(), "y2");

	auto list = QStringList({// data rect of the plot does not change, so retransforming the
							 // plot is not needed
							 "Project/Worksheet/Plot/x",
							 "Project/Worksheet/Plot/y",
							 "Project/Worksheet/Plot/x2",
							 "Project/Worksheet/Plot/y2",
							 "Project/Worksheet/Plot/curve"});

	CartesianPlotDock dock(nullptr);
	dock.setPlots({plot});
	dock.addYRange();
	QCOMPARE(plot->rangeCount(Dimension::Y), 2);
	dock.addPlotRange();
	QCOMPARE(plot->coordinateSystemCount(), 2);

	dock.autoScaleChanged(Dimension::Y, 1, false);
	QCOMPARE(plot->autoScale(Dimension::Y, 1), false);
	dock.minChanged(Dimension::Y, 1, "10");
	dock.maxChanged(Dimension::Y, 1, "20");

	// Csystem1:
	// x 0..1
	// y 0..1
	// Csystem2:
	// x 0..1
	// y 10..20

	c.resetRetransformCount();

	dock.PlotRangeChanged(1, Dimension::Y, 1);

	QCOMPARE(c.logsXScaleRetransformed.count(), 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 1);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 1);

	QCOMPARE(plot->range(Dimension::X, 0).start(), 0);
	QCOMPARE(plot->range(Dimension::X, 0).end(), 100);
	QCOMPARE(plot->range(Dimension::Y, 0).start(), 0);
	QCOMPARE(plot->range(Dimension::Y, 0).end(), 100);
	QCOMPARE(plot->range(Dimension::Y, 1).start(), 10);
	QCOMPARE(plot->range(Dimension::Y, 1).end(), 20);

	c.resetRetransformCount();

	{
		AxisDock d(nullptr);
		d.setAxes({yAxis2});
		QCOMPARE(yAxis2->coordinateSystemIndex(), 0);
		d.plotRangeChanged(1); // change plotrange of yAxis2 from 0 to 1
		QCOMPARE(yAxis2->coordinateSystemIndex(), 1);
	}

	// the y scale of cSystem1 and the y scale of CSystem2 shall be retransformed, because
	// an element switches from y1 to y2
	// xScale shall not be retransformed because it is common for both cSystems
	QCOMPARE(c.logsXScaleRetransformed.count(), 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 0);
	//	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0); // range did not change so retransformScale was not called
	//	QCOMPARE(c.logsYScaleRetransformed.at(1).index, 1); // range did not change so retransformScale was not called

	const auto dataRect = plot->dataRect();

	// Check that both lines go from bottom to top
	{
		QCOMPARE(yAxis1->d_func()->lines.count(), 1);
		const auto line = yAxis1->d_func()->lines.at(0);
		QCOMPARE(line.p1().y(), dataRect.bottom());
		QCOMPARE(line.p2().y(), dataRect.top());
	}

	{
		QCOMPARE(yAxis2->d_func()->lines.count(), 1);
		const auto line = yAxis2->d_func()->lines.at(0);
		QCOMPARE(line.p1().y(), dataRect.bottom());
		QCOMPARE(line.p2().y(), dataRect.top());
	}
}

void RetransformTest::TestChangePlotRangeElement2() {
	// Change the plotrange of one of the elements

	RetransformCallCounter c;
	Project project;

	auto* sheet = new Spreadsheet("Spreadsheet", false);
	sheet->setColumnCount(4);
	sheet->setRowCount(100);

	project.addChild(sheet);

	auto* worksheet = new Worksheet("Worksheet");
	project.addChild(worksheet);

	auto* p = new CartesianPlot("Plot");
	p->setType(CartesianPlot::Type::FourAxes); // Otherwise no axis are created
	worksheet->addChild(p);
	p->setNiceExtend(false);

	auto* curve = new XYCurve("curve");
	p->addChild(curve);
	{
		QVector<int> xData;
		QVector<double> yData;
		for (int i = 1; i < 101; i++) {
			xData.append(i);
			yData.append(i);
		}
		auto* xColumn = sheet->column(0);
		xColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
		xColumn->replaceInteger(0, xData);
		auto* yColumn = sheet->column(1);
		yColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
		yColumn->replaceValues(0, yData);
		curve->setXColumn(xColumn);
		curve->setYColumn(yColumn);
	}

	auto* curve2 = new XYCurve("curve2");
	p->addChild(curve2);
	{
		QVector<int> xData;
		QVector<double> yData;
		for (int i = 1; i < 31; i++) { // different to the above
			xData.append(i);
			yData.append(i);
		}
		auto* xColumn = sheet->column(2);
		xColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
		xColumn->replaceInteger(0, xData);
		auto* yColumn = sheet->column(3);
		yColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
		yColumn->replaceValues(0, yData);
		curve2->setXColumn(xColumn);
		curve2->setYColumn(yColumn);
	}

	// SAVE_PROJECT("TestChangePlotRangeElement2.lml");

	QCOMPARE(p->range(Dimension::X, 0).start(), 1);
	QCOMPARE(p->range(Dimension::X, 0).end(), 100);
	QCOMPARE(p->range(Dimension::Y, 0).start(), 1);
	QCOMPARE(p->range(Dimension::Y, 0).end(), 100);

	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);

	// Spreadsheet "Spreadsheet"
	// Column "1"
	// Column "2"
	// Column "3"
	// Column "4"
	// Worksheet "Worksheet"
	// CartesianPlot "Plot"
	// Axis "x"
	// Axis "x2"
	// Axis "y"
	// Axis "y2"
	// XYCurve "curve"
	// XYCurve "curve2"
	QCOMPARE(children.length(), 13);
	for (const auto& child : children)
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);

	auto plots = project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.length(), 1);
	auto* plot = static_cast<CartesianPlot*>(plots[0]);
	connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);

	// check axis ranges
	auto axes = project.children(AspectType::Axis, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(axes.length(), 4);
	auto* xAxis1 = static_cast<Axis*>(axes.at(0));
	auto* xAxis2 = static_cast<Axis*>(axes.at(1));
	auto* yAxis1 = static_cast<Axis*>(axes.at(2));
	auto* yAxis2 = static_cast<Axis*>(axes.at(3));
	QCOMPARE(xAxis1->name(), "x");
	QCOMPARE(xAxis2->name(), "x2");
	QCOMPARE(yAxis1->name(), "y");
	QCOMPARE(yAxis2->name(), "y2");

	auto list = QStringList({// data rect of the plot does not change, so retransforming the
							 // plot is not needed
							 "Project/Worksheet/Plot/x",
							 "Project/Worksheet/Plot/y",
							 "Project/Worksheet/Plot/x2",
							 "Project/Worksheet/Plot/y2",
							 "Project/Worksheet/Plot/curve",
							 "Project/Worksheet/Plot/curve2"});

	CartesianPlotDock dock(nullptr);
	dock.setPlots({plot});
	dock.addYRange();
	QCOMPARE(plot->rangeCount(Dimension::Y), 2);
	dock.addPlotRange();
	QCOMPARE(plot->coordinateSystemCount(), 2);

	c.resetRetransformCount();

	dock.PlotRangeChanged(1, Dimension::Y, 1); // switch for second csystem to y range 2

	// No update of the scales, because the range did not change
	QCOMPARE(c.logsXScaleRetransformed.count(), 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 0);

	c.resetRetransformCount();

	XYCurveDock curveDock(nullptr);
	curveDock.setupGeneral();
	curveDock.setCurves({curve2});
	QCOMPARE(curve2->coordinateSystemIndex(), 0);
	curveDock.plotRangeChanged(1); // set curve range to other
	QCOMPARE(curve2->coordinateSystemIndex(), 1);

	// CSystem1:
	// x 1..100
	// y 1..100
	// CSystem2:
	// x 1..100 // bigger 100 > 30 (same xrange as for CSystem1
	// y 1..30

	QCOMPARE(plot->range(Dimension::X, 0).start(), 1);
	QCOMPARE(plot->range(Dimension::X, 0).end(), 100);
	QCOMPARE(plot->range(Dimension::Y, 0).start(), 1);
	QCOMPARE(plot->range(Dimension::Y, 0).end(), 100);
	QCOMPARE(plot->range(Dimension::Y, 1).start(), 1);
	QCOMPARE(plot->range(Dimension::Y, 1).end(), 30);

	// the y scale of cSystem1 and the y scale of CSystem2 shall be retransformed, because
	// cruve2 switches from y1 to y2
	// xScale shall not be retransformed because it did not change
	QCOMPARE(c.logsXScaleRetransformed.count(), 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 1);
	// QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0); // did not change, because range is already at 1..100
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 1);

	const auto dataRect = plot->dataRect();

	// Check that both lines go from bottom to top
	{
		QCOMPARE(yAxis1->d_func()->lines.count(), 1);
		const auto line = yAxis1->d_func()->lines.at(0);
		QCOMPARE(line.p1().y(), dataRect.bottom());
		QCOMPARE(line.p2().y(), dataRect.top());
	}

	{
		QCOMPARE(yAxis2->d_func()->lines.count(), 1);
		const auto line = yAxis2->d_func()->lines.at(0);
		QCOMPARE(line.p1().y(), dataRect.bottom());
		QCOMPARE(line.p2().y(), dataRect.top());
	}

	// Now change cSystemIndex of second yAxis too
	c.resetRetransformCount();

	{
		AxisDock d(nullptr);
		d.setAxes({yAxis2});
		d.plotRangeChanged(1); // change from cSystem1 to cSystem2
	}

	// the y scale of cSystem1 and the y scale of CSystem2 shall be retransformed, because
	// yAxis2 switches from y1 to y2
	// xScale shall not be retransformed because it did not change
	QCOMPARE(c.logsXScaleRetransformed.count(), 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 0);
	//	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0); // range did not change, so retransformScale gets not called
	//	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 1); // range did not change, so retransformScale gets not called

	{
		QCOMPARE(yAxis1->d_func()->lines.count(), 1);
		const auto line = yAxis1->d_func()->lines.at(0);
		QCOMPARE(line.p1().y(), dataRect.bottom());
		QCOMPARE(line.p2().y(), dataRect.top());
	}

	{
		QCOMPARE(yAxis2->d_func()->lines.count(), 1);
		const auto line = yAxis2->d_func()->lines.at(0);
		QCOMPARE(line.p1().y(), dataRect.bottom());
		QCOMPARE(line.p2().y(), dataRect.top());
	}
}

void RetransformTest::TestChangePlotRangeElement3() {
	// Change the plotrange of one of the elements
	// This time curve1 changes csystem
	// --> yRange1 and yRange2 have to be transformed

	RetransformCallCounter c;
	Project project;

	auto* sheet = new Spreadsheet("Spreadsheet", false);
	sheet->setColumnCount(4);
	sheet->setRowCount(100);

	project.addChild(sheet);

	auto* worksheet = new Worksheet("Worksheet");
	project.addChild(worksheet);

	auto* p = new CartesianPlot("Plot");
	p->setType(CartesianPlot::Type::FourAxes); // Otherwise no axis are created
	worksheet->addChild(p);
	p->setNiceExtend(false);

	auto* curve = new XYCurve("curve");
	p->addChild(curve);
	{
		QVector<int> xData;
		QVector<double> yData;
		for (int i = 1; i < 101; i++) {
			xData.append(i);
			yData.append(i);
		}
		auto* xColumn = sheet->column(0);
		xColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
		xColumn->replaceInteger(0, xData);
		auto* yColumn = sheet->column(1);
		yColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
		yColumn->replaceValues(0, yData);
		curve->setXColumn(xColumn);
		curve->setYColumn(yColumn);
	}

	auto* curve2 = new XYCurve("curve2");
	p->addChild(curve2);
	{
		QVector<int> xData;
		QVector<double> yData;
		for (int i = 1; i < 31; i++) { // different to the above
			xData.append(i);
			yData.append(i);
		}
		auto* xColumn = sheet->column(2);
		xColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
		xColumn->replaceInteger(0, xData);
		auto* yColumn = sheet->column(3);
		yColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
		yColumn->replaceValues(0, yData);
		curve2->setXColumn(xColumn);
		curve2->setYColumn(yColumn);
	}

	// SAVE_PROJECT("TestChangePlotRangeElement2.lml");

	QCOMPARE(p->range(Dimension::X, 0).start(), 1);
	QCOMPARE(p->range(Dimension::X, 0).end(), 100);
	QCOMPARE(p->range(Dimension::Y, 0).start(), 1);
	QCOMPARE(p->range(Dimension::Y, 0).end(), 100);

	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);

	// Spreadsheet "Spreadsheet"
	// Column "1"
	// Column "2"
	// Column "3"
	// Column "4"
	// Worksheet "Worksheet"
	// CartesianPlot "Plot"
	// Axis "x"
	// Axis "x2"
	// Axis "y"
	// Axis "y2"
	// XYCurve "curve"
	// XYCurve "curve2"
	QCOMPARE(children.length(), 13);
	for (const auto& child : children)
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);

	auto plots = project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.length(), 1);
	auto* plot = static_cast<CartesianPlot*>(plots[0]);
	connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);

	// check axis ranges
	auto axes = project.children(AspectType::Axis, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(axes.length(), 4);
	auto* xAxis1 = static_cast<Axis*>(axes.at(0));
	auto* xAxis2 = static_cast<Axis*>(axes.at(1));
	auto* yAxis1 = static_cast<Axis*>(axes.at(2));
	auto* yAxis2 = static_cast<Axis*>(axes.at(3));
	QCOMPARE(xAxis1->name(), "x");
	QCOMPARE(xAxis2->name(), "x2");
	QCOMPARE(yAxis1->name(), "y");
	QCOMPARE(yAxis2->name(), "y2");

	auto list = QStringList({// data rect of the plot does not change, so retransforming the
							 // plot is not needed
							 "Project/Worksheet/Plot/x",
							 "Project/Worksheet/Plot/y",
							 "Project/Worksheet/Plot/x2",
							 "Project/Worksheet/Plot/y2",
							 "Project/Worksheet/Plot/curve",
							 "Project/Worksheet/Plot/curve2"});

	CartesianPlotDock dock(nullptr);
	dock.setPlots({plot});
	dock.addYRange();
	QCOMPARE(plot->rangeCount(Dimension::Y), 2);
	dock.addPlotRange();
	QCOMPARE(plot->coordinateSystemCount(), 2);

	c.resetRetransformCount();

	dock.PlotRangeChanged(1, Dimension::Y, 1); // switch for second csystem to y range 2

	// No update of the scales, because the range did not change
	QCOMPARE(c.logsXScaleRetransformed.count(), 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 0);

	c.resetRetransformCount();

	XYCurveDock curveDock(nullptr);
	curveDock.setupGeneral();
	curveDock.setCurves({curve});
	QCOMPARE(curve->coordinateSystemIndex(), 0);
	curveDock.plotRangeChanged(1); // set curve range to other
	QCOMPARE(curve->coordinateSystemIndex(), 1);

	// CSystem1:
	// x 1..100
	// y 1..30
	// CSystem2:
	// x 1..100
	// y 1..100

	QCOMPARE(plot->range(Dimension::X, 0).start(), 1);
	QCOMPARE(plot->range(Dimension::X, 0).end(), 100);
	QCOMPARE(plot->range(Dimension::Y, 0).start(), 1);
	QCOMPARE(plot->range(Dimension::Y, 0).end(), 30);
	QCOMPARE(plot->range(Dimension::Y, 1).start(), 1);
	QCOMPARE(plot->range(Dimension::Y, 1).end(), 100);

	// the y scale of cSystem1 and the y scale of CSystem2 shall be retransformed, because
	// cruve2 switches from y1 to y2
	// xScale shall not be retransformed because it did not change
	QCOMPARE(c.logsXScaleRetransformed.count(), 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 2);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.at(1).index, 1);

	const auto dataRect = plot->dataRect();

	// Check that both lines go from bottom to top
	{
		QCOMPARE(yAxis1->d_func()->lines.count(), 1);
		const auto line = yAxis1->d_func()->lines.at(0);
		QCOMPARE(line.p1().y(), dataRect.bottom());
		QCOMPARE(line.p2().y(), dataRect.top());
	}

	{
		QCOMPARE(yAxis2->d_func()->lines.count(), 1);
		const auto line = yAxis2->d_func()->lines.at(0);
		QCOMPARE(line.p1().y(), dataRect.bottom());
		QCOMPARE(line.p2().y(), dataRect.top());
	}

	// Now change cSystemIndex of second yAxis too
	c.resetRetransformCount();

	{
		AxisDock d(nullptr);
		d.setAxes({yAxis2});
		d.plotRangeChanged(1); // change from cSystem1 to cSystem2
	}

	// the y scale of cSystem1 and the y scale of CSystem2 shall be retransformed, because
	// yAxis2 switches from y1 to y2
	// xScale shall not be retransformed because it did not change
	QCOMPARE(c.logsXScaleRetransformed.count(), 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 0);
	//	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0); // range did not change, so retransformScale gets not called
	//	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 1); // range did not change, so retransformScale gets not called

	{
		QCOMPARE(yAxis1->d_func()->lines.count(), 1);
		const auto line = yAxis1->d_func()->lines.at(0);
		QCOMPARE(line.p1().y(), dataRect.bottom());
		QCOMPARE(line.p2().y(), dataRect.top());
	}

	{
		QCOMPARE(yAxis2->d_func()->lines.count(), 1);
		const auto line = yAxis2->d_func()->lines.at(0);
		QCOMPARE(line.p1().y(), dataRect.bottom());
		QCOMPARE(line.p2().y(), dataRect.top());
	}
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