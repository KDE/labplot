/*
	File                 : MultiRangeTest2.cpp
	Project              : LabPlot
	Description          : Tests for multi ranges, part 1
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MultiRangeTest2.h"
#include "MultiRangeTest_macros.h"

void MultiRangeTest2::autoScaleYAfterZoomInX() {
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

void MultiRangeTest2::autoScaleXAfterZoomInY() {
	LOAD_PROJECT
	p1->setNiceExtend(true);
	const auto refValues = vertAxisP1->tickLabelValues();
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

	// Revert the zoom
	p1->navigate(tanCurve->coordinateSystemIndex(), CartesianPlot::NavigationOperation::ScaleAutoY);

	// All x ranges are zoomed, for plot 1 and plot 2
	CHECK_RANGE(p1, sinCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::X, 0., 1.);
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.); // autoscaled
	CHECK_RANGE(p1, logCurve, Dimension::X, 0., 100.);
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.);
}

/*!
 * \brief MultiRangeTest2::mouseWheelXAxisApplyToAllX
 * If applied to all x is activated, using the mousewheel on a
 * selected axis should also execute the mousewheel on other plots
 */
void MultiRangeTest2::mouseWheelXAxisApplyToAllX() {
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
 * \brief MultiRangeTest2::mouseWheelXAxisApplyToAllX
 * If applied to all x is activated, using the mousewheel on a
 * selected axis should also execute the mousewheel on other plots
 * This time the second x axis is used. In the second plot no second x axis is used
 * so check that application does not crash
 */
void MultiRangeTest2::mouseWheelTanCurveApplyToAllX() {
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

void MultiRangeTest2::mouseWheelXAxisApplyToSelected() {
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

void MultiRangeTest2::axisMouseMoveApplyToAllX() {
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

void MultiRangeTest2::axisMouseMoveApplyToSelection() {
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

QTEST_MAIN(MultiRangeTest2)
