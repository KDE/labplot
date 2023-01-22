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
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"
#include "backend/worksheet/plots/cartesian/ReferenceRangePrivate.h"
#include "kdefrontend/widgets/LabelWidget.h"

#define SETUP_PROJECT                                                                                                                                          \
	Project project;                                                                                                                                           \
	auto* ws = new Worksheet(QStringLiteral("worksheet"));                                                                                                     \
	QVERIFY(ws != nullptr);                                                                                                                                    \
	project.addChild(ws);                                                                                                                                      \
                                                                                                                                                               \
	auto* p = new CartesianPlot(QStringLiteral("plot"));                                                                                                       \
	p->setType(CartesianPlot::Type::TwoAxes); /* Otherwise no axis are created */                                                                              \
	QVERIFY(p != nullptr);                                                                                                                                     \
	ws->addChild(p);                                                                                                                                           \
                                                                                                                                                               \
	p->setHorizontalPadding(0);                                                                                                                                \
	p->setVerticalPadding(0);                                                                                                                                  \
	p->setRightPadding(0);                                                                                                                                     \
	p->setBottomPadding(0);                                                                                                                                    \
	p->setRect(QRectF(0, 0, 100, 100));                                                                                                                        \
                                                                                                                                                               \
	QCOMPARE(p->rangeCount(Dimension::X), 1);                                                                                                                  \
	QCOMPARE(p->range(Dimension::X, 0).start(), 0);                                                                                                            \
	QCOMPARE(p->range(Dimension::X, 0).end(), 1);                                                                                                              \
	QCOMPARE(p->rangeCount(Dimension::Y), 1);                                                                                                                  \
	QCOMPARE(p->range(Dimension::Y, 0).start(), 0);                                                                                                            \
	QCOMPARE(p->range(Dimension::Y, 0).end(), 1);                                                                                                              \
                                                                                                                                                               \
	/* For simplicity use even numbers */                                                                                                                      \
	QCOMPARE(p->dataRect().x(), -50);                                                                                                                          \
	QCOMPARE(p->dataRect().y(), -50);                                                                                                                          \
	QCOMPARE(p->dataRect().width(), 100);                                                                                                                      \
	QCOMPARE(p->dataRect().height(), 100);

void WorksheetElementTest::customPointSetPositionLogical() {
	SETUP_PROJECT

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
	SETUP_PROJECT

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

void WorksheetElementTest::customPointKeyPressMoveRight() {
	SETUP_PROJECT

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

void WorksheetElementTest::customPointKeyPressMoveLeft() {
	SETUP_PROJECT

	auto* point = new CustomPoint(p, QStringLiteral("point"));
	p->addChild(point);
	point->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	auto pp = point->position();
	pp.point = QPointF(0, 0);
	point->setPosition(pp);
	point->setCoordinateBindingEnabled(true);

	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Left, Qt::KeyboardModifier::NoModifier);
	point->d_ptr->keyPressEvent(&event);

	VALUES_EQUAL(point->position().point.x(), -5);
	VALUES_EQUAL(point->position().point.y(), 0);
	VALUES_EQUAL(point->positionLogical().x(), 0.45);
	VALUES_EQUAL(point->positionLogical().y(), 0.5);
}

void WorksheetElementTest::customPointKeyPressMoveUp() {
	SETUP_PROJECT

	auto* point = new CustomPoint(p, QStringLiteral("point"));
	p->addChild(point);
	point->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	auto pp = point->position();
	pp.point = QPointF(0, 0);
	point->setPosition(pp);
	point->setCoordinateBindingEnabled(true);

	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	point->d_ptr->keyPressEvent(&event);

	VALUES_EQUAL(point->position().point.x(), 0);
	VALUES_EQUAL(point->position().point.y(), 5);
	VALUES_EQUAL(point->positionLogical().x(), 0.5);
	VALUES_EQUAL(point->positionLogical().y(), 0.55);
}

void WorksheetElementTest::customPointKeyPressMoveDown() {
	SETUP_PROJECT

	auto* point = new CustomPoint(p, QStringLiteral("point"));
	p->addChild(point);
	point->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	auto pp = point->position();
	pp.point = QPointF(0, 0);
	point->setPosition(pp);
	point->setCoordinateBindingEnabled(true);

	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
	point->d_ptr->keyPressEvent(&event);

	VALUES_EQUAL(point->position().point.x(), 0);
	VALUES_EQUAL(point->position().point.y(), -5);
	VALUES_EQUAL(point->positionLogical().x(), 0.5);
	VALUES_EQUAL(point->positionLogical().y(), 0.45);
}

// Switching between setCoordinateBindingEnabled true and false should not move the point
void WorksheetElementTest::customPointEnableDisableCoordBinding() {
	SETUP_PROJECT

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

void WorksheetElementTest::customPointShiftXPlotCoordBinding() {
	SETUP_PROJECT

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

	p->shiftLeftX();

	VALUES_EQUAL(point->position().point.x(), 25); // shift factor is 0.1 -> 1(current range)*0.1 = 0.1 = -10 in scene coords
	VALUES_EQUAL(point->position().point.y(), 20);
	VALUES_EQUAL(point->positionLogical().x(), 0.85);
	VALUES_EQUAL(point->positionLogical().y(), 0.7);
}

void WorksheetElementTest::customPointShiftYPlotCoordBinding() {
	SETUP_PROJECT

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

	p->shiftUpY();

	VALUES_EQUAL(point->position().point.x(), 35);
	VALUES_EQUAL(point->position().point.y(), 30); // shift factor is 0.1 -> 1(current range)*0.1 = 0.1 = +10 in scene coords (for UP)
	VALUES_EQUAL(point->positionLogical().x(), 0.85);
	VALUES_EQUAL(point->positionLogical().y(), 0.7);
}

void WorksheetElementTest::customPointShiftXPlotNoCoordBinding() {
	SETUP_PROJECT

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
	point->setCoordinateBindingEnabled(false);

	VALUES_EQUAL(point->position().point.x(), 35);
	VALUES_EQUAL(point->position().point.y(), 20);
	VALUES_EQUAL(point->positionLogical().x(), 0.85);
	VALUES_EQUAL(point->positionLogical().y(), 0.7);

	p->shiftLeftX();

	VALUES_EQUAL(point->position().point.x(), 35);
	VALUES_EQUAL(point->position().point.y(), 20);
	VALUES_EQUAL(point->positionLogical().x(), 0.95); // shift factor is 0.1 -> 1(current range)*0.1 = 0.1
	VALUES_EQUAL(point->positionLogical().y(), 0.7);
}

void WorksheetElementTest::customPointShiftYPlotNoCoordBinding() {
	SETUP_PROJECT

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
	point->setCoordinateBindingEnabled(false);

	VALUES_EQUAL(point->position().point.x(), 35);
	VALUES_EQUAL(point->position().point.y(), 20);
	VALUES_EQUAL(point->positionLogical().x(), 0.85);
	VALUES_EQUAL(point->positionLogical().y(), 0.7);

	p->shiftUpY();

	VALUES_EQUAL(point->position().point.x(), 35);
	VALUES_EQUAL(point->position().point.y(), 20);
	VALUES_EQUAL(point->positionLogical().x(), 0.85);
	VALUES_EQUAL(point->positionLogical().y(), 0.6); // shift factor is 0.1 -> 1(current range)*0.1 = 0.1
}

void WorksheetElementTest::referenceRangeXMouseMove() {
	SETUP_PROJECT

	auto* referenceRange = new ReferenceRange(p, QStringLiteral("range"));
	referenceRange->setOrientation(ReferenceRange::Orientation::Vertical);
	p->addChild(referenceRange);
	referenceRange->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	auto pp = referenceRange->position();
	pp.point = QPointF(0, 0);
	referenceRange->setPosition(pp);
	referenceRange->setCoordinateBindingEnabled(true);

	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.45);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.55);
	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -5); // - width / 2 (logical 0.55 - 0.45) -> scene (-5 - (+5)) = 10
	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 10);
	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);

	// Simulate mouse move
	referenceRange->d_ptr->setPos(QPointF(25, -10)); // item change will be called (negative value is up)
	QCOMPARE(referenceRange->positionLogical().x(), 0.75); // 25/50 * 0.5 + 0.5
	QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.7);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.8);
	// Rect did not change
	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -5);
	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 10);
	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);

	// Set Logical Start
	referenceRange->setPositionLogicalStart(QPointF(0.1, 0.5));
	QCOMPARE(referenceRange->positionLogical().x(), 0.45);
	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
	QCOMPARE(referenceRange->position().point.x(), -5);
	QCOMPARE(referenceRange->position().point.y(), 0);
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.8);
	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -35); // (0.8 - 0.1) * 100 / 2
	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 70);
	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);

	// Set Logical End
	referenceRange->setPositionLogicalEnd(QPointF(0.3, 0.5));
	QCOMPARE(referenceRange->positionLogical().x(), 0.2);
	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
	QCOMPARE(referenceRange->position().point.x(), -30); //
	QCOMPARE(referenceRange->position().point.y(), 0);
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.3);
	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -10); // (0.3 - 0.1) * 100 / 2
	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 20);
	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);
}

void WorksheetElementTest::referenceRangeYMouseMove() {
	SETUP_PROJECT

	auto* referenceRange = new ReferenceRange(p, QStringLiteral("range"));
	referenceRange->setOrientation(ReferenceRange::Orientation::Horizontal);
	p->addChild(referenceRange);
	referenceRange->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	auto pp = referenceRange->position();
	pp.point = QPointF(0, 0);
	referenceRange->setPosition(pp);
	referenceRange->setCoordinateBindingEnabled(true);

	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.45);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.55);
	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -50); // - width / 2 (logical 0.55 - 0.45) -> scene (-5 - (+5)) = 10
	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -5);
	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 100);
	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 10);

	// Simulate mouse move
	referenceRange->d_ptr->setPos(QPointF(-10, -25)); // item change will be called (negative value is up)
	QCOMPARE(referenceRange->positionLogical().y(), 0.75); // 25/50 * 0.5 + 0.5
	QCOMPARE(referenceRange->positionLogical().x(), 0.5); // Only vertical considered
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.8);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.7);
	// Rect did not change
	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -50);
	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -5);
	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 100);
	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 10);

	// Set Logical Start
	referenceRange->setPositionLogicalStart(QPointF(0.5, 0.1));
	QCOMPARE(referenceRange->positionLogical().y(), 0.4);
	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
	QCOMPARE(referenceRange->position().point.y(), -10);
	QCOMPARE(referenceRange->position().point.x(), 0);
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.7);
	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -50);
	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -30); // (0.7 - 0.1) * 100 / 2
	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 100);
	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 60);

	// Set Logical End
	referenceRange->setPositionLogicalEnd(QPointF(0.5, 0.3));
	QCOMPARE(referenceRange->positionLogical().y(), 0.2);
	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
	QCOMPARE(referenceRange->position().point.y(), -30);
	QCOMPARE(referenceRange->position().point.x(), 0);
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.3);
	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -50);
	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -10);
	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 100);
	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 20);
}

// TODO: create test with reference range with nonlinear ranges!
// Zoom in cartesianplot leads to move the CustomPoint
// Testing without setCoordinateBindingEnabled

// Undo redo of moving the position, keyboard, mousemove, manual setting

QTEST_MAIN(WorksheetElementTest)
