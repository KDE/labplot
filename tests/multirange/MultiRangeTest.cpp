/*
	File                 : MultiRangeTest.cpp
	Project              : LabPlot
	Description          : Tests for multi ranges, part 1
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MultiRangeTest.h"
#include "MultiRangeTest_macros.h"

// ##############################################################################
// #####################  import of LabPlot projects ############################
// ##############################################################################

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
	p1->setNiceExtend(true);
	const auto refValuesAxis1 = vertAxisP1->tickLabelValues();
	const auto refValuesAxis2 = vertAxis2P1->tickLabelValues();
	const auto refValuesAxis3 = vertAxis3P1->tickLabelValues();
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
	CHECK_RANGE(p1, sinCurve, Dimension::Y, -1., 1.); // autoscale
	CHECK_RANGE(p1, tanCurve, Dimension::X, .2, .6); // zoom
	CHECK_RANGE(p1, tanCurve, Dimension::Y, -250., 250.); // autoscale
	CHECK_RANGE(p1, logCurve, Dimension::X, 20., 60.); // zoom
	CHECK_RANGE(p1, logCurve, Dimension::Y, -10., 6.); // No niceExtends() done!
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
}

void MultiRangeTest::zoomInX_SingleRangeDateTimeMonotonicIncrease() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* view = dynamic_cast<WorksheetView*>(ws->view());
	QVERIFY(view != nullptr);
	view->initActions(); // needed by SET_CARTESIAN_MOUSE_MODE()

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);
	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	QDateTime dt1 = QDateTime::fromString(QStringLiteral("2017-07-10T00:00:00Z"), Qt::ISODate);
	QDateTime dt2 = QDateTime::fromString(QStringLiteral("2017-07-11T00:00:00Z"), Qt::ISODate);
	QDateTime dt3 = QDateTime::fromString(QStringLiteral("2017-07-12T00:00:00Z"), Qt::ISODate);
	QDateTime dt4 = QDateTime::fromString(QStringLiteral("2017-07-13T00:00:00Z"), Qt::ISODate);
	QDateTime dt5 = QDateTime::fromString(QStringLiteral("2017-07-14T00:00:00Z"), Qt::ISODate);
	QDateTime dt6 = QDateTime::fromString(QStringLiteral("2017-07-15T00:00:00Z"), Qt::ISODate);
	QDateTime dt7 = QDateTime::fromString(QStringLiteral("2017-07-16T00:00:00Z"), Qt::ISODate);
	QDateTime dt8 = QDateTime::fromString(QStringLiteral("2017-07-17T00:00:00Z"), Qt::ISODate);
	QDateTime dt9 = QDateTime::fromString(QStringLiteral("2017-07-18T00:00:00Z"), Qt::ISODate);
	QDateTime dt10 = QDateTime::fromString(QStringLiteral("2017-07-19T00:00:00Z"), Qt::ISODate);
	xCol->replaceDateTimes(-1, QVector<QDateTime>({dt1, dt2, dt3, dt4, dt5, dt6, dt7, dt8, dt9, dt10}));
	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4., 5., 6., 7., 8., 9., 10., 11.}));

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	p->setNiceExtend(false);
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("curve"));
	p->addChild(curve);
	curve->setXColumn(xCol);
	curve->setYColumn(yCol);

	CHECK_RANGE(p, curve, Dimension::X, (double)dt1.toMSecsSinceEpoch(), (double)dt10.toMSecsSinceEpoch());
	CHECK_RANGE(p, curve, Dimension::Y, 2., 11.);

	QCOMPARE(p->rangeCount(Dimension::X), 1);
	QCOMPARE(p->rangeCount(Dimension::Y), 1);
	QCOMPARE(p->range(Dimension::Y, 0).autoScale(), true);

	const auto& axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	axes.at(0)->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	p->mousePressZoomSelectionMode(QPointF((double)dt3.toMSecsSinceEpoch(), 3.), 0);
	p->mouseMoveZoomSelectionMode(QPointF((double)dt5.toMSecsSinceEpoch(), 3.), 0);
	p->mouseReleaseZoomSelectionMode(0);

	CHECK_RANGE(p, curve, Dimension::X, (double)dt3.toMSecsSinceEpoch(), (double)dt5.toMSecsSinceEpoch()); // zoom
	CHECK_RANGE(p, curve, Dimension::Y, 4., 6.); // autoscaled
}

void MultiRangeTest::zoomInX_SingleRangeDateTimeNonMonotonic() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* view = dynamic_cast<WorksheetView*>(ws->view());
	QVERIFY(view != nullptr);
	view->initActions(); // needed by SET_CARTESIAN_MOUSE_MODE()

	Spreadsheet* spreadsheetData = new Spreadsheet(QStringLiteral("data"), false);
	spreadsheetData->setColumnCount(2);
	spreadsheetData->setRowCount(3);
	project.addChild(spreadsheetData);
	auto* xCol = spreadsheetData->column(0);
	xCol->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	QDateTime dt1 = QDateTime::fromString(QStringLiteral("2017-07-10T00:00:00Z"), Qt::ISODate);
	QDateTime dt2 = QDateTime::fromString(QStringLiteral("2017-07-11T00:00:00Z"), Qt::ISODate);
	QDateTime dt3 = QDateTime::fromString(QStringLiteral("2017-07-12T00:00:00Z"), Qt::ISODate);
	QDateTime dt4 = QDateTime::fromString(QStringLiteral("2017-07-15T00:00:00Z"), Qt::ISODate);
	QDateTime dt5 = QDateTime::fromString(QStringLiteral("2017-07-14T00:00:00Z"), Qt::ISODate); // Nonmonoton
	QDateTime dt6 = QDateTime::fromString(QStringLiteral("2017-07-15T00:00:00Z"), Qt::ISODate);
	QDateTime dt7 = QDateTime::fromString(QStringLiteral("2017-07-16T00:00:00Z"), Qt::ISODate);
	QDateTime dt8 = QDateTime::fromString(QStringLiteral("2017-07-17T00:00:00Z"), Qt::ISODate);
	QDateTime dt9 = QDateTime::fromString(QStringLiteral("2017-07-18T00:00:00Z"), Qt::ISODate);
	QDateTime dt10 = QDateTime::fromString(QStringLiteral("2017-07-19T00:00:00Z"), Qt::ISODate);
	xCol->replaceDateTimes(-1, QVector<QDateTime>({dt1, dt2, dt3, dt4, dt5, dt6, dt7, dt8, dt9, dt10}));
	auto* yCol = spreadsheetData->column(1);
	yCol->replaceValues(-1, QVector<double>({2., 3., 4., 5., 6., 7., 8., 9., 10., 11.}));

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	p->setNiceExtend(false);
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve = new XYCurve(QStringLiteral("curve"));
	p->addChild(curve);
	curve->setXColumn(xCol);
	curve->setYColumn(yCol);

	CHECK_RANGE(p, curve, Dimension::X, (double)dt1.toMSecsSinceEpoch(), (double)dt10.toMSecsSinceEpoch());
	CHECK_RANGE(p, curve, Dimension::Y, 2., 11.);

	QCOMPARE(p->rangeCount(Dimension::X), 1);
	QCOMPARE(p->rangeCount(Dimension::Y), 1);
	QCOMPARE(p->range(Dimension::Y, 0).autoScale(), true);

	const auto& axes = p->children<Axis>();
	QCOMPARE(axes.count(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	axes.at(0)->setSelected(true);
	SET_CARTESIAN_MOUSE_MODE(CartesianPlot::MouseMode::ZoomXSelection)

	p->mousePressZoomSelectionMode(QPointF((double)dt3.toMSecsSinceEpoch(), 3.), 0);
	p->mouseMoveZoomSelectionMode(QPointF((double)dt5.addSecs(3600 * 2).toMSecsSinceEpoch(), 3.),
								  0); // Adding an offset, because the error happens only if the exact time is not hit
	p->mouseReleaseZoomSelectionMode(0);

	CHECK_RANGE(p, curve, Dimension::X, (double)dt3.toMSecsSinceEpoch(), (double)dt5.addSecs(3600 * 2).toMSecsSinceEpoch()); // zoom
	CHECK_RANGE(p, curve, Dimension::Y, 4., 6.); // autoscaled
}

void MultiRangeTest::zoomInX_AllRanges() {
	LOAD_PROJECT

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
}

void MultiRangeTest::zoomInY_SingleRange() {
	LOAD_PROJECT

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
}

void MultiRangeTest::zoomInY_AllRanges() {
	LOAD_PROJECT

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
}

void MultiRangeTest::zoomOutX_SingleRange() {
	LOAD_PROJECT

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
}

void MultiRangeTest::zoomOutX_AllRanges() {
	LOAD_PROJECT

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
}

void MultiRangeTest::zoomOutY_SingleRange() {
	LOAD_PROJECT

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
}

void MultiRangeTest::zoomOutY_AllRanges() {
	LOAD_PROJECT
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
}

// SHIFT

void MultiRangeTest::shiftLeft_SingleRange() {
	LOAD_PROJECT
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
}

void MultiRangeTest::shiftRight_SingleRange() {
	LOAD_PROJECT
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
}

void MultiRangeTest::shiftLeft_AllRanges() {
	LOAD_PROJECT
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
}

void MultiRangeTest::shiftRight_AllRanges() {
	LOAD_PROJECT

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
}

void MultiRangeTest::shiftUp_SingleRange() {
	LOAD_PROJECT

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
}

void MultiRangeTest::shiftDown_SingleRange() {
	LOAD_PROJECT

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
}

void MultiRangeTest::shiftUp_AllRanges() {
	LOAD_PROJECT
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
}

void MultiRangeTest::shiftDown_AllRanges() {
	LOAD_PROJECT
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
}

QTEST_MAIN(MultiRangeTest)
