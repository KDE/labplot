#ifndef HELPERMACROS_H
#define HELPERMACROS_H

#include "backend/lib/UndoStack.h"

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
	QCOMPARE(p->range(Dimension::X, 0).start(), 0.);                                                                                                           \
	QCOMPARE(p->range(Dimension::X, 0).end(), 1.);                                                                                                             \
	QCOMPARE(p->rangeCount(Dimension::Y), 1);                                                                                                                  \
	QCOMPARE(p->range(Dimension::Y, 0).start(), 0.);                                                                                                           \
	QCOMPARE(p->range(Dimension::Y, 0).end(), 1.);                                                                                                             \
                                                                                                                                                               \
	/* For simplicity use even numbers */                                                                                                                      \
	QCOMPARE(p->dataRect().x(), -50.);                                                                                                                         \
	QCOMPARE(p->dataRect().y(), -50.);                                                                                                                         \
	QCOMPARE(p->dataRect().width(), 100.);                                                                                                                     \
	QCOMPARE(p->dataRect().height(), 100.);

#define WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, MACRO_NAME) void WorksheetElementType##MACRO_NAME()

#define WORKSHEETELEMENT_TEST(WorksheetElementType, MACRO_NAME, DockType, dockSetElementsMethodName)                                                           \
	void WorksheetElementTest::WorksheetElementType##MACRO_NAME() {                                                                                            \
		SETUP_PROJECT                                                                                                                                          \
                                                                                                                                                               \
		auto* element = new WorksheetElementType(p, QStringLiteral("element"));                                                                                \
		p->addChild(element);                                                                                                                                  \
		auto* dock = new DockType(nullptr);                                                                                                                    \
		Q_UNUSED(dock);                                                                                                                                        \
		MACRO_NAME(element, dockSetElementsMethodName);                                                                                                        \
	}

// Without Cartesian Plot as argument to the constructor
// Because of that the logical positions are not available before calling at least
// once updatePosition(). Because of this retransform() will be done in this version
#define WORKSHEETELEMENT_TEST2(WorksheetElementType, MACRO_NAME, DockType, dockSetElementsMethodName)                                                          \
	void WorksheetElementTest::WorksheetElementType##MACRO_NAME() {                                                                                            \
		SETUP_PROJECT                                                                                                                                          \
                                                                                                                                                               \
		auto* element = new WorksheetElementType(QStringLiteral("element"));                                                                                   \
		p->addChild(element);                                                                                                                                  \
		VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                       \
		VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                       \
		element->retransform(); /* Needed because otherwise logical position is not set */                                                                     \
		auto* dock = new DockType(nullptr);                                                                                                                    \
		Q_UNUSED(dock);                                                                                                                                        \
		MACRO_NAME(element, dockSetElementsMethodName);                                                                                                        \
	}

// Test Macros
// Testing if logical points are set correctly
#define WORKSHEETELEMENT_SETPOSITIONLOGICAL(element, dockSetElementsMethodName)                                                                                \
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      \
	auto pp = element->position();                                                                                                                             \
	pp.point = QPointF(0, 0);                                                                                                                                  \
	element->setPosition(pp);                                                                                                                                  \
	element->setCoordinateBindingEnabled(true);                                                                                                                \
                                                                                                                                                               \
	QCOMPARE(element->positionLogical().x(), 0.5);                                                                                                             \
	QCOMPARE(element->positionLogical().y(), 0.5);                                                                                                             \
                                                                                                                                                               \
	element->setPositionLogical(QPointF(1, 1));                                                                                                                \
	QCOMPARE(element->positionLogical().x(), 1.);                                                                                                              \
	QCOMPARE(element->positionLogical().y(), 1.);                                                                                                              \
	QCOMPARE(element->position().point.x(), 50.);                                                                                                              \
	QCOMPARE(element->position().point.y(), 50.);

#define WORKSHEETELEMENT_MOUSE_MOVE(element, dockSetElementsMethodName)                                                                                        \
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      \
	auto pp = element->position();                                                                                                                             \
	pp.point = QPointF(0, 0);                                                                                                                                  \
	element->setPosition(pp);                                                                                                                                  \
	element->setCoordinateBindingEnabled(true);                                                                                                                \
                                                                                                                                                               \
	QCOMPARE(element->positionLogical().x(), 0.5);                                                                                                             \
	QCOMPARE(element->positionLogical().y(), 0.5);                                                                                                             \
                                                                                                                                                               \
	/* Simulate mouse move */                                                                                                                                  \
	element->d_ptr->setPos(QPointF(25, -10)); /* item change will be called (negative value is up) */                                                          \
	QCOMPARE(element->positionLogical().x(), 0.75); /* 25/50 * 0.5 + 0.5 */                                                                                    \
	QCOMPARE(element->positionLogical().y(), 0.6);                                                                                                             \
                                                                                                                                                               \
	element->setPositionLogical(QPointF(0.3, 0.1));                                                                                                            \
	QCOMPARE(element->positionLogical().x(), 0.3);                                                                                                             \
	QCOMPARE(element->positionLogical().y(), 0.1);                                                                                                             \
	QCOMPARE(element->position().point.x(), -20.);                                                                                                             \
	QCOMPARE(element->position().point.y(), -40.);

#define WORKSHEETELEMENT_KEYPRESS_NO_COORD_BINDING(element, dockSetElementsMethodName, KeyType, xScene, yScene, xLogical, yLogical)                            \
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      \
	auto pp = element->position();                                                                                                                             \
	pp.point = QPointF(0, 0);                                                                                                                                  \
	element->setPosition(pp);                                                                                                                                  \
	element->setCoordinateBindingEnabled(false);                                                                                                               \
                                                                                                                                                               \
	dock->dockSetElementsMethodName({element});                                                                                                                \
	QCOMPARE(dock->ui.chbBindLogicalPos->isChecked(), false);                                                                                                  \
	QCOMPARE(dock->ui.sbPositionX->value(),                                                                                                                    \
			 BaseDock::roundValue(Worksheet::convertFromSceneUnits(element->position().point.x(), Worksheet::Unit::Centimeter)));                              \
	QCOMPARE(dock->ui.sbPositionY->value(),                                                                                                                    \
			 BaseDock::roundValue(Worksheet::convertFromSceneUnits(element->position().point.y(), Worksheet::Unit::Centimeter)));                              \
	QCOMPARE(dock->ui.sbPositionXLogical->value(), element->positionLogical().x());                                                                            \
	QCOMPARE(dock->ui.sbPositionYLogical->value(), element->positionLogical().y());                                                                            \
                                                                                                                                                               \
	QKeyEvent event(QKeyEvent::Type::KeyPress, KeyType, Qt::KeyboardModifier::NoModifier);                                                                     \
	element->d_ptr->keyPressEvent(&event);                                                                                                                     \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), xScene);                                                                                                       \
	VALUES_EQUAL(element->position().point.y(), yScene);                                                                                                       \
	VALUES_EQUAL(element->positionLogical().x(), xLogical);                                                                                                    \
	VALUES_EQUAL(element->positionLogical().y(), yLogical);                                                                                                    \
                                                                                                                                                               \
	QCOMPARE(dock->ui.sbPositionX->value(),                                                                                                                    \
			 BaseDock::roundValue(Worksheet::convertFromSceneUnits(element->position().point.x(), Worksheet::Unit::Centimeter)));                              \
	QCOMPARE(dock->ui.sbPositionY->value(),                                                                                                                    \
			 BaseDock::roundValue(Worksheet::convertFromSceneUnits(element->position().point.y(), Worksheet::Unit::Centimeter)));                              \
	QCOMPARE(dock->ui.sbPositionXLogical->value(), element->positionLogical().x());                                                                            \
	QCOMPARE(dock->ui.sbPositionYLogical->value(), element->positionLogical().y());

#define WORKSHEETELEMENT_KEYPRESS_RIGHT_NO_COORD_BINDING(element, dockSetElementsMethodName)                                                                   \
	WORKSHEETELEMENT_KEYPRESS_NO_COORD_BINDING(element, dockSetElementsMethodName, Qt::Key_Right, 5., 0., 0.55, 0.5)

#define WORKSHEETELEMENT_KEYPRESS_DOWN_NO_COORD_BINDING(element, dockSetElementsMethodName)                                                                    \
	WORKSHEETELEMENT_KEYPRESS_NO_COORD_BINDING(element, dockSetElementsMethodName, Qt::Key_Down, 0., -5., 0.5, 0.45)

#define WORKSHEETELEMENT_KEYPRESS(element, dockSetElementsMethodName, KeyType, xScene, yScene, xLogical, yLogical)                                             \
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      \
	auto pp = element->position();                                                                                                                             \
	pp.point = QPointF(0, 0);                                                                                                                                  \
	element->setPosition(pp);                                                                                                                                  \
	element->setCoordinateBindingEnabled(true);                                                                                                                \
                                                                                                                                                               \
	dock->dockSetElementsMethodName({element});                                                                                                                \
	QCOMPARE(dock->ui.chbBindLogicalPos->isChecked(), true);                                                                                                   \
	QCOMPARE(dock->ui.sbPositionX->value(),                                                                                                                    \
			 BaseDock::roundValue(Worksheet::convertFromSceneUnits(element->position().point.x(), Worksheet::Unit::Centimeter)));                              \
	QCOMPARE(dock->ui.sbPositionY->value(),                                                                                                                    \
			 BaseDock::roundValue(Worksheet::convertFromSceneUnits(element->position().point.y(), Worksheet::Unit::Centimeter)));                              \
	QCOMPARE(dock->ui.sbPositionXLogical->value(), element->positionLogical().x());                                                                            \
	QCOMPARE(dock->ui.sbPositionYLogical->value(), element->positionLogical().y());                                                                            \
                                                                                                                                                               \
	QKeyEvent event(QKeyEvent::Type::KeyPress, KeyType, Qt::KeyboardModifier::NoModifier);                                                                     \
	element->d_ptr->keyPressEvent(&event);                                                                                                                     \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), xScene);                                                                                                       \
	VALUES_EQUAL(element->position().point.y(), yScene);                                                                                                       \
	VALUES_EQUAL(element->positionLogical().x(), xLogical);                                                                                                    \
	VALUES_EQUAL(element->positionLogical().y(), yLogical);                                                                                                    \
                                                                                                                                                               \
	QCOMPARE(dock->ui.sbPositionX->value(),                                                                                                                    \
			 BaseDock::roundValue(Worksheet::convertFromSceneUnits(element->position().point.x(), Worksheet::Unit::Centimeter)));                              \
	QCOMPARE(dock->ui.sbPositionY->value(),                                                                                                                    \
			 BaseDock::roundValue(Worksheet::convertFromSceneUnits(element->position().point.y(), Worksheet::Unit::Centimeter)));                              \
	QCOMPARE(dock->ui.sbPositionXLogical->value(), element->positionLogical().x());                                                                            \
	QCOMPARE(dock->ui.sbPositionYLogical->value(), element->positionLogical().y());

#define WORKSHEETELEMENT_KEYPRESS_RIGHT(element, dockSetElementsMethodName)                                                                                    \
	WORKSHEETELEMENT_KEYPRESS(element, dockSetElementsMethodName, Qt::Key_Right, 5., 0., 0.55, 0.5)

#define WORKSHEETELEMENT_KEY_PRESSLEFT(element, dockSetElementsMethodName)                                                                                     \
	WORKSHEETELEMENT_KEYPRESS(element, dockSetElementsMethodName, Qt::Key_Left, -5., 0., 0.45, 0.5)

#define WORKSHEETELEMENT_KEYPRESS_UP(element, dockSetElementsMethodName)                                                                                       \
	WORKSHEETELEMENT_KEYPRESS(element, dockSetElementsMethodName, Qt::Key_Up, 0., 5., 0.5, 0.55)

#define WORKSHEETELEMENT_KEYPRESS_DOWN(element, dockSetElementsMethodName)                                                                                     \
	WORKSHEETELEMENT_KEYPRESS(element, dockSetElementsMethodName, Qt::Key_Down, 0., -5., 0.5, 0.45)

#define WORKSHEETELEMENT_KEYPRESS_UNDO(element, dockSetElementsMethodName, KeyType, xScene, yScene, xLogical, yLogical)                                        \
	WORKSHEETELEMENT_KEYPRESS(element, dockSetElementsMethodName, KeyType, xScene, yScene, xLogical, yLogical)                                                 \
	element->undoStack()->undo();                                                                                                                              \
	VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                           \
	VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                           \
	VALUES_EQUAL(element->positionLogical().x(), 0.5);                                                                                                         \
	VALUES_EQUAL(element->positionLogical().y(), 0.5);

#define WORKSHEETELEMENT_KEYPRESS_RIGHT_UNDO(element, dockSetElementsMethodName)                                                                               \
	WORKSHEETELEMENT_KEYPRESS_UNDO(element, dockSetElementsMethodName, Qt::Key_Right, 5., 0., 0.55, 0.5)

#define WORKSHEETELEMENT_KEYPRESS_UP_UNDO(element, dockSetElementsMethodName)                                                                                  \
	WORKSHEETELEMENT_KEYPRESS_UNDO(element, dockSetElementsMethodName, Qt::Key_Up, 0., 5., 0.5, 0.55)

// Switching between setCoordinateBindingEnabled true and false should not move the point
#define WORKSHEETELEMENT_ENABLE_DISABLE_COORDBINDING(element, dockSetElementsMethodName)                                                                       \
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      \
	auto pp = element->position();                                                                                                                             \
	pp.point = QPointF(0, 0);                                                                                                                                  \
	element->setPosition(pp);                                                                                                                                  \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                           \
	VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                           \
	VALUES_EQUAL(element->positionLogical().x(), 0.5);                                                                                                         \
	VALUES_EQUAL(element->positionLogical().y(), 0.5);                                                                                                         \
                                                                                                                                                               \
	element->setCoordinateBindingEnabled(true);                                                                                                                \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                           \
	VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                           \
	VALUES_EQUAL(element->positionLogical().x(), 0.5);                                                                                                         \
	VALUES_EQUAL(element->positionLogical().y(), 0.5);                                                                                                         \
                                                                                                                                                               \
	/* Set position to another than the origin */                                                                                                              \
	element->setPositionLogical(QPointF(0.85, 0.7));                                                                                                           \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 35.);                                                                                                          \
	VALUES_EQUAL(element->position().point.y(), 20.);                                                                                                          \
	VALUES_EQUAL(element->positionLogical().x(), 0.85);                                                                                                        \
	VALUES_EQUAL(element->positionLogical().y(), 0.7);                                                                                                         \
                                                                                                                                                               \
	element->setCoordinateBindingEnabled(false);                                                                                                               \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 35.);                                                                                                          \
	VALUES_EQUAL(element->position().point.y(), 20.);                                                                                                          \
	VALUES_EQUAL(element->positionLogical().x(), 0.85);                                                                                                        \
	VALUES_EQUAL(element->positionLogical().y(), 0.7);                                                                                                         \
                                                                                                                                                               \
	element->setCoordinateBindingEnabled(true);                                                                                                                \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 35.);                                                                                                          \
	VALUES_EQUAL(element->position().point.y(), 20.);                                                                                                          \
	VALUES_EQUAL(element->positionLogical().x(), 0.85);                                                                                                        \
	VALUES_EQUAL(element->positionLogical().y(), 0.7);

#define WORKSHEETELEMENT_SHIFTX_COORDBINDING(element, dockSetElementsMethodName)                                                                               \
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      \
	auto pp = element->position();                                                                                                                             \
	pp.point = QPointF(0, 0);                                                                                                                                  \
	element->setPosition(pp);                                                                                                                                  \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                           \
	VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                           \
	VALUES_EQUAL(element->positionLogical().x(), 0.5);                                                                                                         \
	VALUES_EQUAL(element->positionLogical().y(), 0.5);                                                                                                         \
                                                                                                                                                               \
	element->setCoordinateBindingEnabled(true);                                                                                                                \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                           \
	VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                           \
	VALUES_EQUAL(element->positionLogical().x(), 0.5);                                                                                                         \
	VALUES_EQUAL(element->positionLogical().y(), 0.5);                                                                                                         \
                                                                                                                                                               \
	/* Set position to another than the origin */                                                                                                              \
	element->setPositionLogical(QPointF(0.85, 0.7));                                                                                                           \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 35.);                                                                                                          \
	VALUES_EQUAL(element->position().point.y(), 20.);                                                                                                          \
	VALUES_EQUAL(element->positionLogical().x(), 0.85);                                                                                                        \
	VALUES_EQUAL(element->positionLogical().y(), 0.7);                                                                                                         \
                                                                                                                                                               \
	p->shiftLeftX();                                                                                                                                           \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 25.); /* shift factor is 0.1 -> 1(current range)*0.1 = 0.1 = -10 in scene coords */                            \
	VALUES_EQUAL(element->position().point.y(), 20.);                                                                                                          \
	VALUES_EQUAL(element->positionLogical().x(), 0.85);                                                                                                        \
	VALUES_EQUAL(element->positionLogical().y(), 0.7);

#define WORKSHEETELEMENT_SHIFTY_COORDBINDING(element, dockSetElementsMethodName)                                                                               \
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      \
	auto pp = element->position();                                                                                                                             \
	pp.point = QPointF(0, 0);                                                                                                                                  \
	element->setPosition(pp);                                                                                                                                  \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                           \
	VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                           \
	VALUES_EQUAL(element->positionLogical().x(), 0.5);                                                                                                         \
	VALUES_EQUAL(element->positionLogical().y(), 0.5);                                                                                                         \
                                                                                                                                                               \
	element->setCoordinateBindingEnabled(true);                                                                                                                \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                           \
	VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                           \
	VALUES_EQUAL(element->positionLogical().x(), 0.5);                                                                                                         \
	VALUES_EQUAL(element->positionLogical().y(), 0.5);                                                                                                         \
                                                                                                                                                               \
	/* Set position to another than the origin */                                                                                                              \
	element->setPositionLogical(QPointF(0.85, 0.7));                                                                                                           \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 35.);                                                                                                          \
	VALUES_EQUAL(element->position().point.y(), 20.);                                                                                                          \
	VALUES_EQUAL(element->positionLogical().x(), 0.85);                                                                                                        \
	VALUES_EQUAL(element->positionLogical().y(), 0.7);                                                                                                         \
                                                                                                                                                               \
	p->shiftUpY();                                                                                                                                             \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 35.);                                                                                                          \
	VALUES_EQUAL(element->position().point.y(), 30.); /* shift factor is 0.1 -> 1(current range)*0.1 = 0.1 = +10 in scene coords (for UP) */                   \
	VALUES_EQUAL(element->positionLogical().x(), 0.85);                                                                                                        \
	VALUES_EQUAL(element->positionLogical().y(), 0.7);

#define WORKSHEETELEMENT_SHIFTX_NO_COORDBINDING(element, dockSetElementsMethodName)                                                                            \
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      \
	auto pp = element->position();                                                                                                                             \
	pp.point = QPointF(0, 0);                                                                                                                                  \
	element->setPosition(pp);                                                                                                                                  \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                           \
	VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                           \
	VALUES_EQUAL(element->positionLogical().x(), 0.5);                                                                                                         \
	VALUES_EQUAL(element->positionLogical().y(), 0.5);                                                                                                         \
                                                                                                                                                               \
	element->setCoordinateBindingEnabled(true);                                                                                                                \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                           \
	VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                           \
	VALUES_EQUAL(element->positionLogical().x(), 0.5);                                                                                                         \
	VALUES_EQUAL(element->positionLogical().y(), 0.5);                                                                                                         \
                                                                                                                                                               \
	/* Set position to another than the origin */                                                                                                              \
	element->setPositionLogical(QPointF(0.85, 0.7));                                                                                                           \
	element->setCoordinateBindingEnabled(false);                                                                                                               \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 35.);                                                                                                          \
	VALUES_EQUAL(element->position().point.y(), 20.);                                                                                                          \
	VALUES_EQUAL(element->positionLogical().x(), 0.85);                                                                                                        \
	VALUES_EQUAL(element->positionLogical().y(), 0.7);                                                                                                         \
                                                                                                                                                               \
	p->shiftLeftX();                                                                                                                                           \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 35.);                                                                                                          \
	VALUES_EQUAL(element->position().point.y(), 20.);                                                                                                          \
	VALUES_EQUAL(element->positionLogical().x(), 0.95); /* shift factor is 0.1 -> 1(current range)*0.1 = 0.1 */                                                \
	VALUES_EQUAL(element->positionLogical().y(), 0.7);

#define WORKSHEETELEMENT_SHIFTY_NO_COORDBINDING(element, dockSetElementsMethodName)                                                                            \
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      \
	auto pp = element->position();                                                                                                                             \
	pp.point = QPointF(0, 0);                                                                                                                                  \
	element->setPosition(pp);                                                                                                                                  \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                           \
	VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                           \
	VALUES_EQUAL(element->positionLogical().x(), 0.5);                                                                                                         \
	VALUES_EQUAL(element->positionLogical().y(), 0.5);                                                                                                         \
                                                                                                                                                               \
	element->setCoordinateBindingEnabled(true);                                                                                                                \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 0.);                                                                                                           \
	VALUES_EQUAL(element->position().point.y(), 0.);                                                                                                           \
	VALUES_EQUAL(element->positionLogical().x(), 0.5);                                                                                                         \
	VALUES_EQUAL(element->positionLogical().y(), 0.5);                                                                                                         \
                                                                                                                                                               \
	/* Set position to another than the origin */                                                                                                              \
	element->setPositionLogical(QPointF(0.85, 0.7));                                                                                                           \
	element->setCoordinateBindingEnabled(false);                                                                                                               \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 35.);                                                                                                          \
	VALUES_EQUAL(element->position().point.y(), 20.);                                                                                                          \
	VALUES_EQUAL(element->positionLogical().x(), 0.85);                                                                                                        \
	VALUES_EQUAL(element->positionLogical().y(), 0.7);                                                                                                         \
                                                                                                                                                               \
	p->shiftUpY();                                                                                                                                             \
                                                                                                                                                               \
	VALUES_EQUAL(element->position().point.x(), 35.);                                                                                                          \
	VALUES_EQUAL(element->position().point.y(), 20.);                                                                                                          \
	VALUES_EQUAL(element->positionLogical().x(), 0.85);                                                                                                        \
	VALUES_EQUAL(element->positionLogical().y(), 0.6); /* shift factor is 0.1 -> 1(current range)*0.1 = 0.1 */

/*
 * Check that when moving the element, the datetime is changed correctly and aligned with the datetime of the range
 * No Timezone problems
 * Using a datetime before 1970 because then toMSecsSinceEpoch() will be negative. To test if it works also there
 */
#define MOUSE_MOVE_DATETIME(element, dockSetElementsMethodName)                                                                                                \
	auto start = QDateTime::fromString(QLatin1String("1800-12-01 00:00:00:000Z"), QStringLiteral("yyyy-MM-dd hh:mm:ss:zzzt"));                                 \
	auto end = QDateTime::fromString(QLatin1String("1800-12-02 00:00:00:000Z"), QStringLiteral("yyyy-MM-dd hh:mm:ss:zzzt"));                                   \
	QVERIFY(start.isValid());                                                                                                                                  \
	QVERIFY(end.isValid());                                                                                                                                    \
	Range<double> dt(start.toMSecsSinceEpoch(), end.toMSecsSinceEpoch(), RangeT::Format::DateTime);                                                            \
	element->setCoordinateBindingEnabled(false); /* Disable binding to be sure that the point is still in the center of the plot when changing the range */    \
	p->setRange(Dimension::X, 0, dt);                                                                                                                          \
                                                                                                                                                               \
	dock->dockSetElementsMethodName({element});                                                                                                                \
                                                                                                                                                               \
	QCOMPARE(element->position().point.x(), 0.);                                                                                                               \
	element->setCoordinateBindingEnabled(true);                                                                                                                \
	QCOMPARE(element->d_ptr->pos().x(), 0);                                                                                                                    \
	QCOMPARE(dock->ui.dtePositionXLogical->dateTime().toString(QLatin1String("yyyy-MM-dd hh:mm:ss:zzz")), QLatin1String("1800-12-01 12:00:00:000"));           \
                                                                                                                                                               \
	/* 75% on between start and end -> so 3/4 * 24h = 18h */                                                                                                   \
	element->d_ptr->setPos(QPointF(25, -10)); /* item change will be called (negative value is up) */                                                          \
                                                                                                                                                               \
	QCOMPARE(element->positionLogical().x(),                                                                                                                   \
			 QDateTime::fromString(QLatin1String("1800-12-01 18:00:00:000Z"), QStringLiteral("yyyy-MM-dd hh:mm:ss:zzzt")).toMSecsSinceEpoch());                \
	QCOMPARE(dock->ui.dtePositionXLogical->dateTime().toString(QLatin1String("yyyy-MM-dd hh:mm:ss:zzz")), QLatin1String("1800-12-01 18:00:00:000"));

#define DOCK_CHANGE_DATETIME(element, dockSetElementsMethodName)                                                                                               \
	auto start = QDateTime::fromString(QLatin1String("1800-12-01 00:00:00:000Z"), QStringLiteral("yyyy-MM-dd hh:mm:ss:zzzt"));                                 \
	auto end = QDateTime::fromString(QLatin1String("1800-12-02 00:00:00:000Z"), QStringLiteral("yyyy-MM-dd hh:mm:ss:zzzt"));                                   \
	QVERIFY(start.isValid());                                                                                                                                  \
	QVERIFY(end.isValid());                                                                                                                                    \
	Range<double> dt(start.toMSecsSinceEpoch(), end.toMSecsSinceEpoch(), RangeT::Format::DateTime);                                                            \
	element->setCoordinateBindingEnabled(false); /* Disable binding to be sure that the point is still in the center of the plot when changing the range */    \
	p->setRange(Dimension::X, 0, dt);                                                                                                                          \
                                                                                                                                                               \
	dock->dockSetElementsMethodName({element});                                                                                                                \
                                                                                                                                                               \
	QCOMPARE(element->position().point.x(), 0.);                                                                                                               \
	element->setCoordinateBindingEnabled(true);                                                                                                                \
	QCOMPARE(element->d_ptr->pos().x(), 0);                                                                                                                    \
	QCOMPARE(dock->ui.dtePositionXLogical->dateTime().toString(QLatin1String("yyyy-MM-dd hh:mm:ss:zzz")), QLatin1String("1800-12-01 12:00:00:000"));           \
                                                                                                                                                               \
	/* Change hour by 1 */                                                                                                                                     \
	dock->ui.dtePositionXLogical->lineEdit()->setCursorPosition(13); /* behind the second hour digit */                                                        \
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);                                                                  \
	dock->ui.dtePositionXLogical->keyPressEvent(&event);                                                                                                       \
                                                                                                                                                               \
	QCOMPARE(element->positionLogical().x(),                                                                                                                   \
			 QDateTime::fromString(QLatin1String("1800-12-01 13:00:00:000Z"), QStringLiteral("yyyy-MM-dd hh:mm:ss:zzzt")).toMSecsSinceEpoch());                \
	QCOMPARE(dock->ui.dtePositionXLogical->dateTime().toString(QLatin1String("yyyy-MM-dd hh:mm:ss:zzz")), QLatin1String("1800-12-01 13:00:00:000"));

#endif // HELPERMACROS_H
