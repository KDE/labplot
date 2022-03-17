/*
    File                 : MultiRangeTest.cpp
    Project              : LabPlot
    Description          : Tests for multi ranges
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#define private public

#include "MultiRangeTest.h"

#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/matrix/Matrix.h"
#include "backend/worksheet/Worksheet.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/AxisPrivate.h"

#undef private

#include <QAction>

//##############################################################################
//#####################  import of LabPlot projects ############################
//##############################################################################

#define LOAD_PROJECT \
	Project project; \
	project.load(QFINDTESTDATA(QLatin1String("data/TestMultiRange.lml"))); \
	/* check the project tree for the imported project */ \
	/* first child of the root folder */ \
	auto* aspect = project.child<AbstractAspect>(0); \
	QVERIFY(aspect != nullptr); \
	if (aspect) \
		QCOMPARE(aspect->name(), QLatin1String("Arbeitsblatt")); \
	QVERIFY(aspect->type() == AspectType::Worksheet); \
	auto w = dynamic_cast<Worksheet*>(aspect); \
	if (!w) return; \
 \
	auto p1 = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(0)); \
	QVERIFY(p1 != nullptr); \
	auto p2 = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(1)); \
	QVERIFY(p2 != nullptr); \
	if (!p1 || !p2) return; \
\
	auto* view = dynamic_cast<WorksheetView*>(w->view()); \
	QVERIFY(view != nullptr); \
	Q_EMIT w->useViewSizeRequested(); /* To init the worksheet view actions */\
\
	/* axis selected */ \
	auto sinCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(0)); \
	QVERIFY(sinCurve != nullptr); \
	if (!sinCurve) return; \
	QCOMPARE(sinCurve->name(), "sinCurve"); \
	auto tanCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(1)); \
	QVERIFY(tanCurve != nullptr); \
	if (!tanCurve) return; \
	QCOMPARE(tanCurve->name(), "tanCurve"); \
	auto logCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(2)); \
	QVERIFY(logCurve != nullptr); \
	if (!logCurve) return; \
	QCOMPARE(logCurve->name(), "logx"); \
\
	auto cosCurve = dynamic_cast<XYCurve*>(p2->child<XYCurve>(0)); \
	QVERIFY(cosCurve != nullptr); \
	if (!cosCurve) return; \
	QCOMPARE(cosCurve->name(), "cosCurve"); \
	\
	auto horAxisP1 = static_cast<Axis*>(p1->child<Axis>(0)); \
	QVERIFY(horAxisP1 != nullptr); \
	QCOMPARE(horAxisP1->orientation() == Axis::Orientation::Horizontal, true); \
	\
	auto vertAxisP1 = static_cast<Axis*>(p1->child<Axis>(1)); \
	QVERIFY(vertAxisP1 != nullptr); \
	QCOMPARE(vertAxisP1->orientation() == Axis::Orientation::Vertical, true); \
\
	auto vertAxis2P1 = static_cast<Axis*>(p1->child<Axis>(2)); \
	QVERIFY(vertAxis2P1 != nullptr); \
	QCOMPARE(vertAxis2P1->orientation() == Axis::Orientation::Vertical, true); \
\
	auto vertAxis3P1 = static_cast<Axis*>(p1->child<Axis>(3)); \
	QVERIFY(vertAxis3P1 != nullptr); \
	QCOMPARE(vertAxis3P1->orientation() == Axis::Orientation::Vertical, true); \
	QCOMPARE(vertAxis3P1->name(), "y-axis 1");

#define SET_CARTESIAN_MOUSE_MODE(mode) \
	QAction a(nullptr); \
	a.setData(static_cast<int>(mode)); \
	view->cartesianPlotMouseModeChanged(&a);

#define COMPARE_DOUBLE_VECTORS(res, ref) \
	QCOMPARE(res.length(), ref.length()); \
	for (int i = 0; i < res.length(); i++) \
		QVERIFY(qFuzzyCompare(res.at(i), ref.at(i)));



////////////////////////////////////////////////////////////////

// Test1:
// Check if the correct actions are enabled/disabled.

// Combinations: Curve selected. Zoom SelectionX , Plot selected: Autoscale X, Autoscale

// Other tests:
// Apply Action To Selection
	// Curve plot 1 selected
	//		Zoom Selection (check dirty state)
	//		X Zoom Selection
	//		Y Zoom Selection
	//		Autoscale X
	//		Autoscale Y
	//		Autoscale
	// X Axis selected
	// Y Axis selected
	// Plot selected
// Apply Action To All
	// Curve plot 1 selected
	// XAxis plot 1 selected
	// YAxis plot 1 selected
	// Curve plot 2 selected
// Apply Action to AllX
// Apply Action to AllY

/*!
 * \brief MultiRangeTest::applyActionToSelection_CurveSelected_ZoomSelection
 *
 */
void MultiRangeTest::applyActionToSelection_CurveSelected_ZoomSelection() {
	LOAD_PROJECT
//	QActionGroup* cartesianPlotActionGroup = nullptr;
//	auto children = view->findChildren<QActionGroup*>();
//	for (int i=0; i < children.count(); i++) {
//		auto actions = children[i]->findChildren<QAction*>();
//		for (int j=0; j < actions.count(); j++) {
//			if (actions[j]->text() == i18n("Select Region and Zoom In")) {
//				// correct action group found
//				cartesianPlotActionGroup = children[i];
//				break;
//			}
//		}
////
//	}
//	QCOMPARE(cartesianPlotActionGroup != nullptr, true);
	// TODO: where to check if the actions are enabled or not?

//	w->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
//	sinCurve->setSelected(true);
//	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomSelection)

//	// Dirty is not stored in the project
//	p1->scaleAuto(-1);
//	p2->scaleAuto(-1);

//	QPointF logicPos(0.2, -0.5);
//	auto* sender =const_cast<QObject*>(QObject::sender());
//	sender = p1;
//	w->cartesianPlotMousePressZoomSelectionMode(logicPos);
//	w->cartesianPlotMouseMoveZoomSelectionMode(QPointF(0.6, 0.3));
//	w->cartesianPlotMouseReleaseZoomSelectionMode();

//	QCOMPARE(p1->xRangeDirty(sinCurve->coordinateSystemIndex()), true);
//	QCOMPARE(p1->yRangeDirty(sinCurve->coordinateSystemIndex()), true);
//	// True, because sinCurve and tanCurve use the same range
//	QCOMPARE(p1->xRangeDirty(tanCurve->coordinateSystemIndex()), true);
//	QCOMPARE(p1->yRangeDirty(tanCurve->coordinateSystemIndex()), true);

//	QCOMPARE(p2->xRangeDirty(cosCurve->coordinateSystemIndex()), false);
//	QCOMPARE(p2->yRangeDirty(cosCurve->coordinateSystemIndex()), false);

//	CHECK_RANGE(p1, sinCurve, x, 0.2, 0.6);
//	CHECK_RANGE(p1, sinCurve, y, -0.5, 0.3);

//	CHECK_RANGE(p1, tanCurve, x, 0, 1);
//	CHECK_RANGE(p1, tanCurve, y, -250, 250);

//	CHECK_RANGE(p2, cosCurve, x, 0, 1);
//	CHECK_RANGE(p2, cosCurve, y, -1, 1);
}

// ZOOM SELECTION

void MultiRangeTest::zoomXSelection_AllRanges() {
	LOAD_PROJECT
	w->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	horAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	// select range with mouse
	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), -1);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), -1);
	p1->mouseReleaseZoomSelectionMode(-1);

	//DEBUG_RANGE(p1, sinCurve)
	//DEBUG_RANGE(p1, tanCurve)
	//DEBUG_RANGE(p1, logCurve)

	CHECK_RANGE(p1, sinCurve, x, 0.2, 0.6);	// zoom
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0.2, 0.6);	// zoom
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 20., 60.);	// zoom
	CHECK_RANGE(p1, logCurve, y, -10., 10.);
}

void MultiRangeTest::zoomXSelection_SingleRange() {
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), 0);
	p1->mouseReleaseZoomSelectionMode(0);

	CHECK_RANGE(p1, sinCurve, x, 0.2, 0.6);	// zoom
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0.2, 0.6);	// zoom
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0., 100.);
	CHECK_RANGE(p1, logCurve, y, -10, 6); // should not change, because y scale is not auto
}

void MultiRangeTest::zoomYSelection_AllRanges() {
	LOAD_PROJECT
	vertAxisP1->setSelected(true);
	w->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomYSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), -1);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), -1);
	p1->mouseReleaseZoomSelectionMode(-1);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1.0, 0.5);	// zoom
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -150., 100.);	// zoom
	CHECK_RANGE(p1, logCurve, x, 0., 100.);
	CHECK_RANGE(p1, logCurve, y, -7, 2);		// zoom
}

void MultiRangeTest::zoomYSelection_SingleRange() {
	LOAD_PROJECT
	vertAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomYSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), 0);
	p1->mouseReleaseZoomSelectionMode(vertAxisP1->coordinateSystemIndex());

	CHECK_RANGE(p1, sinCurve, x, 0, 1);
	CHECK_RANGE(p1, sinCurve, y, -1, 1.);
	CHECK_RANGE(p1, tanCurve, x, 0, 1);
	CHECK_RANGE(p1, tanCurve, y, -150, 100);	// zoom
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::zoomSelection_AllRanges() {
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	vertAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), -1);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), -1);
	p1->mouseReleaseZoomSelectionMode(-1);

	CHECK_RANGE(p1, sinCurve, x, 0.2, 0.6);	// zoom
	CHECK_RANGE(p1, sinCurve, y, -1.0, 0.5);	// zoom
	CHECK_RANGE(p1, tanCurve, x, 0.2, 0.6);	// zoom
	CHECK_RANGE(p1, tanCurve, y, -150, 100);	// zoom
	CHECK_RANGE(p1, logCurve, x, 20, 60);	// zoom
	CHECK_RANGE(p1, logCurve, y, -7, 2);	// zoom
}

void MultiRangeTest::zoomSelection_SingleRange() {
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	vertAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), 0);
	p1->mouseReleaseZoomSelectionMode(0);

	CHECK_RANGE(p1, sinCurve, x, 0.2, 0.6);	// zoom
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0.2, 0.6);	// zoom
	CHECK_RANGE(p1, tanCurve, y, -150, 100);	// zoom
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);
}

// ZOOM

void MultiRangeTest::zoomInX_SingleRange() {
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	p1->zoomInX(0);

	CHECK_RANGE(p1, sinCurve, x, 0.1, 0.9);	// zoom
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0.1, 0.9);	// zoom
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale
	p1->enableAutoScaleX(0);
	p1->scaleAutoX(0, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

}

void MultiRangeTest::zoomInX_AllRanges() {
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	p1->zoomInX();

	CHECK_RANGE(p1, sinCurve, x, 0.1, 0.9);	// zoom
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0.1, 0.9);	// zoom
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 10, 90);	// zoom
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale (all)
	p1->enableAutoScaleX();
	p1->scaleAuto(-1, -1, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::zoomInY_SingleRange() {
	LOAD_PROJECT
	vertAxisP1->setSelected(true);
	p1->zoomInY(0);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -200, 200);	// zoom
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale
	p1->enableAutoScaleY(0);
	p1->scaleAutoY(0, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::zoomInY_AllRanges() {
	LOAD_PROJECT
	vertAxisP1->setSelected(true);
	p1->zoomInY();

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -0.5, 0.5);	// zoom
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -200, 200);	// zoom
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -5, 0);		// zoom

	// check auto scale
	p1->enableAutoScaleY();
	p1->scaleAutoY(-1, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 5);
}

void MultiRangeTest::zoomOutX_SingleRange() {
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	p1->zoomOutX(0);

	CHECK_RANGE(p1, sinCurve, x, -0.5, 1.5);	// zoom
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, -0.5, 1.5);	// zoom
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale
	p1->enableAutoScaleX(0);
	p1->scaleAutoX(0, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::zoomOutX_AllRanges() {
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	p1->zoomOutX();

	CHECK_RANGE(p1, sinCurve, x, -0.5, 1.5);	// zoom
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, -0.5, 1.5);	// zoom
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, -50, 150);		// zoom
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale
	p1->enableAutoScaleX();
	p1->scaleAutoX(-1, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::zoomOutY_SingleRange() {
	LOAD_PROJECT
	vertAxisP1->setSelected(true);
	p1->zoomOutY(0);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -300, 300);	// zoom
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale
	p1->enableAutoScaleY(0);
	p1->scaleAutoY(0, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::zoomOutY_AllRanges() {
	LOAD_PROJECT
	vertAxisP1->setSelected(true);
	p1->zoomOutY();

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1.5, 1.5);	// zoom
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -300, 300);	// zoom
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -15, 10);		// zoom

	// check auto scale (all)
	p1->enableAutoScaleY();
	p1->scaleAuto(-1, -1, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 5);
}

// SHIFT

void MultiRangeTest::shiftLeft_SingleRange() {
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	p1->shiftLeftX(0);

	CHECK_RANGE(p1, sinCurve, x, 0.1, 1.1);	// shift
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0.1, 1.1);	// shift
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale
	p1->enableAutoScaleX(0);
	p1->scaleAutoX(0, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::shiftRight_SingleRange() {
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	p1->shiftRightX(0);

	CHECK_RANGE(p1, sinCurve, x, -0.1, 0.9);	// shift
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, -0.1, 0.9);	// shift
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale
	p1->enableAutoScaleX(0);
	p1->scaleAutoX(0, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::shiftLeft_AllRanges() {
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	p1->shiftLeftX();

	CHECK_RANGE(p1, sinCurve, x, 0.1, 1.1);	// shift
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0.1, 1.1);	// shift
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 10, 110);	// shift
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale (all)
	p1->enableAutoScaleX();
	p1->scaleAuto(-1, -1, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::shiftRight_AllRanges() {
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	p1->shiftRightX();

	CHECK_RANGE(p1, sinCurve, x, -0.1, 0.9);	// shift
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, -0.1, 0.9);	// shift
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, -10, 90);		// shift
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale
	p1->enableAutoScaleX();
	p1->scaleAutoX(-1, true);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::shiftUp_SingleRange() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->d_func()->tickLabelValues;
	auto refValuesAxis2 = vertAxis2P1->d_func()->tickLabelValues;
	auto refValuesAxis3 = vertAxis3P1->d_func()->tickLabelValues;
	vertAxisP1->setSelected(true);
	p1->shiftUpY(0);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -300, 200);	// shift
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale
	p1->navigate(0, CartesianPlot::NavigationOperation::ScaleAutoY);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// retransform of vertAxisP1 is done, so the tickLabelValues change back
	COMPARE_DOUBLE_VECTORS(vertAxisP1->d_func()->tickLabelValues, refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->d_func()->tickLabelValues, refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->d_func()->tickLabelValues, refValuesAxis3);
}

void MultiRangeTest::shiftDown_SingleRange() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->d_func()->tickLabelValues;
	auto refValuesAxis2 = vertAxis2P1->d_func()->tickLabelValues;
	auto refValuesAxis3 = vertAxis3P1->d_func()->tickLabelValues;

	vertAxisP1->setSelected(true);
	p1->shiftDownY(0);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.)
	CHECK_RANGE(p1, tanCurve, y, -200, 300);	// shift
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// check auto scale
	//p1->enableAutoScaleY(0);
	p1->navigate(0, CartesianPlot::NavigationOperation::ScaleAutoY);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	// retransform of vertAxisP1 is done, so the tickLabelValues change back
	COMPARE_DOUBLE_VECTORS(vertAxisP1->d_func()->tickLabelValues, refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->d_func()->tickLabelValues, refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->d_func()->tickLabelValues, refValuesAxis3);
}

void MultiRangeTest::shiftUp_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->d_func()->tickLabelValues;
	auto refValuesAxis2 = vertAxis2P1->d_func()->tickLabelValues;
	vertAxisP1->setSelected(true);
	p1->shiftUpY();

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1.2, 0.8);	// shift
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -300, 200);	// shift
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -11.6, 4.4);	// shift

	// check auto scale
	p1->setSelected(true);
	p1->navigate(-1, CartesianPlot::NavigationOperation::ScaleAuto);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 5);
	// retransform of vertAxisP1 is done, so the tickLabelValues change back
	COMPARE_DOUBLE_VECTORS(vertAxisP1->d_func()->tickLabelValues, refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->d_func()->tickLabelValues, refValuesAxis2);
	QVector<double> ref = {-10.0, -5, 0, 5};
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->d_func()->tickLabelValues, ref); // vertAxis3 is not autoscaled, so it does not work as above
}

void MultiRangeTest::shiftDown_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->d_func()->tickLabelValues;
	auto refValuesAxis2 = vertAxis2P1->d_func()->tickLabelValues;
	auto refValuesAxis3 = vertAxis3P1->d_func()->tickLabelValues;
	vertAxisP1->setSelected(true);
	p1->shiftDownY();

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -0.8, 1.2);	// shift
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -200, 300);	// shift
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -8.4, 7.6);	// shift

	// check auto scale (all)
	p1->setSelected(true);
	p1->navigate(-1, CartesianPlot::NavigationOperation::ScaleAuto);

	CHECK_RANGE(p1, sinCurve, x, 0., 1.);
	CHECK_RANGE(p1, sinCurve, y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, x, 0., 1.);
	CHECK_RANGE(p1, tanCurve, y, -250, 250);
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 5);

	// retransform of vertAxisP1 is done, so the tickLabelValues change back
	COMPARE_DOUBLE_VECTORS(vertAxisP1->d_func()->tickLabelValues, refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->d_func()->tickLabelValues, refValuesAxis2);
	QVector<double> ref = {-10.0, -5, 0, 5};
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->d_func()->tickLabelValues, ref); // vertAxis3 is not autoscaled, so it does not work as above
}

void MultiRangeTest::autoScaleYAfterZoomInX() {
	/* 1) Zoom in X
	 * 2) Autoscale X
	 * 3) Check that y also changed! */
	LOAD_PROJECT
	auto refValues = horAxisP1->d_func()->tickLabelValues;
	horAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), 0);
	p1->mouseReleaseZoomSelectionMode(0);

	CHECK_RANGE(p1, tanCurve, x, 0.2, 0.6);	// zoom
	CHECK_RANGE(p1, tanCurve, y, -250, 250);

	p1->navigate(tanCurve->coordinateSystemIndex(), CartesianPlot::NavigationOperation::ScaleAutoX);

	CHECK_RANGE(p1, tanCurve, x, 0, 1);
	CHECK_RANGE(p1, horAxisP1, x, 0, 1); // range is changed in retransform scale

	// retransform of horAxisP1 is done, so the tickLabelValues change back
	// to be in the range of 0, 1
	COMPARE_DOUBLE_VECTORS(horAxisP1->d_func()->tickLabelValues, refValues);
}

void MultiRangeTest::autoScaleXAfterZoomInY() {
	LOAD_PROJECT
	auto refValues = vertAxisP1->d_func()->tickLabelValues;
	vertAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomYSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), 0);
	p1->mouseReleaseZoomSelectionMode(0);

	CHECK_RANGE(p1, sinCurve, x, 0, 1);
	CHECK_RANGE(p1, sinCurve, y, -1, 1.);
	CHECK_RANGE(p1, tanCurve, x, 0, 1);
	CHECK_RANGE(p1, tanCurve, y, -150, 100);	// zoom
	CHECK_RANGE(p1, logCurve, x, 0, 100);
	CHECK_RANGE(p1, logCurve, y, -10, 6);

	p1->navigate(tanCurve->coordinateSystemIndex(), CartesianPlot::NavigationOperation::ScaleAutoY);

	// retransform of vertAxisP1 is done, so the tickLabelValues change back
	COMPARE_DOUBLE_VECTORS(vertAxisP1->d_func()->tickLabelValues, refValues);
}

QTEST_MAIN(MultiRangeTest)
