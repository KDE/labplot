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
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#include "kdefrontend/MainWin.h"

#include <QAction>

#define COMPARE(actual, expected, message) \
do {\
	if (!QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__)) {\
		qDebug() << message; \
		return;\
	}\
} while (false)

QHash<QString, int> RetransformTest::statistic(bool includeSuppressed) {
	QHash<QString, int> result;
	for (auto& log: logsRetransformed) {
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

int RetransformTest::elementLogCount(bool includeSuppressed) {
	return statistic(includeSuppressed).count();
}

bool RetransformTest::calledExact(int requiredCallCount, bool includeSuppressed) {

	const auto& result = statistic(includeSuppressed);
	QHash<QString, int>::const_iterator i;
	for (i = result.constBegin(); i != result.constEnd(); ++i) {
		if (i.value() != requiredCallCount)  {
			qDebug() << "Expected CallCount: " << requiredCallCount << ", Current: " << i.value() << ". " << i.key();
			return false;
		}
	}
	return true;
}

int RetransformTest::callCount(const QString& path, bool includeSuppressed) {
	const auto& result = statistic(includeSuppressed);
	if (!result.contains(path))
		return 0;

	return result.value(path);
}

int RetransformTest::callCount(const AbstractAspect* aspect, bool includeSuppressed) {
	int count = 0;
	for (auto& suppressed: aspect->readRetransformCalled()) {
		if (suppressed && !includeSuppressed)
			continue;
		count += 1;
	}
	return count;
}

void RetransformTest::resetRetransformCount() {
	logsRetransformed.clear();
	logsXScaleRetransformed.clear();
	logsYScaleRetransformed.clear();
}

void RetransformTest::aspectRetransformed(const AbstractAspect* sender, bool suppressed) {

	logsRetransformed.append({sender, suppressed});
}

void RetransformTest::retransformXScaleCalled(const CartesianPlot* plot, int index) {
	logsXScaleRetransformed.append({plot, index});
}

void RetransformTest::retransformYScaleCalled(const CartesianPlot* plot, int index) {
	logsYScaleRetransformed.append({plot, index});
}


void RetransformTest::aspectAdded(const AbstractAspect* aspect) {
	connect(aspect, &AbstractAspect::retransformCalledSignal, this, &RetransformTest::aspectRetransformed);
}

void RetransformTest::TestLoadProject() {
	resetRetransformCount(); // Must be called before every test
	Project project;

	// Does not work during load.
	//connect(&project, &Project::aspectAdded, this, &RetransformTest::aspectAdded);

	project.load(QFINDTESTDATA(QLatin1String("data/p1.lml")));

	QHash<QString, int> h = {
		{"Project/Worksheet/xy-plot", 1},
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
	};

	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);
	for (auto& child: children) {
		int expectedCallCount = 0;
		const QString& path = child->path();
		if (h.contains(path))
			expectedCallCount = h.value(path);
		COMPARE(callCount(child, false), expectedCallCount, path);
	}
}

void RetransformTest::TestResizeWindows() {
	resetRetransformCount(); // Must be called before every test
	MainWin mainWin;
	mainWin.resize(100, 100);
	mainWin.openProject(QFINDTESTDATA(QLatin1String("data/p1.lml")));

	auto* project = mainWin.project();
	for (const auto& child: project->children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive))
		connect(child, &AbstractAspect::retransformCalledSignal, this, &RetransformTest::aspectRetransformed);

	mainWin.resize(1000, 1000);
	QCOMPARE(elementLogCount(false), 14);
	QVERIFY(calledExact(1, false));  // only called once!
}

void RetransformTest::TestZoomSelectionAutoscale() {
	resetRetransformCount(); // Must be called before every test
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
	QCOMPARE(children.length(), 18);
	for (const auto& child: children)
		connect(child, &AbstractAspect::retransformCalledSignal, this, &RetransformTest::aspectRetransformed);

	for (const auto& plot: project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive)) {
		connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::retransformXScaleCalled, this, &RetransformTest::retransformXScaleCalled);
		connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::retransformYScaleCalled, this, &RetransformTest::retransformYScaleCalled);
	}

	auto* worksheet = project.child<Worksheet>(0);
	QVERIFY(worksheet);

	auto* view = static_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view);

	auto* plot = worksheet->child<CartesianPlot>(0);
	QVERIFY(plot);
	QCOMPARE(plot->name(), QLatin1String("xy-plot"));

	QAction a(nullptr);                                                                                                                                        \
	a.setData(static_cast<int>(CartesianPlot::MouseMode::ZoomXSelection));                                                                                                                         \
	view->cartesianPlotMouseModeChanged(&a);

	QCOMPARE(elementLogCount(false), 0);
	QVERIFY(calledExact(0, false));

	plot->mousePressZoomSelectionMode(QPointF(0.2, -150), -1);
	plot->mouseMoveZoomSelectionMode(QPointF(0.6, 100), -1);
	plot->mouseReleaseZoomSelectionMode(-1);

	// x and y are called only once
	QCOMPARE(logsXScaleRetransformed.count(), 1);
	QCOMPARE(logsXScaleRetransformed.at(0).plot, plot);
	QCOMPARE(logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(logsYScaleRetransformed.count(), 2); // there are two vertical ranges (sin,cos and tan range)
	QCOMPARE(logsYScaleRetransformed.at(0).plot, plot);
	QCOMPARE(logsYScaleRetransformed.at(0).index, 0);
	QCOMPARE(logsYScaleRetransformed.at(1).plot, plot);
	QCOMPARE(logsYScaleRetransformed.at(1).index, 1);

	// TODO: set to 6. legend should not retransform
	// plot it self does not change so retransform is not called on cartesianplotPrivate
	 QStringList list = {
	 "Project/Worksheet/xy-plot/x",
	 "Project/Worksheet/xy-plot/y",
	 "Project/Worksheet/xy-plot/sin",
	 "Project/Worksheet/xy-plot/cos",
	 "Project/Worksheet/xy-plot/tan",
	 "Project/Worksheet/xy-plot/y-axis",
	 "Project/Worksheet/xy-plot/legend",
	 "Project/Worksheet/xy-plot/plotText",
	 "Project/Worksheet/xy-plot/plotImage"};
	QCOMPARE(elementLogCount(false), list.count());
	for (auto& s: list)
		QCOMPARE(callCount(s, false), 1);

	resetRetransformCount();
	plot->navigate(-1, CartesianPlot::NavigationOperation::ScaleAuto);

	QCOMPARE(elementLogCount(false), list.count());
	for (auto& s: list)
		QCOMPARE(callCount(s, false), 1);

	// x and y are called only once
	QCOMPARE(logsXScaleRetransformed.count(), 1);
	QCOMPARE(logsXScaleRetransformed.at(0).plot, plot);
	QCOMPARE(logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(logsYScaleRetransformed.count(), 2); // there are two vertical ranges (sin,cos and tan range)
	QCOMPARE(logsYScaleRetransformed.at(0).plot, plot);
	QCOMPARE(logsYScaleRetransformed.at(0).index, 0);
	QCOMPARE(logsYScaleRetransformed.at(1).plot, plot);
	QCOMPARE(logsYScaleRetransformed.at(1).index, 1);
}

void RetransformTest::TestPadding() {
	resetRetransformCount(); // Must be called before every test
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
	QCOMPARE(children.length(), 18);
	for (const auto& child: children)
		connect(child, &AbstractAspect::retransformCalledSignal, this, &RetransformTest::aspectRetransformed);

	for (const auto& plot: project.children(AspectType::CartesianPlot, AbstractAspect::ChildIndexFlag::Recursive)) {
		connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::retransformXScaleCalled, this, &RetransformTest::retransformXScaleCalled);
		connect(static_cast<CartesianPlot*>(plot), &CartesianPlot::retransformYScaleCalled, this, &RetransformTest::retransformYScaleCalled);
	}

	auto* worksheet = project.child<Worksheet>(0);
	QVERIFY(worksheet);

	auto* view = static_cast<WorksheetView*>(worksheet->view());
	QVERIFY(view);

	auto* plot = worksheet->child<CartesianPlot>(0);
	QVERIFY(plot);
	QCOMPARE(plot->name(), QLatin1String("xy-plot"));

	// TODO: set to 6. legend should not retransform
	// plot it self does not change so retransform is not called on cartesianplotPrivate
	 QStringList list = {
		 "Project/Worksheet/xy-plot", // datarect changed, so plot must also be retransformed
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

	QCOMPARE(elementLogCount(false), list.count());
	for (auto& s: list)
		QCOMPARE(callCount(s, false), 1);

	// x and y are called only once
	QCOMPARE(logsXScaleRetransformed.count(), 1);
	QCOMPARE(logsXScaleRetransformed.at(0).plot, plot);
	QCOMPARE(logsXScaleRetransformed.at(0).index, 0);
	QCOMPARE(logsYScaleRetransformed.count(), 2); // there are two vertical ranges (sin,cos and tan range)
	QCOMPARE(logsYScaleRetransformed.at(0).plot, plot);
	QCOMPARE(logsYScaleRetransformed.at(0).index, 0);
	QCOMPARE(logsYScaleRetransformed.at(1).plot, plot);
	QCOMPARE(logsYScaleRetransformed.at(1).index, 1);

	resetRetransformCount();

	list = QStringList({
		 // data rect of the plot does not change, so retransforming the
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

	QCOMPARE(elementLogCount(false), list.count());
	for (auto& s: list)
		QCOMPARE(callCount(s, false), 1);

	// x and y are already scaled due to the change of padding
	QCOMPARE(logsXScaleRetransformed.count(), 0);
	QCOMPARE(logsYScaleRetransformed.count(), 0);
}

// Test add new curve autoscale
// Test change data

QTEST_MAIN(RetransformTest)
