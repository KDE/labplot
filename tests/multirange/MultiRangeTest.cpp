/*
	File                 : MultiRangeTest.cpp
	Project              : LabPlot
	Description          : Tests for multi ranges
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MultiRangeTest.h"

#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotPrivate.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "kdefrontend/dockwidgets/XYCurveDock.h"

#include <QAction>
#include <QComboBox>
#include <QGraphicsSceneWheelEvent>

// ##############################################################################
// #####################  import of LabPlot projects ############################
// ##############################################################################

#define LOAD_PROJECT                                                                                                                                           \
	Project project;                                                                                                                                           \
	project.load(QFINDTESTDATA(QLatin1String("data/TestMultiRange.lml")));                                                                                     \
	/* check the project tree for the imported project */                                                                                                      \
	/* first child of the root folder */                                                                                                                       \
	auto* aspect = project.child<AbstractAspect>(0);                                                                                                           \
	QVERIFY(aspect != nullptr);                                                                                                                                \
	if (aspect)                                                                                                                                                \
		QCOMPARE(aspect->name(), QLatin1String("Arbeitsblatt"));                                                                                               \
	QVERIFY(aspect->type() == AspectType::Worksheet);                                                                                                          \
	auto w = dynamic_cast<Worksheet*>(aspect);                                                                                                                 \
	if (!w)                                                                                                                                                    \
		return;                                                                                                                                                \
                                                                                                                                                               \
	auto p1 = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(0));                                                                                   \
	QVERIFY(p1 != nullptr);                                                                                                                                    \
	auto p2 = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(1));                                                                                   \
	QVERIFY(p2 != nullptr);                                                                                                                                    \
	if (!p1 || !p2)                                                                                                                                            \
		return;                                                                                                                                                \
                                                                                                                                                               \
	auto* view = dynamic_cast<WorksheetView*>(w->view());                                                                                                      \
	QVERIFY(view != nullptr);                                                                                                                                  \
	view->initActions(); /* needed by SET_CARTESIAN_MOUSE_MODE() */                                                                                            \
                                                                                                                                                               \
	/* axis selected */                                                                                                                                        \
	auto sinCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(0));                                                                                             \
	QVERIFY(sinCurve != nullptr);                                                                                                                              \
	if (!sinCurve)                                                                                                                                             \
		return;                                                                                                                                                \
	QCOMPARE(sinCurve->name(), QStringLiteral("sinCurve"));                                                                                                    \
	auto tanCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(1));                                                                                             \
	QVERIFY(tanCurve != nullptr);                                                                                                                              \
	if (!tanCurve)                                                                                                                                             \
		return;                                                                                                                                                \
	QCOMPARE(tanCurve->name(), QStringLiteral("tanCurve"));                                                                                                    \
	auto logCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(2));                                                                                             \
	QVERIFY(logCurve != nullptr);                                                                                                                              \
	if (!logCurve)                                                                                                                                             \
		return;                                                                                                                                                \
	QCOMPARE(logCurve->name(), QStringLiteral("logx"));                                                                                                        \
                                                                                                                                                               \
	auto cosCurve = dynamic_cast<XYCurve*>(p2->child<XYCurve>(0));                                                                                             \
	QVERIFY(cosCurve != nullptr);                                                                                                                              \
	if (!cosCurve)                                                                                                                                             \
		return;                                                                                                                                                \
	QCOMPARE(cosCurve->name(), QStringLiteral("cosCurve"));                                                                                                    \
                                                                                                                                                               \
	auto horAxisP1 = static_cast<Axis*>(p1->child<Axis>(0));                                                                                                   \
	QVERIFY(horAxisP1 != nullptr);                                                                                                                             \
	QCOMPARE(horAxisP1->orientation() == Axis::Orientation::Horizontal, true);                                                                                 \
                                                                                                                                                               \
	auto vertAxisP1 = static_cast<Axis*>(p1->child<Axis>(1));                                                                                                  \
	QVERIFY(vertAxisP1 != nullptr);                                                                                                                            \
	QCOMPARE(vertAxisP1->orientation() == Axis::Orientation::Vertical, true);                                                                                  \
                                                                                                                                                               \
	auto vertAxis2P1 = static_cast<Axis*>(p1->child<Axis>(2));                                                                                                 \
	QVERIFY(vertAxis2P1 != nullptr);                                                                                                                           \
	QCOMPARE(vertAxis2P1->orientation() == Axis::Orientation::Vertical, true);                                                                                 \
                                                                                                                                                               \
	auto vertAxis3P1 = static_cast<Axis*>(p1->child<Axis>(3));                                                                                                 \
	QVERIFY(vertAxis3P1 != nullptr);                                                                                                                           \
	QCOMPARE(vertAxis3P1->orientation() == Axis::Orientation::Vertical, true);                                                                                 \
	QCOMPARE(vertAxis3P1->name(), QStringLiteral("y-axis 1"));                                                                                                 \
                                                                                                                                                               \
	auto horAxisP2 = static_cast<Axis*>(p2->child<Axis>(0));                                                                                                   \
	QVERIFY(horAxisP2 != nullptr);                                                                                                                             \
	QCOMPARE(horAxisP2->orientation() == Axis::Orientation::Horizontal, true);

#define SET_CARTESIAN_MOUSE_MODE(mode)                                                                                                                         \
	QAction a(nullptr);                                                                                                                                        \
	a.setData(static_cast<int>(mode));                                                                                                                         \
	view->cartesianPlotMouseModeChanged(&a);

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

	//	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.2, 0.6);
	//	CHECK_RANGE(p1, sinCurve, Dimension::Y, -0.5, 0.3);

	//	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	//	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);

	//	CHECK_RANGE(p2, cosCurve, x, 0., 1.);
	//	CHECK_RANGE(p2, cosCurve, y, -1., 1.);
}

// ZOOM SELECTION

void MultiRangeTest::zoomXSelection_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	w->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	horAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	// select range with mouse
	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), -1);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), -1);
	p1->mouseReleaseZoomSelectionMode(-1);

	// DEBUG_RANGE(p1, sinCurve)
	// DEBUG_RANGE(p1, tanCurve)
	// DEBUG_RANGE(p1, logCurve)

	CHECK_RANGE(p1, sinCurve, Dimension::X, .2, .6); // zoom
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, .2, .6); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 20., 60.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.); // No niceExtends() done!

	QVector<double> ref = {-250, -150.0, -50, 50, 150, 250};
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), ref);
	ref = {-1., -0.5, 0.0, 0.5, 1.0};
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3); // on third axis there is no autoscale, because it uses a different range
}

void MultiRangeTest::zoomXSelection_SingleRange() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	horAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), 0);
	p1->mouseReleaseZoomSelectionMode(0);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.2, 0.6); // zoom
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.2, 0.6); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.); // should not change, because y scale is not auto

	QVector<double> ref = {-250, -150.0, -50, 50, 150, 250};
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), ref);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3); // on third axis there is no autoscale, because it uses a different range
}

void MultiRangeTest::zoomYSelection_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	vertAxisP1->setSelected(true);
	w->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomYSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), -1);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), -1);
	p1->mouseReleaseZoomSelectionMode(-1);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -0.8, 0.6); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -150., 100.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -7., 2.); // zoom

	QVector<double> ref = {-150.0, -100, -50, 0, 50, 100};
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), ref);
	ref = {-0.8, -0.6, -0.4, -0.2, 0, 0.2, 0.4, 0.6};
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), ref);
	ref = {-7., -4., -1., 2.};
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), ref);
}

void MultiRangeTest::zoomYSelection_SingleRange() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	vertAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomYSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), 0);
	p1->mouseReleaseZoomSelectionMode(vertAxisP1->coordinateSystemIndex());

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -150., 100.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	QVector<double> ref = {-150.0, -100, -50, 0, 50, 100};
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), ref);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3);
}

void MultiRangeTest::zoomSelection_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	horAxisP1->setSelected(true);
	vertAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), -1);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), -1);
	p1->mouseReleaseZoomSelectionMode(-1);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.2, 0.6); // zoom
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -0.8, 0.6); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.2, 0.6); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -150., 100.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 20., 60.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::Y, -7., 2.); // zoom

	QVector<double> ref = {-150.0, -100, -50, 0, 50, 100};
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), ref);
	ref = {-0.8, -0.6, -0.4, -0.2, 0, 0.2, 0.4, 0.6};
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), ref);
	ref = {-7., -4., -1., 2.};
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), ref);
}

void MultiRangeTest::zoomSelection_SingleRange() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	horAxisP1->setSelected(true);
	vertAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), 0);
	p1->mouseReleaseZoomSelectionMode(0);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.2, 0.6); // zoom
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.2, 0.6); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -150., 100.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	QVector<double> ref = {-150.0, -100, -50, 0, 50, 100};
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), ref);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3);
}

// ZOOM

void MultiRangeTest::zoomInX_SingleRange() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	horAxisP1->setSelected(true);
	p1->zoomInX(0);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.1, 0.9); // zoom
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.1, 0.9); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale
	p1->navigate(0, CartesianPlot::NavigationOperation::ScaleAutoX);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3); // on third axis there is no autoscale, because it uses a different range
}

void MultiRangeTest::zoomInX_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	horAxisP1->setSelected(true);
	p1->zoomInX();

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.1, 0.9); // zoom
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.1, 0.9); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 10., 90.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale (all)
	p1->navigate(-1, CartesianPlot::NavigationOperation::ScaleAuto);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	QVector<double> ref = {-10, -7.71429, -5.42857, -3.14286, -0.857143, 1.42857, 3.71429, 6};
	// COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), ref); // vertAxis3 is not autoscaled when loading, after autoscaling the values are different
}

void MultiRangeTest::zoomInY_SingleRange() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	vertAxisP1->setSelected(true);
	p1->zoomInY(0);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -200., 200.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale
	p1->navigate(0, CartesianPlot::NavigationOperation::ScaleAutoY);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3); // on third axis there is no autoscale, because it uses a different range
}

void MultiRangeTest::zoomInY_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	vertAxisP1->setSelected(true);
	p1->zoomInY();

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -0.8, 0.8); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -200., 200.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -8., 4.); // zoom

	// check auto scale
	p1->navigate(-1, CartesianPlot::NavigationOperation::ScaleAutoY);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	QVector<double> ref = {-10, -6, -2, 2, 6};
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), ref); // vertAxis3 is not autoscaled when loading, after autoscaling the values are different
}

void MultiRangeTest::zoomOutX_SingleRange() {
	LOAD_PROJECT

	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	horAxisP1->setSelected(true);
	p1->zoomOutX(0);

	CHECK_RANGE(p1, sinCurve, Dimension::X, -0.2, 1.2); // zoom
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, -0.2, 1.2); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale
	p1->navigate(0, CartesianPlot::NavigationOperation::ScaleAutoX);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3); // on third axis there is no autoscale, because it uses a different range
}

void MultiRangeTest::zoomOutX_AllRanges() {
	LOAD_PROJECT

	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	horAxisP1->setSelected(true);
	p1->zoomOutX();

	CHECK_RANGE(p1, sinCurve, Dimension::X, -0.2, 1.2); // zoom
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, -0.2, 1.2); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, -20., 120.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale
	p1->navigate(-1, CartesianPlot::NavigationOperation::ScaleAutoX);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3); // on third axis there is no autoscale, because it uses a different range
}

void MultiRangeTest::zoomOutY_SingleRange() {
	LOAD_PROJECT

	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	vertAxisP1->setSelected(true);
	p1->zoomOutY(0);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -300., 300.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale
	p1->navigate(0, CartesianPlot::NavigationOperation::ScaleAutoY);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3); // on third axis there is no autoscale, because it uses a different range
}

void MultiRangeTest::zoomOutY_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	vertAxisP1->setSelected(true);
	p1->zoomOutY();

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1.5, 1.5); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -300., 300.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -12., 8.); // zoom

	// check auto scale (all)
	p1->navigate(-1, CartesianPlot::NavigationOperation::ScaleAuto);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	QVector<double> ref = {-10, -6, -2, 2, 6};
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), ref); // vertAxis3 is not autoscaled when loading, after autoscaling the values are different
}

// SHIFT

void MultiRangeTest::shiftLeft_SingleRange() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	horAxisP1->setSelected(true);
	p1->shiftLeftX(0);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.1, 1.1); // shift
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.1, 1.1); // shift
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale
	p1->navigate(0, CartesianPlot::NavigationOperation::ScaleAutoX);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3); // on third axis there is no autoscale, because it uses a different range
}

void MultiRangeTest::shiftRight_SingleRange() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	horAxisP1->setSelected(true);
	p1->shiftRightX(0);

	CHECK_RANGE(p1, sinCurve, Dimension::X, -0.1, 0.9); // shift
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, -0.1, 0.9); // shift
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale
	p1->navigate(0, CartesianPlot::NavigationOperation::ScaleAutoX);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3); // on third axis there is no autoscale
}

void MultiRangeTest::shiftLeft_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	horAxisP1->setSelected(true);
	p1->shiftLeftX();

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.1, 1.1); // shift
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.1, 1.1); // shift
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 10., 110.); // shift
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale (all)
	p1->navigate(-1, CartesianPlot::NavigationOperation::ScaleAutoX);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check if retransform is done by comparing the tickLabelValues
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3); // on third axis there is no autoscale
}

void MultiRangeTest::shiftRight_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	horAxisP1->setSelected(true);
	p1->shiftRightX();

	CHECK_RANGE(p1, sinCurve, Dimension::X, -0.1, 0.9); // shift
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, -0.1, 0.9); // shift
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, -10., 90.); // shift
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale
	p1->navigate(-1, CartesianPlot::NavigationOperation::ScaleAutoX);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check if retransform is done by comparing the tickLabelValues
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3); // on third axis there is no autoscale
}

void MultiRangeTest::shiftUp_SingleRange() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	vertAxisP1->setSelected(true);
	p1->shiftUpY(0);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -300., 200.); // shift
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale
	p1->navigate(0, CartesianPlot::NavigationOperation::ScaleAutoY);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// retransform of vertAxisP1 is done, so the tickLabelValues change back
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3);
}

void MultiRangeTest::shiftDown_SingleRange() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();

	vertAxisP1->setSelected(true);
	p1->shiftDownY(0);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.)
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -200., 300.); // shift
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// check auto scale
	// p1->enableAutoScale(Dimension::Y, 0);
	p1->navigate(0, CartesianPlot::NavigationOperation::ScaleAutoY);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// retransform of vertAxisP1 is done, so the tickLabelValues change back
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), refValuesAxis3);
}

void MultiRangeTest::shiftUp_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	vertAxisP1->setSelected(true);
	p1->shiftUpY();

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1.2, 0.8); // shift
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -300., 200.); // shift
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -11.6, 4.4); // shift

	// check auto scale
	p1->setSelected(true);
	p1->navigate(-1, CartesianPlot::NavigationOperation::ScaleAuto);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
	// retransform of vertAxisP1 is done, so the tickLabelValues change back
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	QVector<double> ref = {-10, -6, -2, 2, 6};
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), ref); // vertAxis3 is not autoscaled when loading, after autoscaling the values are different
}

void MultiRangeTest::shiftDown_AllRanges() {
	LOAD_PROJECT
	auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
	vertAxisP1->setSelected(true);
	p1->shiftDownY();

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -0.8, 1.2); // shift
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -200., 300.); // shift
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -8.4, 7.6); // shift

	// check auto scale (all)
	p1->setSelected(true);
	p1->navigate(-1, CartesianPlot::NavigationOperation::ScaleAuto);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	// retransform of vertAxisP1 is done, so the tickLabelValues change back
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValuesAxis1);
	COMPARE_DOUBLE_VECTORS(vertAxis2P1->tickLabelValues(), refValuesAxis2);
	QVector<double> ref = {-10, -6, -2, 2, 6};
	COMPARE_DOUBLE_VECTORS(vertAxis3P1->tickLabelValues(), ref); // vertAxis3 is not autoscaled when loading, after autoscaling the values are different
}

void MultiRangeTest::autoScaleYAfterZoomInX() {
	/* 1) Zoom in X
	 * 2) Autoscale X
	 * 3) Check that y also changed! */
	LOAD_PROJECT
	auto refValues = horAxisP1->tickLabelValues();
	horAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), 0);
	p1->mouseReleaseZoomSelectionMode(0);

	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.2, 0.6); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);

	p1->navigate(tanCurve->coordinateSystemIndex(), CartesianPlot::NavigationOperation::ScaleAutoX);

	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, horAxisP1, Dimension::X, 0., 1.); // range is changed in retransform scale

	// retransform of horAxisP1 is done, so the tickLabelValues change back
	// to be in the range of 0, 1
	COMPARE_DOUBLE_VECTORS(horAxisP1->tickLabelValues(), refValues);
}

void MultiRangeTest::autoScaleXAfterZoomInY() {
	LOAD_PROJECT
	auto refValues = vertAxisP1->tickLabelValues();
	vertAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomYSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -150), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 100), 0);
	p1->mouseReleaseZoomSelectionMode(0);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -150., 100.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);

	p1->navigate(tanCurve->coordinateSystemIndex(), CartesianPlot::NavigationOperation::ScaleAutoY);

	// retransform of vertAxisP1 is done, so the tickLabelValues change back
	COMPARE_DOUBLE_VECTORS(vertAxisP1->tickLabelValues(), refValues);
}

void MultiRangeTest::baseDockSetAspects_NoPlotRangeChange() {
	LOAD_PROJECT

	const int sinCurveCSystemIndex = sinCurve->coordinateSystemIndex();
	const int tanCurveCSystemIndex = tanCurve->coordinateSystemIndex();
	QVERIFY(sinCurveCSystemIndex != tanCurveCSystemIndex);
	// checks directly the plot. In the basedock the element is used and not the plot, so do it here too
	QVERIFY(sinCurve->coordinateSystemCount() == 3);

	XYCurveDock dock(nullptr);
	dock.setupGeneral();
	dock.setCurves(QList<XYCurve*>({sinCurve, tanCurve}));

	dock.updatePlotRanges();

	// The coordinatesystem indices shall not change
	QCOMPARE(sinCurveCSystemIndex, sinCurve->coordinateSystemIndex());
	QCOMPARE(tanCurveCSystemIndex, tanCurve->coordinateSystemIndex());
}

/*!
 * \brief MultiRangeTest::mouseWheelXAxisApplyToAllX
 * If applied to all x is activated, using the mousewheel on a
 * selected axis should also execute the mousewheel on other plots
 */
void MultiRangeTest::mouseWheelXAxisApplyToAllX() {
	LOAD_PROJECT

	QCOMPARE(w->cartesianPlotActionMode(), Worksheet::CartesianPlotActionMode::ApplyActionToAllX);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
	CHECK_RANGE(p2, horAxisP1, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::Y, -1., 1.);

	horAxisP1->setSelected(true); // seems not to work
	view->m_selectedElement = horAxisP1;

	int counter = 0;
	connect(p1,
			&CartesianPlot::wheelEventSignal,
			[&counter](const QPointF& relScenePos, int delta, int xIndex, int /*yIndex*/, bool considerDimension, Dimension dim) {
				QCOMPARE(delta, 10);
				QCOMPARE(xIndex, 0); // x Range of horAxisP1
				QCOMPARE(relScenePos.x(), 0.5);
				QCOMPARE(relScenePos.y(), 0.5);
				QCOMPARE(considerDimension, true);
				QCOMPARE(dim, Dimension::X);
				counter++;
			});

	QGraphicsSceneWheelEvent event;
	event.setDelta(10);
	event.setPos(QPointF(p1->dataRect().center().x(), p1->dataRect().center().y()));
	p1->d_func()->wheelEvent(&event);

	QCOMPARE(counter, 1);

	// All x ranges are zoomed, for plot 1 and plot 2
	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.1, 0.9); // zoom
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.1, 0.9); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 10., 90.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
	CHECK_RANGE(p2, horAxisP1, Dimension::X, 0.1, 0.9);
	CHECK_RANGE(p2, cosCurve, Dimension::X, 0.1, 0.9);
	CHECK_RANGE(p2, cosCurve, Dimension::Y, -1., 1.);
}

/*!
 * \brief MultiRangeTest::mouseWheelXAxisApplyToAllX
 * If applied to all x is activated, using the mousewheel on a
 * selected axis should also execute the mousewheel on other plots
 * This time the second x axis is used. In the second plot no second x axis is used
 * so check that application does not crash
 */
void MultiRangeTest::mouseWheelTanCurveApplyToAllX() {
	LOAD_PROJECT

	QCOMPARE(w->cartesianPlotActionMode(), Worksheet::CartesianPlotActionMode::ApplyActionToAllX);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
	CHECK_RANGE(p2, horAxisP1, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::Y, -1., 1.);

	tanCurve->setSelected(true); // seems not to work
	view->m_selectedElement = tanCurve;

	int counter = 0;
	connect(p1,
			&CartesianPlot::wheelEventSignal,
			[&counter](const QPointF& relScenePos, int delta, int xIndex, int yIndex, bool considerDimension, Dimension dim) {
				Q_UNUSED(yIndex);
				Q_UNUSED(dim);
				QCOMPARE(relScenePos.x(), 0.5);
				QCOMPARE(relScenePos.y(), 0.5);
				QCOMPARE(delta, 10);
				QCOMPARE(xIndex, 0); // tan curve has xIndex 0
				QCOMPARE(considerDimension, false);
				counter++;
			});

	QGraphicsSceneWheelEvent event;
	event.setDelta(10);
	event.setPos(QPointF(p1->dataRect().center().x(), p1->dataRect().center().y()));
	p1->d_func()->wheelEvent(&event);

	QCOMPARE(counter, 1);

	// All x ranges are zoomed, for plot 1 and plot 2
	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.1, 0.9); // zoom
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.1, 0.9); // zoom
	// zoomed in, because with scrolling both axes are scrolled
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -200., 200.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 10., 90.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
	CHECK_RANGE(p2, horAxisP1, Dimension::X, 0.1, 0.9);
	CHECK_RANGE(p2, cosCurve, Dimension::X, 0.1, 0.9);
	CHECK_RANGE(p2, cosCurve, Dimension::Y, -1., 1.);
}

void MultiRangeTest::mouseWheelXAxisApplyToSelected() {
	LOAD_PROJECT

	w->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	QCOMPARE(w->cartesianPlotActionMode(), Worksheet::CartesianPlotActionMode::ApplyActionToSelection);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
	CHECK_RANGE(p2, horAxisP1, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::Y, -1., 1.);

	horAxisP1->setSelected(true); // seems not to work
	view->m_selectedElement = horAxisP1;

	int counter = 0;
	connect(p1,
			&CartesianPlot::wheelEventSignal,
			[&counter](const QPointF& sceneRelPos, int delta, int xIndex, int /*yIndex*/, bool considerDimension, Dimension dim) {
				Q_UNUSED(sceneRelPos);
				QCOMPARE(delta, 10);
				QCOMPARE(xIndex, 0); // x Range of horAxisP1
				QCOMPARE(considerDimension, true);
				QCOMPARE(dim, Dimension::X);
				counter++;
			});

	QGraphicsSceneWheelEvent event;
	event.setDelta(10);
	event.setPos(QPointF(p1->dataRect().center().x(), p1->dataRect().center().y()));
	p1->d_func()->wheelEvent(&event);

	QCOMPARE(counter, 1);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.1, 0.9); // zoom
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.1, 0.9); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.); // Not zoomed
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
	CHECK_RANGE(p2, horAxisP1, Dimension::X, 0., 1.); // Not zoomed
	CHECK_RANGE(p2, cosCurve, Dimension::X, 0., 1.); // Not zoomed
	CHECK_RANGE(p2, cosCurve, Dimension::Y, -1., 1.);
}

void MultiRangeTest::axisMouseMoveApplyToAllX() {
	LOAD_PROJECT

	w->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToAllX);
	QCOMPARE(w->cartesianPlotActionMode(), Worksheet::CartesianPlotActionMode::ApplyActionToAllX);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
	CHECK_RANGE(p2, horAxisP1, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::Y, -1., 1.);

	const int delta = -10; // delta > 0 --> right or up
	horAxisP1->shiftSignal(delta, Dimension::X, p1->coordinateSystem(horAxisP1->coordinateSystemIndex())->index(Dimension::X));

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.1, 1.1); // shift
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.1, 1.1); // shift
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 10., 110.); // shift
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
	CHECK_RANGE(p2, horAxisP1, Dimension::X, 0.1, 1.1); // shift
	CHECK_RANGE(p2, cosCurve, Dimension::X, 0.1, 1.1); // shift
	CHECK_RANGE(p2, cosCurve, Dimension::Y, -1., 1.);
}

void MultiRangeTest::axisMouseMoveApplyToSelection() {
	LOAD_PROJECT

	w->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	QCOMPARE(w->cartesianPlotActionMode(), Worksheet::CartesianPlotActionMode::ApplyActionToSelection);

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
	CHECK_RANGE(p2, horAxisP1, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::Y, -1., 1.);

	const int delta = -10; // delta > 0 --> right or up
	horAxisP1->shiftSignal(delta, Dimension::X, p1->coordinateSystem(horAxisP1->coordinateSystemIndex())->index(Dimension::X));

	CHECK_RANGE(p1, sinCurve, Dimension::X, 0.1, 1.1); // shift
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0.1, 1.1); // shift
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.);
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
	CHECK_RANGE(p2, horAxisP1, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p2, cosCurve, Dimension::Y, -1., 1.);
}

QTEST_MAIN(MultiRangeTest)
