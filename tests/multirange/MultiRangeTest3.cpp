/*
	File                 : MultiRangeTest3.cpp
	Project              : LabPlot
	Description          : Third tests for multi ranges
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MultiRangeTest3.h"
#include "MultiRangeTest_macros.h"

void MultiRangeTest3::baseDockSetAspects_NoPlotRangeChange() {
	LOAD_PROJECT

	const int sinCurveCSystemIndex = sinCurve->coordinateSystemIndex();
	const int tanCurveCSystemIndex = tanCurve->coordinateSystemIndex();
	QVERIFY(sinCurveCSystemIndex != tanCurveCSystemIndex);
	// checks directly the plot. In the basedock the element is used and not the plot, so do it here too
	QVERIFY(sinCurve->coordinateSystemCount() == 3);

	XYCurveDock dock(nullptr);
	dock.setupGeneral();
	dock.setCurves(QList<XYCurve*>({sinCurve, tanCurve}));

	dock.updatePlotRangeList();

	// The coordinatesystem indices shall not change
	QCOMPARE(sinCurveCSystemIndex, sinCurve->coordinateSystemIndex());
	QCOMPARE(tanCurveCSystemIndex, tanCurve->coordinateSystemIndex());
}

/*!
 * \brief MultiRangeTest3::curveRangeChange
 * When changing the coordinatesystem of an object like a curve, the
 * curve shall be updated accordingly also for undo/redo
 */
void MultiRangeTest3::curveRangeChange() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(plot != nullptr);
	ws->addChild(plot);

	auto* curve{new XYEquationCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(plot->defaultCoordinateSystemIndex());
	plot->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.count = 100;
	data.expression1 = QStringLiteral("sin(x*2*pi*3)");
	curve->setEquationData(data);
	curve->recalculate();

	CHECK_RANGE(plot, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, curve, Dimension::Y, -1., 1.);

	// Create new cSystem
	Range<double> yRange;
	yRange.setFormat(RangeT::Format::Numeric);
	yRange.setAutoScale(false);
	yRange.setRange(0, 10);
	plot->addYRange(yRange);
	CartesianCoordinateSystem* cSystem = new CartesianCoordinateSystem(plot);
	cSystem->setIndex(Dimension::X, 0);
	cSystem->setIndex(Dimension::Y, 1);
	plot->addCoordinateSystem(cSystem);

	QCOMPARE(plot->coordinateSystemCount(), 2);
	QCOMPARE(plot->coordinateSystem(1), cSystem);

	CHECK_RANGE(plot, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, curve, Dimension::Y, -1., 1.);

	curve->setCoordinateSystemIndex(1);

	CHECK_RANGE(plot, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, curve, Dimension::Y, 0., 10.);

	curve->undoStack()->undo();

	QCOMPARE(curve->coordinateSystemIndex(), 0);
	CHECK_RANGE(plot, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, curve, Dimension::Y, -1., 1.);

	curve->undoStack()->redo();

	QCOMPARE(curve->coordinateSystemIndex(), 1);
	CHECK_RANGE(plot, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, curve, Dimension::Y, 0., 10.);
}

QTEST_MAIN(MultiRangeTest3)
