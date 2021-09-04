/*
	File                 : MultiRangeTest.cpp
    Project              : LabPlot
    Description          : Tests for project imports
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MultiRangeTest.h"

#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/matrix/Matrix.h"
#include "backend/worksheet/Worksheet.h"
#define private public
#include "commonfrontend/worksheet/WorksheetView.h"
#undef private
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QAction>

void MultiRangeTest::initTestCase() {
//	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
//	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

//##############################################################################
//#####################  import of LabPlot projects ############################
//##############################################################################

#define LOAD_PROJECT \
	Project project; \
	project.load(QFINDTESTDATA(QLatin1String("data/TestMultiRange.lml"))); \
	/* check the project tree for the imported project */ \
	/* first child of the root folder, spreadsheet "Book3" */ \
	auto* aspect = project.child<AbstractAspect>(0); \
	QCOMPARE(aspect != nullptr, true); \
	if (aspect != nullptr) \
		QCOMPARE(aspect->name(), QLatin1String("Arbeitsblatt")); \
	QCOMPARE(aspect->type() == AspectType::Worksheet, true); \
	auto w = dynamic_cast<Worksheet*>(aspect); \
 \
	auto p1 = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(0)); \
	QCOMPARE(p1 != nullptr, true); \
	auto p2 = dynamic_cast<CartesianPlot*>(aspect->child<CartesianPlot>(1)); \
	QCOMPARE(p2 != nullptr, true); \
\
	auto* view = dynamic_cast<WorksheetView*>(w->view()); \
	QCOMPARE(view != nullptr, true); \
	emit w->useViewSizeRequested(); /* To init the worksheet view actions */\
\
	/* axis selected */ \
	auto sinCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(0)); \
	QCOMPARE(sinCurve != nullptr, true); \
	QCOMPARE(sinCurve->name(), "sinCurve"); \
	auto tanCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(1)); \
	QCOMPARE(tanCurve != nullptr, true); \
	QCOMPARE(tanCurve->name(), "tanCurve"); \
	auto logCurve = dynamic_cast<XYCurve*>(p1->child<XYCurve>(2)); \
	QCOMPARE(logCurve != nullptr, true); \
	QCOMPARE(logCurve->name(), "logx"); \
\
	auto cosCurve = dynamic_cast<XYCurve*>(p2->child<XYCurve>(0)); \
	QCOMPARE(cosCurve != nullptr, true); \
	QCOMPARE(cosCurve->name(), "cosCurve"); \
	\
	auto horAxisP1 = static_cast<Axis*>(p1->child<Axis>(0)); \
	QCOMPARE(horAxisP1 != nullptr, true); \
	QCOMPARE(horAxisP1->orientation() == Axis::Orientation::Horizontal, true); \
	\
	auto vertAxisP1 = static_cast<Axis*>(p1->child<Axis>(1)); \
	QCOMPARE(vertAxisP1 != nullptr, true); \
	QCOMPARE(vertAxisP1->orientation() == Axis::Orientation::Vertical, true);

#define SET_CARTESIAN_MOUSE_MODE(mode) \
	QAction a(nullptr); \
	a.setData(static_cast<int>(mode)); \
	view->cartesianPlotMouseModeChanged(&a);

#define VALUES_EQUAL(v1, v2) QCOMPARE(abs(v1 - v2) <= qMin(0.1*v2, 0.1*v1), true)

#define RANGE_CORRECT(range, start_, end_) \
	VALUES_EQUAL(range.start(), start_); \
	VALUES_EQUAL(range.end(), end_);

#define CHECK_RANGE(plot, aspect, xy, start_, end_) \
	RANGE_CORRECT(plot->xy ## Range(aspect->coordinateSystemIndex()), start_, end_)

// Test1:
// Check if the correct actions are enabled/disabled. Oder schränkt es zu viel ein?

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
 * \brief MultiRangeTest::testApplyActionToSelection_CurveSelected_ZoomSelection
 *
 */
void MultiRangeTest::testApplyActionToSelection_CurveSelected_ZoomSelection()
{
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

void MultiRangeTest::testZoomXSelection_AllRanges()
{
	LOAD_PROJECT
	w->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	horAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -0.5), -1);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 0.3), -1);
	p1->mouseReleaseZoomSelectionMode(-1);

	// TODO
	//CHECK_RANGE(p1, sinCurve, x, 0.2, 0.6);
	//CHECK_RANGE(p1, sinCurve, y, -0.5, 0.3);
	CHECK_RANGE(p1, tanCurve, x, 0.2, 0.6);
	//CHECK_RANGE(p1, tanCurve, y, -250, 250);
}

void MultiRangeTest::testZoomXSelection_SingleRange()
{
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -0.5), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 0.3), 0);
	p1->mouseReleaseZoomSelectionMode(0);

	// TODO
	//CHECK_RANGE(p1, sinCurve, x, 0, 1);
	//CHECK_RANGE(p1, sinCurve, y, -0.5, 0.3);
	//CHECK_RANGE(p1, tanCurve, x, 0, 1);
	//CHECK_RANGE(p1, tanCurve, y, -250, 250);
}

void MultiRangeTest::testZoomYSelection_AllRanges()
{
	LOAD_PROJECT
	w->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	horAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -0.5), -1);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 0.3), -1);
	p1->mouseReleaseZoomSelectionMode(-1);

	// TODO
	//CHECK_RANGE(p1, sinCurve, x, 0, 1);
	//CHECK_RANGE(p1, sinCurve, y, -0.5, 0.3);
	//CHECK_RANGE(p1, tanCurve, x, 0, 1);
	//CHECK_RANGE(p1, tanCurve, y, -0.5, 0.3);
	//CHECK_RANGE(p1, logCurve, x, 0, 100);
	//CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::testZoomYSelection_SingleRange()
{
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -0.5), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 0.3), 0);
	p1->mouseReleaseZoomSelectionMode(0);

	// TODO
	//CHECK_RANGE(p1, sinCurve, x, 0, 1);
	//CHECK_RANGE(p1, sinCurve, y, -0.5, 0.3);
	//CHECK_RANGE(p1, tanCurve, x, 0, 1);
	//CHECK_RANGE(p1, tanCurve, y, -250, 250);
	//CHECK_RANGE(p1, logCurve, x, 0, 100);
	//CHECK_RANGE(p1, logCurve, y, -10, 6);
}

void MultiRangeTest::testZoomSelection_SingleRange()
{
	LOAD_PROJECT
	horAxisP1->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomSelection)

	p1->mousePressZoomSelectionMode(QPointF(0.2, -0.5), 0);
	p1->mouseMoveZoomSelectionMode(QPointF(0.6, 0.3), 0);
	p1->mouseReleaseZoomSelectionMode(0);

	// TODO
	//CHECK_RANGE(p1, sinCurve, x, 0.2, 0.6);
	//CHECK_RANGE(p1, sinCurve, y, -0.5, 0.3);
	CHECK_RANGE(p1, tanCurve, x, 0, 1);
	//CHECK_RANGE(p1, tanCurve, y, -250, 250);
	//CHECK_RANGE(p1, logCurve, x, 0, 100);
	//CHECK_RANGE(p1, logCurve, y, -10, 6);
}

QTEST_MAIN(MultiRangeTest)
