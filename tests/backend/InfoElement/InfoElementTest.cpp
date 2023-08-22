/*
	File                 : InfoElementTest.cpp
	Project              : LabPlot
	Description          : Tests for InfoElement
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "InfoElementTest.h"
#include "backend/core/Project.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"

void InfoElementTest::addPlot() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

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

	auto* curve2{new XYEquationCurve(QStringLiteral("f(x^2)"))};
	curve2->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve2);

	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.count = 11;
	data.expression1 = QStringLiteral("x^2");
	curve2->setEquationData(data);
	curve2->recalculate();

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 100.);

	auto* ie = new InfoElement(QStringLiteral("InfoElement"), p, curve, 4.9);
	QVERIFY(ie != nullptr);
	p->addChild(ie);

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(ie->markerPointsCount(), 1);
		QCOMPARE(ie->markerPointAt(0).curve, curve);
		QCOMPARE(points.count(), 1);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
	}

	ie->addCurve(curve2);

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(points.count(), 2);

		QCOMPARE(ie->markerPointsCount(), 2);
		QCOMPARE(ie->markerPointAt(0).curve, curve);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
		QCOMPARE(ie->markerPointAt(1).curve, curve2);
		QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
		QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
	}
}

void InfoElementTest::addRemoveCurve() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

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

	auto* curve2{new XYEquationCurve(QStringLiteral("f(x^2)"))};
	curve2->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve2);

	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.count = 11;
	data.expression1 = QStringLiteral("x^2");
	curve2->setEquationData(data);
	curve2->recalculate();

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 100.);

	auto* ie = new InfoElement(QStringLiteral("InfoElement"), p, curve, 4.9);
	QVERIFY(ie != nullptr);
	p->addChild(ie);

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(ie->markerPointsCount(), 1);
		QCOMPARE(ie->markerPointAt(0).curve, curve);
		QCOMPARE(points.count(), 1);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
	}
	QCOMPARE(ie->connectionLineCurveName(), curve->name());

	ie->addCurve(curve2);

	QCOMPARE(ie->connectionLineCurveName(), curve->name()); // No change here

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(points.count(), 2);

		QCOMPARE(ie->markerPointsCount(), 2);
		QCOMPARE(ie->markerPointAt(0).curve, curve);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
		QCOMPARE(ie->markerPointAt(1).curve, curve2);
		QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
		QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
	}

	ie->removeCurve(curve);

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(points.count(), 1);

		QCOMPARE(ie->markerPointsCount(), 1);
		QCOMPARE(ie->markerPointAt(0).curve, curve2);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 25));
	}

	QCOMPARE(ie->connectionLineCurveName(), curve2->name());
}

QTEST_MAIN(InfoElementTest)
