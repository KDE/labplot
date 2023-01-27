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

#define CHECK_REFERENCERANGE_RECT(referenceRange, xLeftRef, yTopRef, xRightRef, yBottomRef)                                                                    \
	do {                                                                                                                                                       \
		QVERIFY(referenceRange->cSystem);                                                                                                                      \
		QPointF topLeftLogical = referenceRange->cSystem->mapSceneToLogical(referenceRange->d_func()->mapToParent(referenceRange->d_func()->rect.topLeft()));  \
		VALUES_EQUAL(topLeftLogical.x(), xLeftRef);                                                                                                            \
		VALUES_EQUAL(topLeftLogical.y(), yTopRef);                                                                                                             \
		QPointF bottomRightLogical =                                                                                                                           \
			referenceRange->cSystem->mapSceneToLogical(referenceRange->d_func()->mapToParent(referenceRange->d_func()->rect.bottomRight()));                   \
		VALUES_EQUAL(bottomRightLogical.x(), xRightRef);                                                                                                       \
		VALUES_EQUAL(bottomRightLogical.y(), yBottomRef);                                                                                                      \
	} while (false);

ALL_WORKSHEETELEMENT_TESTS(CustomPoint)
ALL_WORKSHEETELEMENT_TESTS2(TextLabel)
ALL_WORKSHEETELEMENT_TESTS2(Image)

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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0.45, 1, 0.55, 0);

	// Simulate mouse move
	referenceRange->d_ptr->setPos(QPointF(25, -10)); // item change will be called (negative value is up)
	QCOMPARE(referenceRange->positionLogical().x(), 0.75); // 25/50 * 0.5 + 0.5
	QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.7);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.8);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0.7, 1, 0.8, 0);

	// Set Logical Start
	referenceRange->setPositionLogicalStart(QPointF(0.1, 0.5));
	QCOMPARE(referenceRange->positionLogical().x(), 0.45);
	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
	QCOMPARE(referenceRange->position().point.x(), -5);
	QCOMPARE(referenceRange->position().point.y(), 0);
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.8);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0.1, 1, 0.8, 0);

	// Set Logical End
	referenceRange->setPositionLogicalEnd(QPointF(0.3, 0.5));
	QCOMPARE(referenceRange->positionLogical().x(), 0.2);
	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
	QCOMPARE(referenceRange->position().point.x(), -30); //
	QCOMPARE(referenceRange->position().point.y(), 0);
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.3);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0.1, 1, 0.3, 0);
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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0, 0.55, 1, 0.45);

	// Simulate mouse move
	referenceRange->d_ptr->setPos(QPointF(-10, -25)); // item change will be called (negative value is up)
	QCOMPARE(referenceRange->positionLogical().y(), 0.75); // 25/50 * 0.5 + 0.5
	QCOMPARE(referenceRange->positionLogical().x(), 0.5); // Only vertical considered
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.8);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.7);
	CHECK_REFERENCERANGE_RECT(referenceRange, 0, 0.8, 1, 0.7);

	// Set Logical Start
	referenceRange->setPositionLogicalStart(QPointF(0.5, 0.1));
	QCOMPARE(referenceRange->positionLogical().y(), 0.4);
	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
	QCOMPARE(referenceRange->position().point.y(), -10);
	QCOMPARE(referenceRange->position().point.x(), 0);
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.7);
	CHECK_REFERENCERANGE_RECT(referenceRange, 0, 0.7, 1, 0.1);
	// Set Logical End
	referenceRange->setPositionLogicalEnd(QPointF(0.5, 0.3));
	QCOMPARE(referenceRange->positionLogical().y(), 0.2);
	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
	QCOMPARE(referenceRange->position().point.y(), -30);
	QCOMPARE(referenceRange->position().point.x(), 0);
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.3);
	CHECK_REFERENCERANGE_RECT(referenceRange, 0, 0.3, 1, 0.1);
}

void WorksheetElementTest::referenceRangeXClippingLeftMouse() {
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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0.45, 1, 0.55, 0);

	// Simulate mouse move
	referenceRange->d_ptr->setPos(QPointF(-50, -10)); // item change will be called (negative value is up)
	QCOMPARE(referenceRange->positionLogical().x(), 0.0);
	QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().x(), -0.05);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.05);
	CHECK_REFERENCERANGE_RECT(referenceRange, 0, 1, 0.05, 0);
}

/*          Logical                         Scene
 *     ^                                         ^ -100
 * 1   |                                         |
 *     |                                         |
 *     |                                         |
 *     |                                         |0               100
 * 0.5 |                           --------------+---------------->
 *     |                                         |
 *     |                                         |
 *     |                                         |
 *     |                                         |+100
 * 0   +---------------------->
 *     0         0.5         1
 */

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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0.45, 1, 0.55, 0);

	referenceRange->setPositionLogicalStart(QPointF(-5, 0.5));
	QCOMPARE(referenceRange->positionLogical().x(), -2.225); // (-5+0.55)/2
	QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().x(), -5);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.55);
	QCOMPARE(referenceRange->position().point.x(), -2.225 * 100 - 50); // - 50 because 0 logical is -50 in scene
	QCOMPARE(referenceRange->position().point.y(), 0);
	QCOMPARE(referenceRange->d_func()->pos().x(), -2.225 * 100 - 50); // - 50 because 0 logical is -50 in scene
	QCOMPARE(referenceRange->d_func()->pos().y(), 0);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0, 1, 0.55, 0);
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

	CHECK_REFERENCERANGE_RECT(referenceRange, 0.45, 1, 0.55, 0);

	// Simulate mouse move
	referenceRange->setPositionLogicalEnd(QPointF(+5, 0.5));
	QCOMPARE(referenceRange->positionLogical().x(), 2.725); // (5+0.45)/2
	QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.45);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 5);
	QCOMPARE(referenceRange->position().point.x(), 2.725 * 100 - 50); // 0 logical is at -50
	QCOMPARE(referenceRange->position().point.y(), 0);
	QCOMPARE(referenceRange->d_func()->pos().x(), (2.725) * 100 - 50);
	QCOMPARE(referenceRange->d_func()->pos().y(), 0);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0.45, 1, 1, 0); // Xend is clipped
}

void WorksheetElementTest::referenceRangeYClippingBottomSetEnd() {
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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0, 0.55, 1, 0.45);

	// Simulate mouse move
	referenceRange->setPositionLogicalStart(QPointF(0.5, -5));
	QCOMPARE(referenceRange->positionLogical().y(), -2.225); // (-5+0.45)/2
	QCOMPARE(referenceRange->positionLogical().x(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().y(), -5);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.55);
	QCOMPARE(referenceRange->position().point.y(), -272.5);
	QCOMPARE(referenceRange->position().point.x(), 0);
	QCOMPARE(referenceRange->d_func()->pos().y(), (+2.725) * 100);
	QCOMPARE(referenceRange->d_func()->pos().x(), 0);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0, 0.55, 1, 0);
}

void WorksheetElementTest::referenceRangeYClippingTopSetEnd() {
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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0, 0.55, 1, 0.45);

	// Simulate mouse move
	referenceRange->setPositionLogicalEnd(QPointF(0.5, 5));
	QCOMPARE(referenceRange->positionLogical().y(), 2.725); // (5+0.45)/2
	QCOMPARE(referenceRange->positionLogical().x(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.45);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 5);
	QCOMPARE(referenceRange->position().point.y(), (2.225) * 100);
	QCOMPARE(referenceRange->position().point.x(), 0);
	QCOMPARE(referenceRange->d_func()->pos().y(), (-2.225) * 100);
	QCOMPARE(referenceRange->d_func()->pos().x(), 0);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0, 1, 1, 0.45);
}

// TODO: create test with reference range with nonlinear ranges!
// Zooming in cartesianplot leads to move the worksheetelement
// Testing without setCoordinateBindingEnabled
// Undo redo of moving the position, keyboard, mousemove, manual setting

QTEST_MAIN(WorksheetElementTest)
