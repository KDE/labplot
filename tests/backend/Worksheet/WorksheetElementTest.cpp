/*
	File                 : WorksheetTest.cpp
	Project              : LabPlot
	Description          : Tests for Worksheets and positioning them on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "WorksheetElementTest.h"
#include "helperMacros.h"

#include "backend/core/Project.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/TextLabelPrivate.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"
#include "backend/worksheet/plots/cartesian/ReferenceRangePrivate.h"
#include "kdefrontend/widgets/LabelWidget.h"

#define ALL_WORKSHEETELEMENT_TESTS(WorksheetElementType)                                                                                                       \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_SETPOSITIONLOGICAL);                                                                          \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_MOUSE_MOVE);                                                                                  \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT);                                                                              \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEY_PRESSLEFT);                                                                               \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_UP);                                                                                 \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_DOWN);                                                                               \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_ENABLE_DISABLE_COORDBINDING);                                                                 \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_COORDBINDING);                                                                         \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_COORDBINDING);                                                                         \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_NO_COORDBINDING);                                                                      \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_NO_COORDBINDING);

#define ALL_WORKSHEETELEMENT_TESTS2(WorksheetElementType)                                                                                                      \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_SETPOSITIONLOGICAL);                                                                         \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_MOUSE_MOVE);                                                                                 \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT);                                                                             \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEY_PRESSLEFT);                                                                              \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_UP);                                                                                \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_DOWN);                                                                              \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_ENABLE_DISABLE_COORDBINDING);                                                                \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_COORDBINDING);                                                                        \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_COORDBINDING);                                                                        \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_NO_COORDBINDING);                                                                     \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_NO_COORDBINDING);

//ALL_WORKSHEETELEMENT_TESTS(CustomPoint)
//ALL_WORKSHEETELEMENT_TESTS2(TextLabel)
//ALL_WORKSHEETELEMENT_TESTS2(Image)

//void WorksheetElementTest::referenceRangeXMouseMove() {
//	SETUP_PROJECT

//	auto* referenceRange = new ReferenceRange(p, QStringLiteral("range"));
//	referenceRange->setOrientation(ReferenceRange::Orientation::Vertical);
//	p->addChild(referenceRange);
//	referenceRange->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
//	auto pp = referenceRange->position();
//	pp.point = QPointF(0, 0);
//	referenceRange->setPosition(pp);
//	referenceRange->setCoordinateBindingEnabled(true);

//	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
//	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
//	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.45);
//	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.55);
//	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -5); // - width / 2 (logical 0.55 - 0.45) -> scene (-5 - (+5)) = 10
//	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
//	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 10);
//	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);

//	// Simulate mouse move
//	referenceRange->d_ptr->setPos(QPointF(25, -10)); // item change will be called (negative value is up)
//	QCOMPARE(referenceRange->positionLogical().x(), 0.75); // 25/50 * 0.5 + 0.5
//	QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
//	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.7);
//	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.8);
//	// Rect did not change
//	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -5);
//	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
//	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 10);
//	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);

//	// Set Logical Start
//	referenceRange->setPositionLogicalStart(QPointF(0.1, 0.5));
//	QCOMPARE(referenceRange->positionLogical().x(), 0.45);
//	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
//	QCOMPARE(referenceRange->position().point.x(), -5);
//	QCOMPARE(referenceRange->position().point.y(), 0);
//	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.1);
//	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.8);
//	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -35); // (0.8 - 0.1) * 100 / 2
//	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
//	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 70);
//	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);

//	// Set Logical End
//	referenceRange->setPositionLogicalEnd(QPointF(0.3, 0.5));
//	QCOMPARE(referenceRange->positionLogical().x(), 0.2);
//	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
//	QCOMPARE(referenceRange->position().point.x(), -30); //
//	QCOMPARE(referenceRange->position().point.y(), 0);
//	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.1);
//	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.3);
//	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -10); // (0.3 - 0.1) * 100 / 2
//	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
//	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 20);
//	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);
//}

//void WorksheetElementTest::referenceRangeYMouseMove() {
//	SETUP_PROJECT

//	auto* referenceRange = new ReferenceRange(p, QStringLiteral("range"));
//	referenceRange->setOrientation(ReferenceRange::Orientation::Horizontal);
//	p->addChild(referenceRange);
//	referenceRange->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
//	auto pp = referenceRange->position();
//	pp.point = QPointF(0, 0);
//	referenceRange->setPosition(pp);
//	referenceRange->setCoordinateBindingEnabled(true);

//	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
//	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
//	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.45);
//	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.55);
//	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -50); // - width / 2 (logical 0.55 - 0.45) -> scene (-5 - (+5)) = 10
//	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -5);
//	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 100);
//	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 10);

//	// Simulate mouse move
//	referenceRange->d_ptr->setPos(QPointF(-10, -25)); // item change will be called (negative value is up)
//	QCOMPARE(referenceRange->positionLogical().y(), 0.75); // 25/50 * 0.5 + 0.5
//	QCOMPARE(referenceRange->positionLogical().x(), 0.5); // Only vertical considered
//	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.8);
//	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.7);
//	// Rect did not change
//	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -50);
//	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -5);
//	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 100);
//	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 10);

//	// Set Logical Start
//	referenceRange->setPositionLogicalStart(QPointF(0.5, 0.1));
//	QCOMPARE(referenceRange->positionLogical().y(), 0.4);
//	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
//	QCOMPARE(referenceRange->position().point.y(), -10);
//	QCOMPARE(referenceRange->position().point.x(), 0);
//	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.1);
//	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.7);
//	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -50);
//	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -30); // (0.7 - 0.1) * 100 / 2
//	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 100);
//	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 60);

//	// Set Logical End
//	referenceRange->setPositionLogicalEnd(QPointF(0.5, 0.3));
//	QCOMPARE(referenceRange->positionLogical().y(), 0.2);
//	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
//	QCOMPARE(referenceRange->position().point.y(), -30);
//	QCOMPARE(referenceRange->position().point.x(), 0);
//	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.1);
//	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.3);
//	VALUES_EQUAL(referenceRange->d_func()->rect.x(), -50);
//	VALUES_EQUAL(referenceRange->d_func()->rect.y(), -10);
//	VALUES_EQUAL(referenceRange->d_func()->rect.width(), 100);
//	VALUES_EQUAL(referenceRange->d_func()->rect.height(), 20);
//}

//void WorksheetElementTest::referenceRangeXClippingLeftMouse() {
//    SETUP_PROJECT

//    auto* referenceRange = new ReferenceRange(p, QStringLiteral("range"));
//    referenceRange->setOrientation(ReferenceRange::Orientation::Vertical);
//    p->addChild(referenceRange);
//    referenceRange->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
//    auto pp = referenceRange->position();
//    pp.point = QPointF(0, 0);
//    referenceRange->setPosition(pp);
//    referenceRange->setCoordinateBindingEnabled(true);

//    QCOMPARE(referenceRange->positionLogical().x(), 0.5);
//    QCOMPARE(referenceRange->positionLogical().y(), 0.5);
//    QCOMPARE(referenceRange->positionLogicalStart().x(), 0.45);
//    QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.55);
//    VALUES_EQUAL(referenceRange->d_func()->rect.x(), -5); // - width / 2 (logical 0.55 - 0.45) -> scene (-5 - (+5)) = 10
//    VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
//    VALUES_EQUAL(referenceRange->d_func()->rect.width(), 10);
//    VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);

//    // Simulate mouse move
//    referenceRange->d_ptr->setPos(QPointF(-50, -10)); // item change will be called (negative value is up)
//    QCOMPARE(referenceRange->positionLogical().x(), 0.0);
//    QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
//    QCOMPARE(referenceRange->positionLogicalStart().x(), -0.05);
//    QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.05);
//    // Rect is clipped
//    VALUES_EQUAL(referenceRange->d_func()->rect.x(), 0); // clipped value
//    VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
//    VALUES_EQUAL(referenceRange->d_func()->rect.width(), 5);
//    VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);
//}

void WorksheetElementTest::referenceRangeXClippingLeftSetStart() {
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


    referenceRange->setPositionLogicalStart(QPointF(-5, 0.5));
    QCOMPARE(referenceRange->positionLogical().x(), -2.75); // (-5+0.5)/2
    QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
    QCOMPARE(referenceRange->positionLogicalStart().x(), -5);
    QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.55);
    // Rect is clipped
    VALUES_EQUAL(referenceRange->d_func()->rect.x(), 0); // clipped value
    VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
    VALUES_EQUAL(referenceRange->d_func()->rect.width(), 55);
    VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);
}

void WorksheetElementTest::referenceRangeXClippingRightSetEnd() {
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
    referenceRange->setPositionLogicalEnd(QPointF(+5, 0.5));
    QCOMPARE(referenceRange->positionLogical().x(), 2.75); // (-5+0.5)/2
    QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
    QCOMPARE(referenceRange->positionLogicalStart().x(), 0.45);
    QCOMPARE(referenceRange->positionLogicalEnd().x(), 5);
    // Rect is clipped
    VALUES_EQUAL(referenceRange->d_func()->rect.x(), 0); // clipped value
    VALUES_EQUAL(referenceRange->d_func()->rect.y(), -50);
    VALUES_EQUAL(referenceRange->d_func()->rect.width(), 55);
    VALUES_EQUAL(referenceRange->d_func()->rect.height(), 100);
}

// TODO: create test with reference range with nonlinear ranges!
// Zoom in cartesianplot leads to move the CustomPoint
// Testing without setCoordinateBindingEnabled

// Undo redo of moving the position, keyboard, mousemove, manual setting

QTEST_MAIN(WorksheetElementTest)
