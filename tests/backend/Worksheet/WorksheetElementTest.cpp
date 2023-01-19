/*
	File                 : WorksheetTest.cpp
	Project              : LabPlot
	Description          : Tests for Worksheets and positioning them on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "WorksheetElementTest.h"
#include "backend/core/Project.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/TextLabelPrivate.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "kdefrontend/widgets/LabelWidget.h"

void WorksheetElementTest::customPointSetPositionLogical() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	p->setHorizontalPadding(0);
	p->setVerticalPadding(0);
	p->setRightPadding(0);
	p->setBottomPadding(0);
	p->setRect(QRectF(0, 0, 100, 100));

	QCOMPARE(p->rangeCount(Dimension::X), 1);
	QCOMPARE(p->range(Dimension::X, 0).start(), 0);
	QCOMPARE(p->range(Dimension::X, 0).end(), 1);
	QCOMPARE(p->rangeCount(Dimension::Y), 1);
	QCOMPARE(p->range(Dimension::Y, 0).start(), 0);
	QCOMPARE(p->range(Dimension::Y, 0).end(), 1);

	QCOMPARE(p->dataRect().x(), -50);
	QCOMPARE(p->dataRect().y(), -50);
	QCOMPARE(p->dataRect().width(), 100);
	QCOMPARE(p->dataRect().height(), 100);

	auto* point = new CustomPoint(p, QStringLiteral("point"));
	p->addChild(point);
	point->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	auto pp = point->position();
	pp.point = QPointF(0, 0);
	point->setPosition(pp);
	point->setCoordinateBindingEnabled(true);

	QCOMPARE(point->positionLogical().x(), 0.5);
	QCOMPARE(point->positionLogical().y(), 0.5);

	point->setPositionLogical(QPointF(1, 1));
	QCOMPARE(point->positionLogical().x(), 1);
	QCOMPARE(point->positionLogical().y(), 1);
	QCOMPARE(point->position().point.x(), 50);
	QCOMPARE(point->position().point.y(), 50);
}

void WorksheetElementTest::customPointMouseMove() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	p->setHorizontalPadding(0);
	p->setVerticalPadding(0);
	p->setRightPadding(0);
	p->setBottomPadding(0);
	p->setRect(QRectF(0, 0, 100, 100));

	QCOMPARE(p->rangeCount(Dimension::X), 1);
	QCOMPARE(p->range(Dimension::X, 0).start(), 0);
	QCOMPARE(p->range(Dimension::X, 0).end(), 1);
	QCOMPARE(p->rangeCount(Dimension::Y), 1);
	QCOMPARE(p->range(Dimension::Y, 0).start(), 0);
	QCOMPARE(p->range(Dimension::Y, 0).end(), 1);

	QCOMPARE(p->dataRect().x(), -50);
	QCOMPARE(p->dataRect().y(), -50);
	QCOMPARE(p->dataRect().width(), 100);
	QCOMPARE(p->dataRect().height(), 100);

	auto* point = new CustomPoint(p, QStringLiteral("point"));
	p->addChild(point);
	point->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	auto pp = point->position();
	pp.point = QPointF(0, 0);
	point->setPosition(pp);
	point->setCoordinateBindingEnabled(true);

	QCOMPARE(point->positionLogical().x(), 0.5);
	QCOMPARE(point->positionLogical().y(), 0.5);

	// Simulate mouse move
	point->d_ptr->setPos(QPointF(25, -10)); // item change will be called (negative value is up)
	QCOMPARE(point->positionLogical().x(), 0.75); // 25/50 * 0.5 + 0.5
	QCOMPARE(point->positionLogical().y(), 0.6);

	point->setPositionLogical(QPointF(0.3, 0.1));
	QCOMPARE(point->positionLogical().x(), 0.3);
	QCOMPARE(point->positionLogical().y(), 0.1);
	QCOMPARE(point->position().point.x(), -20);
	QCOMPARE(point->position().point.y(), -40);
}

void WorksheetElementTest::customPointKeyPressMove() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	p->setHorizontalPadding(0);
	p->setVerticalPadding(0);
	p->setRightPadding(0);
	p->setBottomPadding(0);
	p->setRect(QRectF(0, 0, 100, 100));

	QCOMPARE(p->rangeCount(Dimension::X), 1);
	QCOMPARE(p->range(Dimension::X, 0).start(), 0);
	QCOMPARE(p->range(Dimension::X, 0).end(), 1);
	QCOMPARE(p->rangeCount(Dimension::Y), 1);
	QCOMPARE(p->range(Dimension::Y, 0).start(), 0);
	QCOMPARE(p->range(Dimension::Y, 0).end(), 1);

	QCOMPARE(p->dataRect().x(), -50);
	QCOMPARE(p->dataRect().y(), -50);
	QCOMPARE(p->dataRect().width(), 100);
	QCOMPARE(p->dataRect().height(), 100);

	auto* point = new CustomPoint(p, QStringLiteral("point"));
	p->addChild(point);
	point->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	auto pp = point->position();
	pp.point = QPointF(0, 0);
	point->setPosition(pp);
	point->setCoordinateBindingEnabled(true);

	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Right, Qt::KeyboardModifier::NoModifier);
	point->d_ptr->keyPressEvent(&event);

	VALUES_EQUAL(point->position().point.x(), 5);
	VALUES_EQUAL(point->position().point.y(), 0);
	VALUES_EQUAL(point->positionLogical().x(), 0.55);
	VALUES_EQUAL(point->positionLogical().y(), 0.5);
}

// Switching between setCoordinateBindingEnabled true and false should not move the point
void WorksheetElementTest::customPointEnableDisableCoordBinding() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	p->setHorizontalPadding(0);
	p->setVerticalPadding(0);
	p->setRightPadding(0);
	p->setBottomPadding(0);
	p->setRect(QRectF(0, 0, 100, 100));

	QCOMPARE(p->rangeCount(Dimension::X), 1);
	QCOMPARE(p->range(Dimension::X, 0).start(), 0);
	QCOMPARE(p->range(Dimension::X, 0).end(), 1);
	QCOMPARE(p->rangeCount(Dimension::Y), 1);
	QCOMPARE(p->range(Dimension::Y, 0).start(), 0);
	QCOMPARE(p->range(Dimension::Y, 0).end(), 1);

	QCOMPARE(p->dataRect().x(), -50);
	QCOMPARE(p->dataRect().y(), -50);
	QCOMPARE(p->dataRect().width(), 100);
	QCOMPARE(p->dataRect().height(), 100);

	auto* point = new CustomPoint(p, QStringLiteral("point"));
	p->addChild(point);
	point->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	auto pp = point->position();
	pp.point = QPointF(0, 0);
	point->setPosition(pp);

	VALUES_EQUAL(point->position().point.x(), 0);
	VALUES_EQUAL(point->position().point.y(), 0);
	VALUES_EQUAL(point->positionLogical().x(), 0.5);
	VALUES_EQUAL(point->positionLogical().y(), 0.5);

	point->setCoordinateBindingEnabled(true);

	VALUES_EQUAL(point->position().point.x(), 0);
	VALUES_EQUAL(point->position().point.y(), 0);
	VALUES_EQUAL(point->positionLogical().x(), 0.5);
	VALUES_EQUAL(point->positionLogical().y(), 0.5);

	// Set position to another than the origin
	point->setPositionLogical(QPointF(0.85, 0.7));

	VALUES_EQUAL(point->position().point.x(), 35);
	VALUES_EQUAL(point->position().point.y(), 20);
	VALUES_EQUAL(point->positionLogical().x(), 0.85);
	VALUES_EQUAL(point->positionLogical().y(), 0.7);

	point->setCoordinateBindingEnabled(false);

	VALUES_EQUAL(point->position().point.x(), 35);
	VALUES_EQUAL(point->position().point.y(), 20);
	VALUES_EQUAL(point->positionLogical().x(), 0.85);
	VALUES_EQUAL(point->positionLogical().y(), 0.7);

	point->setCoordinateBindingEnabled(true);

	VALUES_EQUAL(point->position().point.x(), 35);
	VALUES_EQUAL(point->position().point.y(), 20);
	VALUES_EQUAL(point->positionLogical().x(), 0.85);
	VALUES_EQUAL(point->positionLogical().y(), 0.7);
}

// Zoom in cartesianplot leads to move the CustomPoint
// Testing without setCoordinateBindingEnabled

QTEST_MAIN(WorksheetElementTest)
