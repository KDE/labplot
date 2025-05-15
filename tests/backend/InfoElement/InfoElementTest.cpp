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
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"

#include <QUndoStack>

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

void InfoElementTest::removeCurve() {
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

	// add/removeCurve from Infoelement currently not an undocommand
	// project.undoStack()->undo();

	// QCOMPARE(ie->connectionLineCurveName(), curve2->name()); // first curve shall be connected again
	// // QCOMPARE(ie->connectionLineCurveName(), curve->name()); // Not implememented yet, quite difficult
	// {
	// 	const auto points = ie->children<CustomPoint>();
	// 	QCOMPARE(points.count(), 2);

	// 	QCOMPARE(ie->markerPointsCount(), 2);
	// 	QCOMPARE(ie->markerPointAt(0).curve, curve);
	// 	QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
	// 	QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
	// 	QCOMPARE(ie->markerPointAt(1).curve, curve2);
	// 	QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
	// 	QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
	// }
}

void InfoElementTest::removeColumn() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());

	auto* spreadsheet = new Spreadsheet(QStringLiteral("spreadsheet"));
	spreadsheet->setRowCount(11);
	project.addChild(spreadsheet);
	const auto& columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);

	for (int i = 0; i < spreadsheet->rowCount(); i++) {
		const double value = i;
		xColumn->setValueAt(i, value);
		yColumn->setValueAt(i, value);
	}

	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);
	p->addChild(curve);

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 10.);

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

	// InfoElement is invalid
	const auto& labels = ie->children<TextLabel>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	const auto& points = ie->children<CustomPoint>();
	QCOMPARE(labels.count(), 1);
	QCOMPARE(points.count(), 1);

	QCOMPARE(ie->isValid(), true);
	QCOMPARE(labels.at(0)->isVisible(), true);
	QCOMPARE(points.at(0)->isVisible(), true);

	spreadsheet->removeChild(yColumn);

	QCOMPARE(ie->isValid(), false);
	QCOMPARE(labels.at(0)->isVisible(), false);
	QCOMPARE(points.at(0)->isVisible(), false);

	spreadsheet->addChild(yColumn);

	// InfoElement is valid
	QCOMPARE(ie->isValid(), true);
	QCOMPARE(labels.at(0)->isVisible(), true);
	QCOMPARE(points.at(0)->isVisible(), true);
}

void InfoElementTest::changeColumn() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());

	auto* spreadsheet = new Spreadsheet(QStringLiteral("spreadsheet"));
	spreadsheet->setColumnCount(3);
	spreadsheet->setRowCount(11);
	project.addChild(spreadsheet);
	const auto& columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 3);
	QCOMPARE(spreadsheet->rowCount(), 11);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);
	auto* y2Column = columns.at(2);

	for (int i = 0; i < spreadsheet->rowCount(); i++) {
		xColumn->setValueAt(i, i);
		yColumn->setValueAt(i, i);
		y2Column->setValueAt(i, i + 1);
	}

	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);
	p->addChild(curve);

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 10.);

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

	// InfoElement is invalid
	const auto& labels = ie->children<TextLabel>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	const auto& points = ie->children<CustomPoint>();
	QCOMPARE(labels.count(), 1);
	QCOMPARE(points.count(), 1);

	QCOMPARE(ie->isValid(), true);
	QCOMPARE(labels.at(0)->isVisible(), true);
	QCOMPARE(points.at(0)->isVisible(), true);

	curve->setYColumn(y2Column);

	QCOMPARE(ie->isValid(), true);
	QCOMPARE(labels.at(0)->isVisible(), true);
	QCOMPARE(points.at(0)->isVisible(), true);

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(ie->markerPointsCount(), 1);
		QCOMPARE(ie->markerPointAt(0).curve, curve);
		QCOMPARE(points.count(), 1);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 6)); // y2 = i + 1
		QCOMPARE(ie->markerPointAt(0).visible, true);
		QCOMPARE(points.at(0)->isVisible(), true);
	}
}

void InfoElementTest::columnValueChanged() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());

	auto* spreadsheet = new Spreadsheet(QStringLiteral("spreadsheet"));
	spreadsheet->setColumnCount(2);
	spreadsheet->setRowCount(11);
	project.addChild(spreadsheet);
	const auto& columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);
	QCOMPARE(spreadsheet->rowCount(), 11);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);

	for (int i = 0; i < spreadsheet->rowCount(); i++) {
		xColumn->setValueAt(i, i);
		yColumn->setValueAt(i, i);
	}

	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);
	p->addChild(curve);

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 10.);

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

	// InfoElement is invalid
	const auto& labels = ie->children<TextLabel>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	const auto& points = ie->children<CustomPoint>();
	QCOMPARE(labels.count(), 1);
	QCOMPARE(points.count(), 1);

	QCOMPARE(ie->isValid(), true);
	QCOMPARE(labels.at(0)->isVisible(), true);
	QCOMPARE(points.at(0)->isVisible(), true);

	yColumn->setValueAt(5, 10.);

	QCOMPARE(ie->isValid(), true);
	QCOMPARE(labels.at(0)->isVisible(), true);
	QCOMPARE(points.at(0)->isVisible(), true);

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(ie->markerPointsCount(), 1);
		QCOMPARE(ie->markerPointAt(0).curve, curve);
		QCOMPARE(points.count(), 1);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 10));
		QCOMPARE(ie->markerPointAt(0).visible, true);
		QCOMPARE(points.at(0)->isVisible(), true);
	}
}

void InfoElementTest::deleteCurveRenameAddedAutomatically() {
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
		QCOMPARE(ie->markerPointAt(0).visible, true);
		QCOMPARE(points.at(0)->isVisible(), true);
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
		QCOMPARE(ie->markerPointAt(0).visible, true);
		QCOMPARE(points.at(0)->isVisible(), true);

		QCOMPARE(ie->markerPointAt(1).curve, curve2);
		QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
		QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
		QCOMPARE(ie->markerPointAt(1).visible, true);
		QCOMPARE(points.at(1)->isVisible(), true);
	}

	p->removeChild(curve);

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(points.count(), 2);

		QCOMPARE(ie->markerPointsCount(), 2);
		QCOMPARE(ie->markerPointAt(0).curve, nullptr);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
		QCOMPARE(ie->markerPointAt(0).visible, true);
		QCOMPARE(points.at(0)->isVisible(), false);

		QCOMPARE(ie->markerPointAt(1).curve, curve2);
		QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
		QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
		QCOMPARE(ie->markerPointAt(1).visible, true);
		QCOMPARE(points.at(1)->isVisible(), true);
	}

	QCOMPARE(ie->connectionLineCurveName(), curve2->name());

	const QString oldName = curve->name();

	curve->setName(QStringLiteral("new name for curve"));
	p->addChild(curve);

	QCOMPARE(ie->connectionLineCurveName(), curve2->name());

	// rename
	curve->setName(oldName);

	QCOMPARE(ie->connectionLineCurveName(), curve2->name());
	// QCOMPARE(ie->connectionLineCurveName(), curve->name()); // Not implemented yet, because quite complicated

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(points.count(), 2);

		QCOMPARE(ie->markerPointsCount(), 2);
		QCOMPARE(ie->markerPointAt(0).curve, curve);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
		QCOMPARE(ie->markerPointAt(0).visible, true);
		QCOMPARE(points.at(0)->isVisible(), true);

		QCOMPARE(ie->markerPointAt(1).curve, curve2);
		QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
		QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
		QCOMPARE(ie->markerPointAt(1).visible, true);
		QCOMPARE(points.at(1)->isVisible(), true);
	}
}

void InfoElementTest::deleteCurveRenameAddedAutomaticallyCustomPointInvisible() {
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
		QCOMPARE(ie->markerPointAt(0).visible, true);
		QCOMPARE(points.at(0)->isVisible(), true);
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
		QCOMPARE(ie->markerPointAt(0).visible, true);
		QCOMPARE(points.at(0)->isVisible(), true);

		QCOMPARE(ie->markerPointAt(1).curve, curve2);
		QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
		QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
		QCOMPARE(ie->markerPointAt(1).visible, true);
		QCOMPARE(points.at(1)->isVisible(), true);
	}

	{
		const auto points = ie->children<CustomPoint>();
		points.at(0)->setVisible(false);
	}

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(points.count(), 2);

		QCOMPARE(ie->markerPointsCount(), 2);
		QCOMPARE(ie->markerPointAt(0).curve, curve);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
		QCOMPARE(ie->markerPointAt(0).visible, false);
		QCOMPARE(points.at(0)->isVisible(), false);

		QCOMPARE(ie->markerPointAt(1).curve, curve2);
		QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
		QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
		QCOMPARE(ie->markerPointAt(1).visible, true);
		QCOMPARE(points.at(1)->isVisible(), true);
	}

	p->removeChild(curve);

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(points.count(), 2);

		QCOMPARE(ie->markerPointsCount(), 2);
		QCOMPARE(ie->markerPointAt(0).curve, nullptr);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
		QCOMPARE(ie->markerPointAt(0).visible, false);
		QCOMPARE(points.at(0)->isVisible(), false);

		QCOMPARE(ie->markerPointAt(1).curve, curve2);
		QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
		QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
		QCOMPARE(ie->markerPointAt(1).visible, true);
		QCOMPARE(points.at(1)->isVisible(), true);
	}

	QCOMPARE(ie->connectionLineCurveName(), curve2->name());

	const QString oldName = curve->name();
	curve->setName(QStringLiteral("new name for curve"));
	p->addChild(curve);

	QCOMPARE(ie->connectionLineCurveName(), curve2->name());

	// rename
	curve->setName(oldName);

	QCOMPARE(ie->connectionLineCurveName(), curve2->name()); // stays on curve 2

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(points.count(), 2);

		QCOMPARE(ie->markerPointsCount(), 2);
		QCOMPARE(ie->markerPointAt(0).curve, curve);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
		QCOMPARE(ie->markerPointAt(0).visible, false);
		QCOMPARE(points.at(0)->isVisible(), false);

		QCOMPARE(ie->markerPointAt(1).curve, curve2);
		QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
		QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
		QCOMPARE(ie->markerPointAt(1).visible, true);
		QCOMPARE(points.at(1)->isVisible(), true);
	}
}

void InfoElementTest::addRemoveRenameColumn() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());

	auto* spreadsheet = new Spreadsheet(QStringLiteral("spreadsheet"));
	spreadsheet->setRowCount(11);
	project.addChild(spreadsheet);
	const auto& columns = spreadsheet->children<Column>();
	QCOMPARE(columns.count(), 2);

	auto* xColumn = columns.at(0);
	auto* yColumn = columns.at(1);

	for (int i = 0; i < spreadsheet->rowCount(); i++) {
		const double value = i;
		xColumn->setValueAt(i, value);
		yColumn->setValueAt(i, value);
	}

	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);
	p->addChild(curve);

	CHECK_RANGE(p, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(p, curve, Dimension::Y, 0., 10.);

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

	spreadsheet->removeChild(yColumn);

	// InfoElement is invalid
	const auto& labels = ie->children<TextLabel>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	const auto& points = ie->children<CustomPoint>();
	QCOMPARE(labels.count(), 1);
	QCOMPARE(points.count(), 1);

	QCOMPARE(ie->isValid(), false);
	QCOMPARE(labels.at(0)->isVisible(), false);
	QCOMPARE(ie->markerPointAt(0).visible, true);
	QCOMPARE(points.at(0)->isVisible(), false);

	const QString oldName = yColumn->name();
	yColumn->setName(QStringLiteral("New name for column"));

	spreadsheet->addChild(yColumn);

	QCOMPARE(ie->isValid(), false);
	QCOMPARE(labels.at(0)->isVisible(), false);
	QCOMPARE(points.at(0)->isVisible(), false);

	yColumn->setName(oldName);

	// InfoElement is valid
	QCOMPARE(ie->isValid(), true);
	QCOMPARE(labels.at(0)->isVisible(), true);
	QCOMPARE(points.at(0)->isVisible(), true);
}

void InfoElementTest::moveDuringMissingCurve() {
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

	auto* curve3{new XYEquationCurve(QStringLiteral("New curve"))};
	curve3->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve3);
	data.min = QStringLiteral("0");
	data.max = QStringLiteral("10");
	data.count = 11;
	data.expression1 = QStringLiteral("x^2");
	curve3->setEquationData(data);
	curve3->recalculate();

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

	p->removeChild(curve);

	QCOMPARE(ie->isValid(), false);
	ie->setPositionLogical(7); // No crash!

	{
		const auto points = ie->children<CustomPoint>();
		QCOMPARE(points.count(), 1);

		QCOMPARE(ie->markerPointsCount(), 1);
		QCOMPARE(ie->markerPointAt(0).curve, nullptr);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
	}
}

void InfoElementTest::moveCurve() {
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

	curve->moveDown();

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

	QCOMPARE(ie->connectionLineCurveName(), curve->name());
}

void InfoElementTest::saveLoad() {
	QString savePath;
	{
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
		ie->addCurve(curve2);
		{
			const auto points = ie->children<CustomPoint>();
			QCOMPARE(ie->markerPointsCount(), 2);
			QCOMPARE(points.count(), 2);
			QCOMPARE(ie->markerPointAt(0).curve, curve);
			QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
			QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
			QCOMPARE(points.at(0)->isVisible(), true);

			QCOMPARE(ie->markerPointAt(1).curve, curve2);
			QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
			QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
			QCOMPARE(points.at(1)->isVisible(), true);
		}

		QCOMPARE(ie->connectionLineCurveName(), curve->name());
		SAVE_PROJECT("TestInfoElementSaveLoad.lml");
	}

	{
		Project project;
		QCOMPARE(project.load(savePath), true);

		const auto* ws = project.child<Worksheet>(0);
		QVERIFY(ws);
		const auto* p = ws->child<CartesianPlot>(0);
		QVERIFY(p);
		const auto* ie = p->child<InfoElement>(0);
		QVERIFY(ie);

		const auto* curve1 = p->child<XYCurve>(0);
		QCOMPARE(curve1->name(), QStringLiteral("f(x)"));
		const auto* curve2 = p->child<XYCurve>(1);
		QCOMPARE(curve2->name(), QStringLiteral("f(x^2)"));

		const auto points = ie->children<CustomPoint>();
		QCOMPARE(ie->markerPointsCount(), 2);
		QCOMPARE(points.count(), 2);
		QCOMPARE(ie->markerPointAt(0).curve, curve1);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
		QCOMPARE(points.at(0)->isVisible(), true);

		QCOMPARE(ie->markerPointAt(1).curve, curve2);
		QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
		QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
		QCOMPARE(points.at(1)->isVisible(), true);

		QCOMPARE(ie->connectionLineCurveName(), QStringLiteral("f(x)"));
	}
}

void InfoElementTest::saveLoadInvisiblePoint() {
	QString savePath;
	{
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
		ie->addCurve(curve2);
		{
			const auto points = ie->children<CustomPoint>();
			points.at(0)->setVisible(false); // Set the custom point invisible. So the line just points to the position
			QCOMPARE(ie->markerPointsCount(), 2);
			QCOMPARE(points.count(), 2);
			QCOMPARE(ie->markerPointAt(0).curve, curve);
			QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
			QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
			QCOMPARE(points.at(0)->isVisible(), false);

			QCOMPARE(ie->markerPointAt(1).curve, curve2);
			QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
			QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
			QCOMPARE(points.at(1)->isVisible(), true);
		}

		QCOMPARE(ie->connectionLineCurveName(), curve->name());
		SAVE_PROJECT("TestInfoElementSaveLoadCustomPointInvisible.lml");
	}

	{
		Project project;
		QCOMPARE(project.load(savePath), true);

		const auto* ws = project.child<Worksheet>(0);
		QVERIFY(ws);
		const auto* p = ws->child<CartesianPlot>(0);
		QVERIFY(p);
		const auto* ie = p->child<InfoElement>(0);
		QVERIFY(ie);

		const auto* curve1 = p->child<XYCurve>(0);
		QCOMPARE(curve1->name(), QStringLiteral("f(x)"));
		const auto* curve2 = p->child<XYCurve>(1);
		QCOMPARE(curve2->name(), QStringLiteral("f(x^2)"));

		const auto points = ie->children<CustomPoint>();
		QCOMPARE(ie->markerPointsCount(), 2);
		QCOMPARE(points.count(), 2);
		QCOMPARE(ie->markerPointAt(0).curve, curve1);
		QCOMPARE(ie->markerPointAt(0).customPoint, points.at(0));
		QCOMPARE(points.at(0)->positionLogical(), QPointF(5, 5));
		QCOMPARE(points.at(0)->isVisible(), false);

		QCOMPARE(ie->markerPointAt(1).curve, curve2);
		QCOMPARE(ie->markerPointAt(1).customPoint, points.at(1));
		QCOMPARE(points.at(1)->positionLogical(), QPointF(5, 25));
		QCOMPARE(points.at(1)->isVisible(), true);

		QCOMPARE(ie->connectionLineCurveName(), QStringLiteral("f(x)"));
	}
}

QTEST_MAIN(InfoElementTest)
