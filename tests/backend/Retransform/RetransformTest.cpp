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
#include "commonfrontend/worksheet/WorksheetView.h"
//#include "backend/lib/trace.h"
#include "kdefrontend/MainWin.h"

#include <QAction>

//#define GET_CURVE_PRIVATE(plot, child_index, column_name, curve_variable_name)                                                                                 \
//	auto* curve_variable_name = plot->child<XYCurve>(child_index);                                                                                             \
//	QVERIFY(curve_variable_name != nullptr);                                                                                                                   \
//	QCOMPARE(curve_variable_name->name(), QLatin1String(column_name));                                                                                         \
//	QCOMPARE(curve_variable_name->type(), AspectType::XYCurve);                                                                                                \
//	auto* curve_variable_name##Private = curve_variable_name->d_func();                                                                                        \
//	Q_UNUSED(curve_variable_name##Private)

//#define LOAD_PROJECT
//	Project project;
//	connect(this, &Project::aspectAdded, this, &Project::aspectAddedSlot);
//#define LOAD_PROJECT                                                                                                                                           \
//	Project project;                                                                                                                                           \
//	project.load(QFINDTESTDATA(QLatin1String("data/TestUpdateLines.lml")));                                                                                    \
//	auto* spreadsheet = project.child<AbstractAspect>(0);                                                                                                      \
//	QVERIFY(spreadsheet != nullptr);                                                                                                                           \
//	QCOMPARE(spreadsheet->name(), QLatin1String("lastValueInvalid"));                                                                                          \
//	QCOMPARE(spreadsheet->type(), AspectType::Spreadsheet);                                                                                                    \
//                                                                                                                                                               \
//	auto* worksheet = project.child<AbstractAspect>(1);                                                                                                        \
//	QVERIFY(worksheet != nullptr);                                                                                                                             \
//	QCOMPARE(worksheet->name(), QLatin1String("Worksheet"));                                                                                                   \
//	QCOMPARE(worksheet->type(), AspectType::Worksheet);                                                                                                        \
//                                                                                                                                                               \
//	auto* plot = worksheet->child<CartesianPlot>(0);                                                                                                           \
//	QVERIFY(plot != nullptr);                                                                                                                                  \
//	QCOMPARE(plot->name(), QLatin1String("plot"));                                                                                                             \
//	/* enable once implemented correctly */                                                                                                                    \
//	/* QCOMPARE(plot->type(), AspectType::CartesianPlot); */                                                                                                   \
//                                                                                                                                                               \
//	GET_CURVE_PRIVATE(plot, 0, "lastValueInvalid", lastValueInvalidCurve)                                                                                      \
//	GET_CURVE_PRIVATE(plot, 1, "lastVertical", lastVerticalCurve)                                                                                              \
//	GET_CURVE_PRIVATE(plot, 2, "withGap", withGapCurve)                                                                                                        \
//	GET_CURVE_PRIVATE(plot, 3, "withGap2", withGapCurve2)

#define COMPARE(actual, expected, message) \
do {\
	if (!QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__)) {\
		qDebug() << message; \
		return;\
	}\
} while (false)

QHash<QString, int> RetransformTest::statistic(bool includeSuppressed) {
	QHash<QString, int> result;
	for (auto& log: logs) {
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
	logs.clear();
}

void RetransformTest::initTestCase() {
	// Called before every test
	resetRetransformCount();
}

void RetransformTest::aspectRetransformed(const AbstractAspect* sender, bool suppressed) {

	logs.append(Retransformed(sender, suppressed));
}


void RetransformTest::aspectAdded(const AbstractAspect* aspect) {
	connect(aspect, &AbstractAspect::retransformCalledSignal, this, &RetransformTest::aspectRetransformed);
}

void RetransformTest::TestLoadProject() {
	Project project;

	// Does not work during load.
	//connect(&project, &Project::aspectAdded, this, &RetransformTest::aspectAdded);

	project.load(QFINDTESTDATA(QLatin1String("data/p1.lml")));

	QStringList list = {
		"Project/Worksheet/xy-plot",
		"Project/Worksheet/xy-plot/x",
		"Project/Worksheet/xy-plot/y",
		"Project/Worksheet/xy-plot/sin",
		"Project/Worksheet/xy-plot/cos",
		"Project/Worksheet/xy-plot/tan",
		"Project/Worksheet/xy-plot/y-axis",
		"Project/Worksheet/xy-plot/legend"
	};

	auto children = project.children(AspectType::AbstractAspect, AbstractAspect::ChildIndexFlag::Recursive);
	for (auto& child: children) {
		bool expectedCallCount = 0;
		const QString& path = child->path();
		if (list.contains(path))
			expectedCallCount = 1;
		COMPARE(callCount(child, false), expectedCallCount, path);
	}
}

void RetransformTest::TestResizeWindows() {
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
	QCOMPARE(children.length(), 14);
	for (const auto& child: children)
		connect(child, &AbstractAspect::retransformCalledSignal, this, &RetransformTest::aspectRetransformed);

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

	// TODO: set to 6. legend should not retransform
	// plot it self does not change so retransform is not called on cartesianplotPrivate
	 QStringList list = {"Project/Worksheet/xy-plot/x",
	 "Project/Worksheet/xy-plot/y",
	 "Project/Worksheet/xy-plot/sin",
	 "Project/Worksheet/xy-plot/cos",
	 "Project/Worksheet/xy-plot/tan",
	 "Project/Worksheet/xy-plot/y-axis",
	 "Project/Worksheet/xy-plot/legend"};
	QCOMPARE(elementLogCount(false), 7);
	for (auto& s: list)
		QCOMPARE(callCount(s, false), 1);
}

// Test add new curve autoscale
// Test change data

QTEST_MAIN(RetransformTest)
