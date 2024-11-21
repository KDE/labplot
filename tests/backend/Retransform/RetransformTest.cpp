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
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/plots/cartesian/XYFunctionCurve.h"
#include "frontend/dockwidgets/AxisDock.h"
#include "frontend/dockwidgets/CartesianPlotDock.h"
#include "frontend/dockwidgets/XYCurveDock.h"
#include "frontend/worksheet/WorksheetView.h"

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

	QHash<QString, int> h = {{QStringLiteral("Project/Worksheet/xy-plot"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/x"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/y"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/sin"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/cos"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/tan"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/y-axis"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/legend"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/plotImage"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/plotText"), 1},
							 {QStringLiteral("Project/Worksheet/Text Label"), 1},
							 {QStringLiteral("Project/Worksheet/Image"), 1},
							 {QStringLiteral("Project/Worksheet/plot2"), 1},
							 {QStringLiteral("Project/Worksheet/plot2/x"), 1},
							 {QStringLiteral("Project/Worksheet/plot2/y"), 1},
							 {QStringLiteral("Project/Worksheet/plot2/xy-curve"), 1}};

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
	QLocale::setDefault(QLocale::C); // . as decimal separator
	RetransformCallCounter c;
	Project project;

	project.load(QFINDTESTDATA(QLatin1String("data/bars_dis_004.lml")));

	QHash<QString, int> h = {{QStringLiteral("Project/Worksheet - Spreadsheet/Plot - Spreadsheet"), 1},
							 {QStringLiteral("Project/Worksheet - Spreadsheet/Plot - Spreadsheet/x"), 1},
							 {QStringLiteral("Project/Worksheet - Spreadsheet/Plot - Spreadsheet/y"), 1},
							 {QStringLiteral("Project/Worksheet - Spreadsheet/Plot - Spreadsheet/x2"), 1},
							 {QStringLiteral("Project/Worksheet - Spreadsheet/Plot - Spreadsheet/y2"), 1},
							 {QStringLiteral("Project/Worksheet - Spreadsheet/Plot - Spreadsheet/Frequency"), 1},
							 {QStringLiteral("Project/Worksheet - Spreadsheet/Plot - Spreadsheet/Cumulative"), 1},
							 {QStringLiteral("Project/Worksheet - Spreadsheet/Plot - Spreadsheet/legend"), 1}};

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

	QCOMPARE(xAxis->name(), QStringLiteral("x"));
	QCOMPARE(xAxis2->name(), QStringLiteral("x2"));
	QCOMPARE(yAxis1->name(), QStringLiteral("y"));
	QCOMPARE(yAxis2->name(), QStringLiteral("y2"));

	// xAxis2 does not have any labels
	QVector<QString> refString = {QStringLiteral("161.2"),
								  QStringLiteral("166.7"),
								  QStringLiteral("172.2"),
								  QStringLiteral("177.8"),
								  QStringLiteral("183.3"),
								  QStringLiteral("188.8"),
								  QStringLiteral("194.4")};
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
	QHash<QString, int> h = {{QStringLiteral("Project/Worksheet/xy-plot"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/x"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/y"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/sin"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/cos"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/tan"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/y-axis"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/legend"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/plotImage"), 1},
							 {QStringLiteral("Project/Worksheet/xy-plot/plotText"), 1},
							 //{QStringLiteral("Project/Worksheet/Text Label"), 1}, // TODO: turn on when fixed
							 //{QStringLiteral("Project/Worksheet/Image"), 1},  // TODO: turn on when fixed
							 {QStringLiteral("Project/Worksheet/plot2"), 1},
							 {QStringLiteral("Project/Worksheet/plot2/x"), 1},
							 {QStringLiteral("Project/Worksheet/plot2/y"), 1},
							 {QStringLiteral("Project/Worksheet/plot2/xy-curve"), 1}};

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

	QCOMPARE(plot->childCount<XYCurve>(), 3);
	QCOMPARE(plot->child<XYCurve>(0)->name(), QStringLiteral("sin"));
	QCOMPARE(plot->child<XYCurve>(0)->coordinateSystemIndex(), 0);
	QCOMPARE(plot->child<XYCurve>(1)->name(), QStringLiteral("cos"));
	QCOMPARE(plot->child<XYCurve>(1)->coordinateSystemIndex(), 0);
	QCOMPARE(plot->child<XYCurve>(2)->name(), QStringLiteral("tan"));
	QCOMPARE(plot->child<XYCurve>(2)->coordinateSystemIndex(), 1);

	QAction a(nullptr);
	a.setData(static_cast<int>(CartesianPlot::MouseMode::ZoomXSelection));
	view->cartesianPlotMouseModeChanged(&a);

	QCOMPARE(c.elementLogCount(false), 0);
	QVERIFY(c.calledExact(0, false));

	Q_EMIT plot->mousePressZoomSelectionModeSignal(QPointF(0.2, -150));
	Q_EMIT plot->mouseMoveZoomSelectionModeSignal(QPointF(0.6, 100));
	Q_EMIT plot->mouseReleaseZoomSelectionModeSignal();

	// x and y are called only once
	QCOMPARE(c.logsXScaleRetransformed.count(), 2); // 2 plots with each one x axis
	QCOMPARE(c.logsXScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsXScaleRetransformed.at(1).plot, plot2);
	QCOMPARE(c.logsXScaleRetransformed.at(1).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.count(),
			 3); // there are two vertical ranges (sin, cos and tan range) for the first plot and one y axis for the second plot
	QCOMPARE(c.logsYScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.at(1).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(1).index, 1);
	QCOMPARE(c.logsYScaleRetransformed.at(2).plot, plot2);
	QCOMPARE(c.logsYScaleRetransformed.at(2).index, 0);

	// Check that every element is exactly called once
	// plot it self does not change so retransform is not called on cartesianplotPrivate
	QStringList list = {
		QStringLiteral("Project/Worksheet/xy-plot/x"),
		QStringLiteral("Project/Worksheet/xy-plot/y"),
		QStringLiteral("Project/Worksheet/xy-plot/sin"),
		QStringLiteral("Project/Worksheet/xy-plot/cos"),
		QStringLiteral("Project/Worksheet/xy-plot/tan"),
		QStringLiteral("Project/Worksheet/xy-plot/y-axis"),
		// not neccesary to retransform legend, but is difficult to
		// distinguish so let it in, because it does not cost that much performance
		QStringLiteral("Project/Worksheet/xy-plot/legend"),
		QStringLiteral("Project/Worksheet/xy-plot/plotText"),
		QStringLiteral("Project/Worksheet/xy-plot/plotImage"),
		// second plot starting
		QStringLiteral("Project/Worksheet/plot2/x"),
		QStringLiteral("Project/Worksheet/plot2/y"),
		QStringLiteral("Project/Worksheet/plot2/xy-curve"),
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
			 3); // there are two vertical ranges (sin, cos and tan range) for the first plot and one y axis for the second plot
	QCOMPARE(c.logsYScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0); // first y axis of first plot
	QCOMPARE(c.logsYScaleRetransformed.at(1).plot, plot);
	QCOMPARE(c.logsYScaleRetransformed.at(1).index, 1); // second y axis of first plot
	QCOMPARE(c.logsYScaleRetransformed.at(2).plot, plot2);
	QCOMPARE(c.logsYScaleRetransformed.at(2).index, 0); // first y axis of second plot
}

/*!
 * \brief RetransformTest::TestZoomAutoscaleSingleYRange
 * Having two coordinatesystems cSystem1 and cSystem2 with a common x Range
 * cSystem1 has automatic scaling of y Range turned on, cSystem2 not
 * When zoom x Selection is done, the y Range of cSystem1 shall be autoscaled,
 * but not the y Range of cSystem2
 * Nice extends should not apply!
 */
void RetransformTest::TestZoomAutoscaleSingleYRange() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(true); // Important must be on!

	// Create new cSystem2
	Range<double> yRange;
	yRange.setFormat(RangeT::Format::Numeric);
	plot->addYRange(yRange);
	CartesianCoordinateSystem* cSystem2 = new CartesianCoordinateSystem(plot);
	cSystem2->setIndex(Dimension::X, 0);
	cSystem2->setIndex(Dimension::Y, 1);
	plot->addCoordinateSystem(cSystem2);

	// Generate data and
	Spreadsheet* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	project.addChild(sheet);
	sheet->setColumnCount(3);
	sheet->setRowCount(11);
	sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::Double);
	sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Double);
	sheet->column(2)->setColumnMode(AbstractColumn::ColumnMode::Double);

	for (int i = 0; i < sheet->rowCount(); i++) {
		sheet->column(0)->setValueAt(i, i);
		sheet->column(1)->setValueAt(i, i + 1000 + 0.3);
		sheet->column(2)->setValueAt(i, -i + 0.1); // This 0.1 is important!
	}

	auto* curve1 = new XYCurve(QStringLiteral("curve1"));
	plot->addChild(curve1);
	curve1->setCoordinateSystemIndex(0);
	curve1->setXColumn(sheet->column(0));
	curve1->setYColumn(sheet->column(1));
	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 1), RangeT::Format::Numeric);

	auto* curve2 = new XYCurve(QStringLiteral("curve2"));
	plot->addChild(curve2);
	curve2->setCoordinateSystemIndex(1);
	curve2->setXColumn(sheet->column(0));
	curve2->setYColumn(sheet->column(2));
	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 1), RangeT::Format::Numeric);

	CHECK_RANGE(plot, curve1, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, curve1, Dimension::Y, 1000., 1011.); // Nice extend applied
	CHECK_RANGE(plot, curve2, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, curve2, Dimension::Y, -10., 1.);

	plot->enableAutoScale(Dimension::Y, 1, false); // disable autoscale for second y range

	auto r = plot->range(Dimension::Y, 1);
	r.setStart(-9.9);
	r.setEnd(0.1);
	plot->setRange(Dimension::Y, 1, r);

	CHECK_RANGE(plot, curve1, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, curve1, Dimension::Y, 1000., 1011.);
	CHECK_RANGE(plot, curve2, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, curve2, Dimension::Y, -9.9, 0.1);

	QAction a(nullptr);
	a.setData(static_cast<int>(CartesianPlot::MouseMode::ZoomXSelection));
	auto* view = static_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view);
	view->initActions();
	view->cartesianPlotMouseModeChanged(&a);

	view->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToAllX);

	// Zoom selection
	Q_EMIT plot->mousePressZoomSelectionModeSignal(QPointF(2, 0));
	Q_EMIT plot->mouseMoveZoomSelectionModeSignal(QPointF(3, 100));
	Q_EMIT plot->mouseReleaseZoomSelectionModeSignal();

	CHECK_RANGE(plot, curve1, Dimension::X, 2., 3.);
	CHECK_RANGE(plot, curve1, Dimension::Y, 1002.2, 1003.3); // Nice Extend applied
	CHECK_RANGE(plot, curve2, Dimension::X, 2., 3.);
	CHECK_RANGE(plot, curve2, Dimension::Y, -9.9, 0.1); // Not changed, because autoscale is turned off
}

/*!
 * \brief RetransformTest::TestZoomAutoscaleSingleXRange
 * Having two coordinatesystems cSystem1 and cSystem2 with a common y Range
 * cSystem1 has automatic scaling of x Range turned on, cSystem2 not
 * When zoom y Selection is done, the x Range of cSystem1 shall be autoscaled,
 * but not the x Range of cSystem2
 * Nice extends should not apply!
 */
void RetransformTest::TestZoomAutoscaleSingleXRange() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(true); // Important must be on!

	// Create new cSystem2
	Range<double> xRange;
	xRange.setFormat(RangeT::Format::Numeric);
	plot->addXRange(xRange);
	CartesianCoordinateSystem* cSystem2 = new CartesianCoordinateSystem(plot);
	cSystem2->setIndex(Dimension::X, 1);
	cSystem2->setIndex(Dimension::Y, 0);
	plot->addCoordinateSystem(cSystem2);

	// Generate data and
	Spreadsheet* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	project.addChild(sheet);
	sheet->setColumnCount(3);
	sheet->setRowCount(11);
	sheet->column(0)->setColumnMode(AbstractColumn::ColumnMode::Double);
	sheet->column(1)->setColumnMode(AbstractColumn::ColumnMode::Double);
	sheet->column(2)->setColumnMode(AbstractColumn::ColumnMode::Double);

	for (int i = 0; i < sheet->rowCount(); i++) {
		sheet->column(0)->setValueAt(i, i);
		sheet->column(1)->setValueAt(i, i + 1000 + 0.3);
		sheet->column(2)->setValueAt(i, -i + 0.1); // This 0.1 is important!
	}

	auto* curve1 = new XYCurve(QStringLiteral("curve1"));
	plot->addChild(curve1);
	curve1->setCoordinateSystemIndex(0);
	curve1->setXColumn(sheet->column(1));
	curve1->setYColumn(sheet->column(0));
	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::X, 1), RangeT::Format::Numeric);

	auto* curve2 = new XYCurve(QStringLiteral("curve2"));
	plot->addChild(curve2);
	curve2->setCoordinateSystemIndex(1);
	curve2->setXColumn(sheet->column(2));
	curve2->setYColumn(sheet->column(0));
	QCOMPARE(plot->rangeFormat(Dimension::X, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::Y, 0), RangeT::Format::Numeric);
	QCOMPARE(plot->rangeFormat(Dimension::X, 1), RangeT::Format::Numeric);

	CHECK_RANGE(plot, curve1, Dimension::Y, 0., 10.);
	CHECK_RANGE(plot, curve1, Dimension::X, 1000., 1011.); // Nice extend applied
	CHECK_RANGE(plot, curve2, Dimension::Y, 0., 10.);
	CHECK_RANGE(plot, curve2, Dimension::X, -10., 1.);

	plot->enableAutoScale(Dimension::X, 1, false); // disable autoscale for second y range

	auto r = plot->range(Dimension::X, 1);
	r.setStart(-9.9);
	r.setEnd(0.1);
	plot->setRange(Dimension::X, 1, r);

	CHECK_RANGE(plot, curve1, Dimension::Y, 0., 10.);
	CHECK_RANGE(plot, curve1, Dimension::X, 1000., 1011.);
	CHECK_RANGE(plot, curve2, Dimension::Y, 0., 10.);
	CHECK_RANGE(plot, curve2, Dimension::X, -9.9, 0.1);

	QAction a(nullptr);
	a.setData(static_cast<int>(CartesianPlot::MouseMode::ZoomYSelection));
	auto* view = static_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view);
	view->initActions();
	view->cartesianPlotMouseModeChanged(&a);

	view->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToAllY);

	// Zoom selection
	Q_EMIT plot->mousePressZoomSelectionModeSignal(QPointF(0, 2));
	Q_EMIT plot->mouseMoveZoomSelectionModeSignal(QPointF(100, 3));
	Q_EMIT plot->mouseReleaseZoomSelectionModeSignal();

	CHECK_RANGE(plot, curve1, Dimension::Y, 2., 3.);
	CHECK_RANGE(plot, curve1, Dimension::X, 1002.2, 1003.3); // Nice Extend applied
	CHECK_RANGE(plot, curve2, Dimension::Y, 2., 3.);
	CHECK_RANGE(plot, curve2, Dimension::X, -9.9, 0.1); // Not changed, because autoscale is turned off
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
	QStringList list = {QStringLiteral("Project/Worksheet/xy-plot"), // datarect changed, so plot must also be retransformed
						QStringLiteral("Project/Worksheet/xy-plot/x"),
						QStringLiteral("Project/Worksheet/xy-plot/y"),
						QStringLiteral("Project/Worksheet/xy-plot/sin"),
						QStringLiteral("Project/Worksheet/xy-plot/cos"),
						QStringLiteral("Project/Worksheet/xy-plot/tan"),
						QStringLiteral("Project/Worksheet/xy-plot/y-axis"),
						QStringLiteral("Project/Worksheet/xy-plot/legend"),
						QStringLiteral("Project/Worksheet/xy-plot/plotText"),
						QStringLiteral("Project/Worksheet/xy-plot/plotImage")};
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
						QStringLiteral("Project/Worksheet/xy-plot/x"),
						QStringLiteral("Project/Worksheet/xy-plot/y"),
						QStringLiteral("Project/Worksheet/xy-plot/sin"),
						QStringLiteral("Project/Worksheet/xy-plot/cos"),
						QStringLiteral("Project/Worksheet/xy-plot/tan"),
						QStringLiteral("Project/Worksheet/xy-plot/y-axis"),
						QStringLiteral("Project/Worksheet/xy-plot/legend"),
						QStringLiteral("Project/Worksheet/xy-plot/plotText"),
						QStringLiteral("Project/Worksheet/xy-plot/plotImage")});
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
	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* ws2 = new Worksheet(QStringLiteral("Worksheet2"));
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
	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	RetransformCallCounter c;

	p->addChild(new XYEquationCurve(QLatin1String("curve")));

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
	data.expression1 = QStringLiteral("x");
	data.expression2 = QString();
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.type = XYEquationCurve::EquationType::Cartesian;
	equationCurve->setEquationData(data);

	auto list =
		QStringList({QStringLiteral("Project/Worksheet/plot/x"), QStringLiteral("Project/Worksheet/plot/y"), QStringLiteral("Project/Worksheet/plot/f(x)")});
	QCOMPARE(c.elementLogCount(false), list.count());
	QCOMPARE(c.callCount(list.at(0)), 1);
	QCOMPARE(c.callCount(list.at(1)), 1);
	QCOMPARE(c.callCount(list.at(2)), 0);

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

	for (const auto& plot : project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive))
		connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);

	auto barplots = project.children(AspectType::BarPlot, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(barplots.length(), 1);
	auto barplot = static_cast<BarPlot*>(barplots.at(0));
	QCOMPARE(barplot->name(), QStringLiteral("Bar Plot"));

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

	auto* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
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

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet - Spreadsheet"));
	project.addChild(worksheet);

	auto* p = new CartesianPlot(QStringLiteral("Plot - Spreadsheet"));
	p->setType(CartesianPlot::Type::FourAxes); // Otherwise no axis are created
	worksheet->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("curve"));
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
	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	Spreadsheet* spreadsheet = new Spreadsheet(QStringLiteral("test"), false);
	spreadsheet->setColumnCount(2);
	spreadsheet->setRowCount(3);

	auto* xCol = spreadsheet->column(0);
	xCol->replaceValues(0, QVector<double>({1, 2, 3}));

	auto* yCol = spreadsheet->column(1);
	yCol->replaceValues(0, QVector<double>({2, 3, 4}));

	QCOMPARE(spreadsheet->rowCount(), 3);
	QCOMPARE(spreadsheet->columnCount(), 2);

	project.addChild(spreadsheet);
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("xy-curve"));
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
	auto properties = filter.properties();
	properties.headerEnabled = true;
	properties.headerLine = 1;
	filter.setProperties(properties);
	filter.readDataFromFile(file.fileName(), spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet->rowCount(), 3);
	QCOMPARE(spreadsheet->columnCount(), 2);

	xCol = spreadsheet->column(0);
	QCOMPARE(xCol->name(), QStringLiteral("1"));
	QCOMPARE(xCol->valueAt(0), 10);
	QCOMPARE(xCol->valueAt(1), 20);
	QCOMPARE(xCol->valueAt(2), 30);

	yCol = spreadsheet->column(1);
	QCOMPARE(yCol->name(), QStringLiteral("2"));
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
	QCOMPARE(c.logsYScaleRetransformed.count(), 1); // one plot with 1 y-Axis
	QCOMPARE(c.logsYScaleRetransformed.at(0).plot, p);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 0);

	auto list = QStringList(
		{QStringLiteral("Project/Worksheet/plot/x"), QStringLiteral("Project/Worksheet/plot/y"), QStringLiteral("Project/Worksheet/plot/xy-curve")});
	QCOMPARE(c.elementLogCount(false), list.count());
	for (auto& s : list) {
		qDebug() << s;
		QCOMPARE(c.callCount(s), 1);
	}
}

// Same as AsciiFilterTest::plotUpdateAfterImportWithColumnRemove() but with retransform check
void RetransformTest::TestImportCSVInvalidateCurve() {
	Project project; // need a project object since the column restore logic is in project

	// create the spreadsheet with the source data
	auto* spreadsheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(spreadsheet);
	spreadsheet->setColumnCount(2);
	spreadsheet->setRowCount(3);

	auto* col = spreadsheet->column(0);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("1"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	col = spreadsheet->column(1);
	col->setColumnMode(AbstractColumn::ColumnMode::Double);
	col->setName(QStringLiteral("2"));
	col->setValueAt(0, 10.);
	col->setValueAt(1, 20.);
	col->setValueAt(2, 30.);

	// create a xy-curve with the both columns in the source spreadsheet and check the ranges
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	project.addChild(p);
	auto* curve = new XYCurve(QStringLiteral("curve"));
	p->addChild(curve);
	curve->setXColumn(spreadsheet->column(0)); // use "1" for x
	curve->setYColumn(spreadsheet->column(1)); // use "2" for y

	auto rangeX = p->range(Dimension::X);
	QCOMPARE(rangeX.start(), 10);
	QCOMPARE(rangeX.end(), 30);

	auto rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeY.start(), 10);
	QCOMPARE(rangeY.end(), 30);

	const auto& children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);
	RetransformCallCounter c;
	// CartesianPlot "plot"
	// XYCurve "curve"
	// Spreadsheet "test"
	// Column "1"
	// Column "2"
	QCOMPARE(children.length(), 5);
	for (const auto& child : children) {
		qDebug() << child->name();
		connect(child, &AbstractAspect::retransformCalledSignal, &c, &RetransformCallCounter::aspectRetransformed);
	}

	const auto& plots = project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive);
	for (const auto& plot : plots)
		connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::scaleRetransformed, &c, &RetransformCallCounter::retransformScaleCalled);

	// import the data into the source spreadsheet, the columns are renamed to "c1" and "c2"
	AsciiFilter filter;

	QStringList fileContent = {
		QStringLiteral("c1;c2"),
		QStringLiteral("1;1"),
		QStringLiteral("2;2"),
		QStringLiteral("3;3"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	filter.readDataFromFile(savePath, spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// the assignment to the data columns got lost since the columns were renamed
	QCOMPARE(curve->xColumn(), nullptr);
	QCOMPARE(curve->yColumn(), nullptr);

	// the range of the plot didn't change, no retransform
	QCOMPARE(c.logsXScaleRetransformed.count(), 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 0);

	// the curve that lost the column assignemnt should be retransformed
	QCOMPARE(c.elementLogCount(false), 1);
	QCOMPARE(c.callCount(QStringLiteral("Project/plot/curve")), 1);
}

void RetransformTest::TestSetScale() {
	RetransformCallCounter c;
	Project project;

	auto* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
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

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* p = new CartesianPlot(QStringLiteral("Plot"));
	p->setType(CartesianPlot::Type::FourAxes); // Otherwise no axis are created
	worksheet->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("curve"));
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
							 QStringLiteral("Project/Worksheet/Plot/x"),
							 QStringLiteral("Project/Worksheet/Plot/x2"),
							 QStringLiteral("Project/Worksheet/Plot/y"),
							 QStringLiteral("Project/Worksheet/Plot/y2"),
							 QStringLiteral("Project/Worksheet/Plot/curve")});

	plot->setRangeScale(Dimension::X, 0, RangeT::Scale::Log10);

	QCOMPARE(c.elementLogCount(false), list.count());
	for (auto& s : list)
		QCOMPARE(c.callCount(s), 1);

	// x and y are called only once
	QCOMPARE(c.logsXScaleRetransformed.count(), 1);
	QCOMPARE(c.logsXScaleRetransformed.at(0).plot, plot);
	QCOMPARE(c.logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(c.logsYScaleRetransformed.count(), 0);
}

void RetransformTest::TestChangePlotRange() {
	RetransformCallCounter c;
	Project project;

	auto* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
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

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* p = new CartesianPlot(QStringLiteral("Plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	worksheet->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("curve"));
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
							 QStringLiteral("Project/Worksheet/Plot/x"),
							 QStringLiteral("Project/Worksheet/Plot/y"),
							 QStringLiteral("Project/Worksheet/Plot/curve")});

	CartesianPlotDock dock(nullptr);
	dock.setPlots({plot});
	dock.addXRange();

	QCOMPARE(plot->rangeCount(Dimension::X), 2);

	dock.autoScaleChanged(Dimension::X, 1, false);
	QCOMPARE(plot->autoScale(Dimension::X, 1), false);

	dock.minChanged(Dimension::X, 1, 10);
	dock.maxChanged(Dimension::X, 1, 20);

	// check axis ranges
	auto axes = project.children(AspectType::Axis, AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(axes.length(), 2);
	auto* xAxis = static_cast<Axis*>(axes.at(0));
	auto* yAxis = static_cast<Axis*>(axes.at(1));

	QCOMPARE(xAxis->name(), QStringLiteral("x"));
	QCOMPARE(yAxis->name(), QStringLiteral("y"));

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

	auto* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
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

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* p = new CartesianPlot(QStringLiteral("Plot"));
	p->setType(CartesianPlot::Type::FourAxes); // Otherwise no axis are created
	worksheet->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("curve"));
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
	QCOMPARE(xAxis1->name(), QStringLiteral("x"));
	QCOMPARE(xAxis2->name(), QStringLiteral("x2"));
	QCOMPARE(yAxis1->name(), QStringLiteral("y"));
	QCOMPARE(yAxis2->name(), QStringLiteral("y2"));

	auto list = QStringList({// data rect of the plot does not change, so retransforming the
							 // plot is not needed
							 QStringLiteral("Project/Worksheet/Plot/x"),
							 QStringLiteral("Project/Worksheet/Plot/y"),
							 QStringLiteral("Project/Worksheet/Plot/x2"),
							 QStringLiteral("Project/Worksheet/Plot/y2"),
							 QStringLiteral("Project/Worksheet/Plot/curve")});

	CartesianPlotDock dock(nullptr);
	dock.setPlots({plot});
	dock.addYRange();
	QCOMPARE(plot->rangeCount(Dimension::Y), 2);
	dock.addPlotRange();
	QCOMPARE(plot->coordinateSystemCount(), 2);

	dock.autoScaleChanged(Dimension::Y, 1, false);
	QCOMPARE(plot->autoScale(Dimension::Y, 1), false);
	dock.minChanged(Dimension::Y, 1, 10);
	dock.maxChanged(Dimension::Y, 1, 20);

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

	auto* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	sheet->setColumnCount(4);
	sheet->setRowCount(100);

	project.addChild(sheet);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* p = new CartesianPlot(QStringLiteral("Plot"));
	p->setType(CartesianPlot::Type::FourAxes); // Otherwise no axis are created
	worksheet->addChild(p);
	p->setNiceExtend(false);

	auto* curve = new XYCurve(QStringLiteral("curve"));
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

	auto* curve2 = new XYCurve(QStringLiteral("curve2"));
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
	QCOMPARE(xAxis1->name(), QStringLiteral("x"));
	QCOMPARE(xAxis2->name(), QStringLiteral("x2"));
	QCOMPARE(yAxis1->name(), QStringLiteral("y"));
	QCOMPARE(yAxis2->name(), QStringLiteral("y2"));

	auto list = QStringList({// data rect of the plot does not change, so retransforming the
							 // plot is not needed
							 QStringLiteral("Project/Worksheet/Plot/x"),
							 QStringLiteral("Project/Worksheet/Plot/y"),
							 QStringLiteral("Project/Worksheet/Plot/x2"),
							 QStringLiteral("Project/Worksheet/Plot/y2"),
							 QStringLiteral("Project/Worksheet/Plot/curve"),
							 QStringLiteral("Project/Worksheet/Plot/curve2")});

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
	QCOMPARE(c.logsYScaleRetransformed.count(), 1);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 1); // The respective coordinate system must be rescaled

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

	auto* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	sheet->setColumnCount(4);
	sheet->setRowCount(100);

	project.addChild(sheet);

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* p = new CartesianPlot(QStringLiteral("Plot"));
	p->setType(CartesianPlot::Type::FourAxes); // Otherwise no axis are created
	worksheet->addChild(p);
	p->setNiceExtend(false);

	auto* curve = new XYCurve(QStringLiteral("curve"));
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

	auto* curve2 = new XYCurve(QStringLiteral("curve2"));
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
	QCOMPARE(xAxis1->name(), QStringLiteral("x"));
	QCOMPARE(xAxis2->name(), QStringLiteral("x2"));
	QCOMPARE(yAxis1->name(), QStringLiteral("y"));
	QCOMPARE(yAxis2->name(), QStringLiteral("y2"));

	auto list = QStringList({// data rect of the plot does not change, so retransforming the
							 // plot is not needed
							 QStringLiteral("Project/Worksheet/Plot/x"),
							 QStringLiteral("Project/Worksheet/Plot/y"),
							 QStringLiteral("Project/Worksheet/Plot/x2"),
							 QStringLiteral("Project/Worksheet/Plot/y2"),
							 QStringLiteral("Project/Worksheet/Plot/curve"),
							 QStringLiteral("Project/Worksheet/Plot/curve2")});

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
	QCOMPARE(c.logsYScaleRetransformed.count(), 1);
	QCOMPARE(c.logsYScaleRetransformed.at(0).index, 1); // The respective coordinate system must be rescaled

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
// ##############################################################################
// ####### Tests checking the retransform behavior on plot shape changes  #######
// ##############################################################################
/*!
 * recalculation of plots without changing the min and max values of the data ranges.
 */
void RetransformTest::testPlotRecalcNoRetransform() {
	// prepare the data
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(100);
	auto* column = sheet.column(0);
	column->setValueAt(0, 2.);
	column->setValueAt(1, 4.);
	column->setValueAt(2, 6.);
	QVector<const AbstractColumn*> dataColumns;
	dataColumns << column;

	// prepare the worksheet + plots
	RetransformCallCounter c;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	ws->addChild(p);
	c.aspectAdded(p);
	const auto& axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	c.aspectAdded(axes.at(0));
	c.aspectAdded(axes.at(1));

	auto* barPlot = new BarPlot(QStringLiteral("barPlot"));
	barPlot->setDataColumns(dataColumns);
	p->addChild(barPlot);
	c.aspectAdded(barPlot);

	auto* boxPlot = new BoxPlot(QStringLiteral("boxPlot"));
	boxPlot->setDataColumns(dataColumns);
	p->addChild(boxPlot);
	c.aspectAdded(boxPlot);

	auto* histPlot = new Histogram(QStringLiteral("histPlot"));
	histPlot->setDataColumn(column);
	p->addChild(histPlot);
	c.aspectAdded(histPlot);

	// call recalc() in the created plots which is called at runtime when modifying the data
	// or any plot properties affecting the shape of the plot.
	// since the data was not changed and no properties were changed affecting plot ranges
	// like the orientation of a box plot changing min and max values for x and y, etc.,
	// there shouldn't be any retransform calls in the parent plot area
	c.resetRetransformCount();
	barPlot->recalc();
	{
		QCOMPARE(c.elementLogCount(false), 1); // only barplot
		const auto stat = c.statistic(false);
		QCOMPARE(stat.contains(barPlot->path()), true);
		QVERIFY(c.calledExact(1, false));
		QCOMPARE(c.logsXScaleRetransformed.count(), 0);
		QCOMPARE(c.logsYScaleRetransformed.count(), 0);
	}

	c.resetRetransformCount();

	boxPlot->recalc();
	{
		QCOMPARE(c.elementLogCount(false), 1); // only boxPlot
		const auto stat = c.statistic(false);
		QCOMPARE(stat.contains(boxPlot->path()), true);
		QVERIFY(c.calledExact(1, false));
		QCOMPARE(c.logsXScaleRetransformed.count(), 0);
		QCOMPARE(c.logsYScaleRetransformed.count(), 0);
	}

	c.resetRetransformCount();

	histPlot->recalc();

	{
		QCOMPARE(c.elementLogCount(false), 1); // only histPlot
		const auto stat = c.statistic(false);
		QCOMPARE(stat.contains(histPlot->path()), true);
		QVERIFY(c.calledExact(1, false));
		QCOMPARE(c.logsXScaleRetransformed.count(), 0);
		QCOMPARE(c.logsYScaleRetransformed.count(), 0);
	}
}

/*!
 * recalculation of plots with changing the min and max values of the data ranges.
 */
void RetransformTest::testPlotRecalcRetransform() {
	// prepare the data
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(100);
	auto* column = sheet.column(0);
	column->setValueAt(0, 2.);
	column->setValueAt(1, 4.);
	column->setValueAt(2, 6.);
	QVector<const AbstractColumn*> dataColumns;
	dataColumns << column;

	RetransformCallCounter c;

	// prepare the worksheet + plots
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	ws->addChild(p);
	c.aspectAdded(p);
	const auto& axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	c.aspectAdded(axes.at(0));
	c.aspectAdded(axes.at(1));

	auto* barPlot = new BarPlot(QStringLiteral("barPlot"));
	barPlot->setDataColumns(dataColumns);
	p->addChild(barPlot);
	c.aspectAdded(barPlot);

	auto* boxPlot = new BoxPlot(QStringLiteral("boxPlot"));
	boxPlot->setDataColumns(dataColumns);
	p->addChild(boxPlot);
	c.aspectAdded(boxPlot);

	auto* histPlot = new Histogram(QStringLiteral("histPlot"));
	histPlot->setDataColumn(column);
	p->addChild(histPlot);
	c.aspectAdded(histPlot);

	c.resetRetransformCount();

	// modify one of the plots so its min and max values are changed.
	// this should trigger the recalculation of the data ranges in the parent plot area
	// and a retransform call for all its children
	QCOMPARE(histPlot->orientation(), Histogram::Orientation::Vertical);
	histPlot->setOrientation(Histogram::Orientation::Horizontal);

	{
		const auto stat = c.statistic(false);
		QCOMPARE(stat.count(), 5); // barPlot, boxPlot, histPlot, xAxis and yAxis are inside
		QCOMPARE(stat.contains(barPlot->path()), true);
		QCOMPARE(stat.contains(boxPlot->path()), true);
		QCOMPARE(stat.contains(histPlot->path()), true);
		QCOMPARE(stat.contains(axes.at(0)->path()), true);
		QCOMPARE(stat.contains(axes.at(1)->path()), true);
		QVERIFY(c.calledExact(1, false));
		QCOMPARE(c.logsXScaleRetransformed.count(), 1);
		QCOMPARE(c.logsYScaleRetransformed.count(), 0); // y range did not change, because boxplot and barplot  are still vertical
	}

	{
		c.resetRetransformCount();
		QCOMPARE(barPlot->orientation(), BarPlot::Orientation::Vertical);
		barPlot->setOrientation(BarPlot::Orientation::Horizontal);
		const auto stat = c.statistic(false);
		QCOMPARE(stat.count(), 5); // barPlot, boxPlot, histPlot, xAxis and yAxis are inside
		QCOMPARE(stat.contains(barPlot->path()), true);
		QCOMPARE(stat.contains(boxPlot->path()), true);
		QCOMPARE(stat.contains(histPlot->path()), true);
		QCOMPARE(stat.contains(axes.at(0)->path()), true);
		QCOMPARE(stat.contains(axes.at(1)->path()), true);
		QVERIFY(c.calledExact(1, false));
		QCOMPARE(c.logsXScaleRetransformed.count(), 1);
		QCOMPARE(c.logsYScaleRetransformed.count(), 0); // y range did not change, because boxplot  is still vertical
	}

	{
		c.resetRetransformCount();
		QCOMPARE(boxPlot->orientation(), BoxPlot::Orientation::Vertical);
		boxPlot->setOrientation(BoxPlot::Orientation::Horizontal);

		const auto stat = c.statistic(false);
		QCOMPARE(stat.count(), 5); // barPlot, boxPlot, histPlot, xAxis and yAxis are inside
		QCOMPARE(stat.contains(barPlot->path()), true);
		QCOMPARE(stat.contains(boxPlot->path()), true);
		QCOMPARE(stat.contains(histPlot->path()), true);
		QCOMPARE(stat.contains(axes.at(0)->path()), true);
		QCOMPARE(stat.contains(axes.at(1)->path()), true);
		QVERIFY(c.calledExact(1, false));
		QCOMPARE(c.logsXScaleRetransformed.count(), 1);
		QCOMPARE(c.logsYScaleRetransformed.count(), 1); // y range changes
	}
}

void RetransformTest::xyFunctionCurve() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	RetransformCallCounter c;

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);
	c.aspectAdded(p);
	const auto& axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	c.aspectAdded(axes.at(0));
	c.aspectAdded(axes.at(1));

	p->addChild(new XYEquationCurve(QLatin1String("eq")));

	auto equationCurves = p->children(AspectType::XYEquationCurve);
	QCOMPARE(equationCurves.count(), 1);
	auto* equationCurve = static_cast<XYEquationCurve*>(equationCurves.at(0));
	XYEquationCurve::EquationData data;
	data.count = 100;
	data.expression1 = QStringLiteral("x");
	data.expression2 = QString();
	data.min = QStringLiteral("1");
	data.max = QStringLiteral("100");
	data.type = XYEquationCurve::EquationType::Cartesian;
	equationCurve->setEquationData(data);

	c.aspectAdded(equationCurve);

	QCOMPARE(equationCurve->xColumn()->rowCount(), data.count);

	p->addChild(new XYFunctionCurve(QLatin1String("eq2")));
	auto functionCurves = p->children(AspectType::XYFunctionCurve);
	QCOMPARE(functionCurves.count(), 1);
	auto* functionCurve = static_cast<XYFunctionCurve*>(functionCurves.at(0));

	c.aspectAdded(functionCurve);

	c.resetRetransformCount();

	functionCurve->setFunction(QStringLiteral("2*x"), {QStringLiteral("x")}, {equationCurve});

	{
		const auto stat = c.statistic(false);
		QCOMPARE(stat.count(), 4); // equationCurve, functionCurve, xAxis and yAxis are inside
		QCOMPARE(stat.contains(equationCurve->path()), true);
		QCOMPARE(stat.contains(functionCurve->path()), true);
		QCOMPARE(stat.contains(axes.at(0)->path()), true);
		QCOMPARE(stat.contains(axes.at(1)->path()), true);
		QVERIFY(c.calledExact(1, false));
		QCOMPARE(c.logsXScaleRetransformed.count(), 0); // only the y range changed
		QCOMPARE(c.logsYScaleRetransformed.count(), 1);
	}

	{
		const auto* xColumn = functionCurve->xColumn();
		const auto* yColumn = functionCurve->yColumn();
		QCOMPARE(xColumn->rowCount(), data.count);
		QCOMPARE(yColumn->rowCount(), data.count);
		for (int i = 0; i < xColumn->rowCount(); i++) {
			VALUES_EQUAL(xColumn->valueAt(i), i + 1.);
			VALUES_EQUAL(yColumn->valueAt(i), (i + 1.) * 2.0);
		}
	}

	c.resetRetransformCount();

	data.expression1 = QStringLiteral("2*x");
	data.min = QStringLiteral("10");
	data.max = QStringLiteral("1000");
	equationCurve->setEquationData(data);

	{
		const auto stat = c.statistic(false);
		QCOMPARE(stat.count(), 4); // equationCurve, functionCurve, xAxis and yAxis are inside
		QCOMPARE(stat.contains(equationCurve->path()), true);
		QCOMPARE(stat.contains(functionCurve->path()), true);
		QCOMPARE(stat.contains(axes.at(0)->path()), true);
		QCOMPARE(stat.contains(axes.at(1)->path()), true);
		// QVERIFY(c.calledExact(1, false)); // TODO: how to verify that retransform gets called only once?
		QCOMPARE(c.logsXScaleRetransformed.count(), 1);
		QVERIFY(c.logsYScaleRetransformed.count() >= 1);
		// QCOMPARE(c.logsYScaleRetransformed.count(), 1);
	}
}

// ############################################################################################
// ############################################################################################
// ############################################################################################

/*!
 * \brief RetransformCallCounter::statistic
 * Returns a statistic how often retransform was called for a specific element
 * They key is the path of the element and the value is the value how often
 * retransform was called
 * \param includeSuppressed. If true all retransforms even the suppressed once will
 * be counted. If false only the retransforms which are really executed are counted
 * \return
 */
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

/*!
 * \brief RetransformCallCounter::elementLogCount
 * Counts the number of different elements which got at least one retransform
 * \param includeSuppressed
 * \return
 */
int RetransformCallCounter::elementLogCount(bool includeSuppressed) {
	return statistic(includeSuppressed).count();
}

/*!
 * \brief RetransformCallCounter::calledExact
 * Checks if all elements are retransformed \p requiredCallCount times
 * \param requiredCallCount
 * \param includeSuppressed
 * \return True if all elements are retransformed \p requiredCallCount times, else false
 */
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

/*!
 * \brief RetransformCallCounter::callCount
 * Returns the call count of a specific element defined by \p path
 * \param path The path of the element element->path()
 * \return
 */
int RetransformCallCounter::callCount(const QString& path) {
	const auto& result = statistic(false);
	if (!result.contains(path))
		return 0;

	return result.value(path);
}

/*!
 * \brief RetransformCallCounter::callCount
 * Returns the number of retransform called for a specific object. This counter contains
 * all retransforms from the beginning when the object was created and not yet connected
 * to the RetransformCallCounter object. This is usefull when checking the retransform
 * counts during loading of a project or during creation of an aspect
 * \param aspect
 * \return
 */
int RetransformCallCounter::callCount(const AbstractAspect* aspect) {
	return aspect->retransformCalled();
}

/*!
 * \brief RetransformCallCounter::resetRetransformCount
 * Reset all counters
 */
void RetransformCallCounter::resetRetransformCount() {
	logsRetransformed.clear();
	logsXScaleRetransformed.clear();
	logsYScaleRetransformed.clear();
}

/*!
 * \brief RetransformCallCounter::aspectRetransformed
 * Slot called whenever an aspects retransform was called after RetransformCallCounter::aspectAdded()
 * was called on the object.
 * \param sender
 * \param suppressed
 */
void RetransformCallCounter::aspectRetransformed(const AbstractAspect* sender, bool suppressed) {
	logsRetransformed.append({sender, suppressed});
}

/*!
 * \brief RetransformCallCounter::retransformScaleCalled
 * Slot called whenever an aspects retransformScale was called after RetransformCallCounter::aspectAdded()
 * was called on the object.
 * \param sender
 * \param suppressed
 */
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

/*!
 * \brief RetransformCallCounter::aspectAdded
 * Connect RetransformCallCounter to the aspects signals to count the retransform calls
 * \param aspect
 */
void RetransformCallCounter::aspectAdded(const AbstractAspect* aspect) {
	connect(aspect, &AbstractAspect::retransformCalledSignal, this, &RetransformCallCounter::aspectRetransformed);
	auto* plot = dynamic_cast<const CartesianPlot*>(aspect);
	if (plot)
		connect(plot, &CartesianPlot::scaleRetransformed, this, &RetransformCallCounter::retransformScaleCalled);
}

void RetransformTest::removeReaddxColum() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	sheet->setColumnCount(2);
	sheet->setRowCount(11);
	project.addChild(sheet);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYEquationCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.count = 11;
	data.expression1 = QStringLiteral("x");
	curve->setEquationData(data);
	curve->recalculate();

	auto* curve2 = new XYCurve(QStringLiteral("curve2"));
	p->addChild(curve2);

	auto* xColumn = sheet->column(0);
	auto* yColumn = sheet->column(1);
	{
		QVector<int> xData;
		QVector<double> yData;
		for (int i = 1; i < 11; i++) { // different to the above
			xData.append(i);
			yData.append(pow(i, 2));
		}

		xColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
		xColumn->replaceInteger(0, xData);
		yColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
		yColumn->replaceValues(0, yData);
		curve2->setXColumn(xColumn);
		curve2->setYColumn(yColumn);
	}

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 100.);

	sheet->removeChild(xColumn);

	// Curve 2 got invalid
	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 10.);

	const auto oldNameXColumn = xColumn->name();
	xColumn->setName(QStringLiteral("NewName"));

	sheet->addChild(xColumn);

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 10.);

	xColumn->setName(oldNameXColumn);

	QCOMPARE(curve2->xColumn(), xColumn);
	QCOMPARE(curve2->yColumn(), yColumn);

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 100.);
}

void RetransformTest::removeReaddyColum() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* sheet = new Spreadsheet(QStringLiteral("Spreadsheet"), false);
	sheet->setColumnCount(2);
	sheet->setRowCount(11);
	project.addChild(sheet);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYEquationCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.count = 11;
	data.expression1 = QStringLiteral("x");
	curve->setEquationData(data);
	curve->recalculate();

	auto* curve2 = new XYCurve(QStringLiteral("curve2"));
	p->addChild(curve2);

	auto* xColumn = sheet->column(0);
	auto* yColumn = sheet->column(1);
	{
		QVector<int> xData;
		QVector<double> yData;
		for (int i = 1; i < 11; i++) { // different to the above
			xData.append(i);
			yData.append(pow(i, 2));
		}

		xColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
		xColumn->replaceInteger(0, xData);
		yColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
		yColumn->replaceValues(0, yData);
		curve2->setXColumn(xColumn);
		curve2->setYColumn(yColumn);
	}

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 100.);

	sheet->removeChild(yColumn);

	// Curve 2 got invalid
	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 10.);

	const auto oldNameYColumn = yColumn->name();
	yColumn->setName(QStringLiteral("NewName"));

	sheet->addChild(yColumn);

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 10.);

	yColumn->setName(oldNameYColumn);

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 100.);
}

// Test change data

QTEST_MAIN(RetransformTest)
