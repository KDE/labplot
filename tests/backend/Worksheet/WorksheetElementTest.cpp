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
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/plots/cartesian/ReferenceLinePrivate.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"
#include "backend/worksheet/plots/cartesian/ReferenceRangePrivate.h"
#include "frontend/ProjectExplorer.h"
#include "frontend/dockwidgets/CustomPointDock.h"
#include "frontend/dockwidgets/ImageDock.h"
#include "frontend/widgets/LabelWidget.h"

#include <QAction>
#include <QItemSelectionModel>
#include <QMenu>
#include <QTreeView>

#define ALL_WORKSHEETELEMENT_TESTS(WorksheetElementType, DockType, dockSetElementsMethodName)                                                                  \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_SETPOSITIONLOGICAL, DockType, dockSetElementsMethodName)                                      \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_MOUSE_MOVE, DockType, dockSetElementsMethodName)                                              \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT_UNDO, DockType, dockSetElementsMethodName)                                     \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_UP_UNDO, DockType, dockSetElementsMethodName)                                        \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT_NO_COORD_BINDING, DockType, dockSetElementsMethodName)                         \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_DOWN_NO_COORD_BINDING, DockType, dockSetElementsMethodName)                          \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT, DockType, dockSetElementsMethodName)                                          \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEY_PRESSLEFT, DockType, dockSetElementsMethodName)                                           \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_UP, DockType, dockSetElementsMethodName)                                             \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_DOWN, DockType, dockSetElementsMethodName)                                           \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_ENABLE_DISABLE_COORDBINDING, DockType, dockSetElementsMethodName)                             \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_COORDBINDING, DockType, dockSetElementsMethodName)                                     \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_COORDBINDING, DockType, dockSetElementsMethodName)                                     \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_NO_COORDBINDING, DockType, dockSetElementsMethodName)                                  \
	WORKSHEETELEMENT_TEST(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_NO_COORDBINDING, DockType, dockSetElementsMethodName)                                  \
	WORKSHEETELEMENT_TEST(WorksheetElementType, MOUSE_MOVE_DATETIME, DockType, dockSetElementsMethodName)                                                      \
	WORKSHEETELEMENT_TEST(WorksheetElementType, DOCK_CHANGE_DATETIME, DockType, dockSetElementsMethodName)

#define ALL_WORKSHEETELEMENT_TESTS2(WorksheetElementType, DockType, dockSetElementsMethodName)                                                                 \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_SETPOSITIONLOGICAL, DockType, dockSetElementsMethodName)                                     \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_MOUSE_MOVE, DockType, dockSetElementsMethodName)                                             \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT_UNDO, DockType, dockSetElementsMethodName)                                    \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_UP_UNDO, DockType, dockSetElementsMethodName)                                       \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT_NO_COORD_BINDING, DockType, dockSetElementsMethodName)                        \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_DOWN_NO_COORD_BINDING, DockType, dockSetElementsMethodName)                         \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT, DockType, dockSetElementsMethodName)                                         \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEY_PRESSLEFT, DockType, dockSetElementsMethodName)                                          \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_UP, DockType, dockSetElementsMethodName)                                            \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_DOWN, DockType, dockSetElementsMethodName)                                          \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_ENABLE_DISABLE_COORDBINDING, DockType, dockSetElementsMethodName)                            \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_COORDBINDING, DockType, dockSetElementsMethodName)                                    \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_COORDBINDING, DockType, dockSetElementsMethodName)                                    \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_NO_COORDBINDING, DockType, dockSetElementsMethodName)                                 \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_NO_COORDBINDING, DockType, dockSetElementsMethodName)                                 \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, MOUSE_MOVE_DATETIME, DockType, dockSetElementsMethodName)                                                     \
	WORKSHEETELEMENT_TEST2(WorksheetElementType, DOCK_CHANGE_DATETIME, DockType, dockSetElementsMethodName)

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

ALL_WORKSHEETELEMENT_TESTS(CustomPoint, CustomPointDock, setPoints)
ALL_WORKSHEETELEMENT_TESTS2(TextLabel, LabelWidget, setLabels)
ALL_WORKSHEETELEMENT_TESTS2(Image, ImageDock, setImages)

/*!
 * \brief create and add a new ReferenceRange, undo and redo this step
 */
void WorksheetElementTest::testReferenceRangeInit() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(plot);

	auto* range = new ReferenceRange(plot, QStringLiteral("range"));
	plot->addChild(range);

	auto children = plot->children<ReferenceRange>();

	QCOMPARE(children.size(), 1);

	project.undoStack()->undo();
	children = plot->children<ReferenceRange>();
	QCOMPARE(children.size(), 0);

	project.undoStack()->redo();
	children = plot->children<ReferenceRange>();
	QCOMPARE(children.size(), 1);
}

/*!
 * \brief create and add a new ReferenceRange, duplicate it and check the number of children
 */
void WorksheetElementTest::testReferenceRangeDuplicate() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(plot);

	auto* range = new ReferenceRange(plot, QStringLiteral("range"));
	plot->addChild(range);

	range->duplicate();

	auto children = plot->children<ReferenceRange>();
	QCOMPARE(children.size(), 2);

	project.undoStack()->undo();
	children = plot->children<ReferenceRange>();
	QCOMPARE(children.size(), 1);
}

void WorksheetElementTest::referenceRangeXMouseMove() {
	SETUP_PROJECT

	auto* referenceRange = new ReferenceRange(p, QStringLiteral("range"));
	referenceRange->setOrientation(ReferenceRange::Orientation::Vertical);
	p->addChild(referenceRange);
	referenceRange->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	referenceRange->setPositionLogical(QPointF(0.5, 0.5));

	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.45);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.55);
	CHECK_REFERENCERANGE_RECT(referenceRange, 0.45, 1., 0.55, 0.);

	// Simulate mouse move
	referenceRange->d_ptr->setPos(QPointF(25, -10)); // item change will be called (negative value is up)
	QCOMPARE(referenceRange->positionLogical().x(), 0.75); // 25/50 * 0.5 + 0.5
	QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.7);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.8);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0.7, 1., 0.8, 0.);

	// Set Logical Start
	referenceRange->setPositionLogicalStart(QPointF(0.1, 0.5));
	QCOMPARE(referenceRange->positionLogical().x(), 0.45);
	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.8);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0.1, 1., 0.8, 0.);

	// Set Logical End
	referenceRange->setPositionLogicalEnd(QPointF(0.3, 0.5));
	QCOMPARE(referenceRange->positionLogical().x(), 0.2);
	QCOMPARE(referenceRange->positionLogical().y(), 0.5);
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.3);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0.1, 1., 0.3, 0.);
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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 0.55, 1., 0.45);

	// Simulate mouse move
	referenceRange->d_ptr->setPos(QPointF(-10, -25)); // item change will be called (negative value is up)
	QCOMPARE(referenceRange->positionLogical().y(), 0.75); // 25/50 * 0.5 + 0.5
	QCOMPARE(referenceRange->positionLogical().x(), 0.5); // Only vertical considered
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.7);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.8);
	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 0.8, 1., 0.7);

	// Set Logical Start
	referenceRange->setPositionLogicalStart(QPointF(0.5, 0.1));
	QCOMPARE(referenceRange->positionLogical().y(), 0.45); // (0.8 + 0.1)/2
	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.8);
	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 0.8, 1., 0.1);
	// Set Logical End
	referenceRange->setPositionLogicalEnd(QPointF(0.5, 0.3));
	QCOMPARE(referenceRange->positionLogical().y(), 0.2); // (0.3 + 0.1)/2
	QCOMPARE(referenceRange->positionLogical().x(), 0.5);
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.1);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.3);
	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 0.3, 1., 0.1);
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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0.45, 1., 0.55, 0.);

	// Simulate mouse move
	referenceRange->d_ptr->setPos(QPointF(-50, -10)); // item change will be called (negative value is up)
	QCOMPARE(referenceRange->positionLogical().x(), 0.0);
	QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().x(), -0.05);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.05);
	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 1., 0.05, 0.);
}

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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0.45, 1., 0.55, 0.);

	referenceRange->setPositionLogicalStart(QPointF(-5, 0.5));
	QCOMPARE(referenceRange->positionLogical().x(), -2.225); // (-5+0.55)/2
	QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().x(), -5.);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 0.55);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 1., 0.55, 0.);
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

	CHECK_REFERENCERANGE_RECT(referenceRange, 0.45, 1., 0.55, 0.);

	// Simulate mouse move
	referenceRange->setPositionLogicalEnd(QPointF(+5, 0.5));
	QCOMPARE(referenceRange->positionLogical().x(), 2.725); // (5+0.45)/2
	QCOMPARE(referenceRange->positionLogical().y(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().x(), 0.45);
	QCOMPARE(referenceRange->positionLogicalEnd().x(), 5.);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0.45, 1., 1., 0.); // Xend is clipped
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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 0.55, 1., 0.45);

	// Simulate mouse move
	referenceRange->setPositionLogicalStart(QPointF(0.5, -5.));
	QCOMPARE(referenceRange->positionLogical().y(), -2.225); // (-5+0.45)/2
	QCOMPARE(referenceRange->positionLogical().x(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().y(), -5.);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.55);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 0.55, 1., 0.);
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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 0.55, 1., 0.45);

	// Simulate mouse move
	referenceRange->setPositionLogicalEnd(QPointF(0.5, 5.));
	QCOMPARE(referenceRange->positionLogical().y(), 2.725); // (5+0.45)/2
	QCOMPARE(referenceRange->positionLogical().x(), 0.5); // Only horizontal considered
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.45);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 5.);

	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 1., 1., 0.45);
}

void WorksheetElementTest::referenceRangeYKeyPressUp() {
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
	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 0.55, 1., 0.45);

	// Simulate mouse move
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	referenceRange->d_ptr->keyPressEvent(&event); // go up 5 in scene coordinates
	QCOMPARE(referenceRange->positionLogical().y(), 0.55); // 5/50 * 0.5 + 0.5
	QCOMPARE(referenceRange->positionLogical().x(), 0.5); // Only vertical considered
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.5);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.6);
	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 0.6, 1., 0.5);

	referenceRange->d_ptr->keyPressEvent(&event); // go up 5 in scene coordinates
	QCOMPARE(referenceRange->positionLogical().y(), 0.6); // 5/50 * 0.5 + 0.5
	QCOMPARE(referenceRange->positionLogical().x(), 0.5); // Only vertical considered
	QCOMPARE(referenceRange->positionLogicalStart().y(), 0.55);
	QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.65);
	CHECK_REFERENCERANGE_RECT(referenceRange, 0., 0.65, 1., 0.55);
}

void WorksheetElementTest::referenceRangeSaveLoad() {
	QString savePath;
	{
		SETUP_PROJECT

		auto* referenceRange = new ReferenceRange(p, QStringLiteral("range"));
		referenceRange->setOrientation(ReferenceRange::Orientation::Horizontal);
		p->addChild(referenceRange);
		referenceRange->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
		auto pp = referenceRange->position();
		pp.point = QPointF(0, 0);
		referenceRange->setPosition(pp);
		referenceRange->setCoordinateBindingEnabled(true);
		SAVE_PROJECT("testReferenceRangeSaveLoad");
	}

	{
		Project project;
		QCOMPARE(project.load(savePath), true); // shall not crash

		const auto* ws = project.child<Worksheet>(0);
		QVERIFY(ws);
		const auto* p = ws->child<CartesianPlot>(0);
		QVERIFY(p);
		const auto* referenceRange = p->child<ReferenceRange>(0);
		QVERIFY(referenceRange);

		QCOMPARE(referenceRange->positionLogical().x(), 0.5);
		QCOMPARE(referenceRange->positionLogical().y(), 0.5);
		QCOMPARE(referenceRange->positionLogicalStart().y(), 0.45);
		QCOMPARE(referenceRange->positionLogicalEnd().y(), 0.55);
		CHECK_REFERENCERANGE_RECT(referenceRange, 0., 0.55, 1., 0.45);
	}
}

/*!
 * \brief create and add a new ReferenceLine, undo and redo this step
 */
void WorksheetElementTest::testReferenceLineInit() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(plot);

	auto* line = new ReferenceLine(plot, QStringLiteral("line"));
	plot->addChild(line);

	auto children = plot->children<ReferenceLine>();

	QCOMPARE(children.size(), 1);

	project.undoStack()->undo();
	children = plot->children<ReferenceLine>();
	QCOMPARE(children.size(), 0);

	project.undoStack()->redo();
	children = plot->children<ReferenceLine>();
	QCOMPARE(children.size(), 1);
}

/*!
 * \brief create and add a new ReferenceLine, duplicate it and check the number of children
 */
void WorksheetElementTest::testReferenceLineDuplicate() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(plot);

	auto* line = new ReferenceLine(plot, QStringLiteral("line"));
	plot->addChild(line);

	line->duplicate();

	auto children = plot->children<ReferenceLine>();
	QCOMPARE(children.size(), 2);

	project.undoStack()->undo();
	children = plot->children<ReferenceLine>();
	QCOMPARE(children.size(), 1);
}

void WorksheetElementTest::referenceLineLinearScaling() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(true);

	const auto& axes = plot->children<Axis>();
	QCOMPARE(axes.length(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* xAxis = axes.at(0);

	CHECK_RANGE(plot, xAxis, Dimension::X, 0., 1.);
	CHECK_RANGE(plot, xAxis, Dimension::Y, 0., 1.);

	plot->setRangeScale(Dimension::X, 0, RangeT::Scale::Linear);

	CHECK_RANGE(plot, xAxis, Dimension::X, 0., 1.);
	CHECK_RANGE(plot, xAxis, Dimension::Y, 0., 1.);

	auto* referenceLine = new ReferenceLine(plot, QStringLiteral("TestLine"));
	referenceLine->setOrientation(ReferenceLine::Orientation::Horizontal);
	plot->addChild(referenceLine);
	referenceLine->retransform();

	const auto& rect = plot->dataRect();

	QCOMPARE(qAbs(referenceLine->d_func()->length), rect.width());
	QCOMPARE(referenceLine->d_func()->pos().x(), rect.center().x());
}

void WorksheetElementTest::referenceLineLog10Scaling() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(true);

	const auto& axes = plot->children<Axis>();
	QCOMPARE(axes.length(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* xAxis = axes.at(0);

	CHECK_RANGE(plot, xAxis, Dimension::X, 0., 1.);
	CHECK_RANGE(plot, xAxis, Dimension::Y, 0., 1.);

	plot->setRangeScale(Dimension::X, 0, RangeT::Scale::Log10);

	CHECK_RANGE(plot, xAxis, Dimension::X, 0.01, 1.);
	CHECK_RANGE(plot, xAxis, Dimension::Y, 0., 1.);

	auto* referenceLine = new ReferenceLine(plot, QStringLiteral("TestLine"));
	referenceLine->setOrientation(ReferenceLine::Orientation::Horizontal);
	plot->addChild(referenceLine);
	referenceLine->retransform();

	const auto& rect = plot->dataRect();

	QCOMPARE(qAbs(referenceLine->d_func()->length), rect.width());
	QCOMPARE(referenceLine->d_func()->pos().x(), rect.center().x());
}

void WorksheetElementTest::referenceLineSquareScaling() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(true);

	const auto& axes = plot->children<Axis>();
	QCOMPARE(axes.length(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* xAxis = axes.at(0);

	CHECK_RANGE(plot, xAxis, Dimension::X, 0., 1.);
	CHECK_RANGE(plot, xAxis, Dimension::Y, 0., 1.);

	plot->setRangeScale(Dimension::X, 0, RangeT::Scale::Square);

	CHECK_RANGE(plot, xAxis, Dimension::X, 0.01, 1.);
	CHECK_RANGE(plot, xAxis, Dimension::Y, 0., 1.);

	auto* referenceLine = new ReferenceLine(plot, QStringLiteral("TestLine"));
	referenceLine->setOrientation(ReferenceLine::Orientation::Horizontal);
	plot->addChild(referenceLine);
	referenceLine->retransform();

	const auto& rect = plot->dataRect();

	QCOMPARE(qAbs(referenceLine->d_func()->length), rect.width());
	QCOMPARE(referenceLine->d_func()->pos().x(), rect.center().x());
}

void WorksheetElementTest::referenceLineSqrtScaling() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(true);

	const auto& axes = plot->children<Axis>();
	QCOMPARE(axes.length(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* xAxis = axes.at(0);

	CHECK_RANGE(plot, xAxis, Dimension::X, 0., 1.);
	CHECK_RANGE(plot, xAxis, Dimension::Y, 0., 1.);

	plot->setRangeScale(Dimension::X, 0, RangeT::Scale::Sqrt);

	CHECK_RANGE(plot, xAxis, Dimension::X, 0., 1.);
	CHECK_RANGE(plot, xAxis, Dimension::Y, 0., 1.);

	auto* referenceLine = new ReferenceLine(plot, QStringLiteral("TestLine"));
	referenceLine->setOrientation(ReferenceLine::Orientation::Horizontal);
	plot->addChild(referenceLine);
	referenceLine->retransform();

	const auto& rect = plot->dataRect();

	QCOMPARE(qAbs(referenceLine->d_func()->length), rect.width());
	QCOMPARE(referenceLine->d_func()->pos().x(), rect.center().x());
}

void WorksheetElementTest::referenceLineInverseScaling() {
	Project project;

	auto* worksheet = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);
	plot->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	plot->setNiceExtend(true);

	const auto& axes = plot->children<Axis>();
	QCOMPARE(axes.length(), 2);
	QCOMPARE(axes.at(0)->name(), QStringLiteral("x"));
	QCOMPARE(axes.at(1)->name(), QStringLiteral("y"));
	auto* xAxis = axes.at(0);

	CHECK_RANGE(plot, xAxis, Dimension::X, 0., 1.);
	CHECK_RANGE(plot, xAxis, Dimension::Y, 0., 1.);

	plot->setRangeScale(Dimension::X, 0, RangeT::Scale::Sqrt);

	CHECK_RANGE(plot, xAxis, Dimension::X, 0.0, 1.);
	CHECK_RANGE(plot, xAxis, Dimension::Y, 0., 1.);

	auto* referenceLine = new ReferenceLine(plot, QStringLiteral("TestLine"));
	referenceLine->setOrientation(ReferenceLine::Orientation::Horizontal);
	plot->addChild(referenceLine);
	referenceLine->retransform();

	const auto& rect = plot->dataRect();

	QCOMPARE(qAbs(referenceLine->d_func()->length), rect.width());
	QCOMPARE(referenceLine->d_func()->pos().x(), rect.center().x());
}

#define DEBUG_ELEMENT_NAMES(aspectVector)                                                                                                                      \
	do {                                                                                                                                                       \
		int index = 0;                                                                                                                                         \
		for (const auto& a : std::as_const(aspectVector)) {                                                                                                    \
			DEBUG(std::string("Index: ") << index << " , name: " << a->name().toStdString());                                                                  \
			index++;                                                                                                                                           \
		}                                                                                                                                                      \
	} while (false)

void WorksheetElementTest::moveElementBefore() {
	Project project;

	AspectTreeModel treemodel(&project, this);

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* lFirst = new TextLabel(QStringLiteral("first"));
	ws->addChild(lFirst);

	auto* lSecond = new TextLabel(QStringLiteral("second"));
	ws->addChild(lSecond);

	auto* lThird = new TextLabel(QStringLiteral("third"));
	ws->addChild(lThird);

	auto children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);

	// First is background
	QCOMPARE(children.at(1)->name(), lFirst->name());
	QCOMPARE(children.at(2)->name(), lSecond->name());
	QCOMPARE(children.at(3)->name(), lThird->name());

	QAction action;
	action.setData(2); // behind lSecond
	lThird->execMoveBehind(&action);

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lFirst->name());
	QCOMPARE(children.at(2)->name(), lThird->name());
	QCOMPARE(children.at(3)->name(), lSecond->name());

	lThird->undoStack()->undo();

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lFirst->name());
	QCOMPARE(children.at(2)->name(), lSecond->name());
	QCOMPARE(children.at(3)->name(), lThird->name());

	action.setData(1);
	lThird->execMoveBehind(&action);

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lThird->name());
	QCOMPARE(children.at(2)->name(), lFirst->name());
	QCOMPARE(children.at(3)->name(), lSecond->name());

	action.setData(1);
	lSecond->execMoveBehind(&action);

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lSecond->name());
	QCOMPARE(children.at(2)->name(), lThird->name());
	QCOMPARE(children.at(3)->name(), lFirst->name());

	lSecond->undoStack()->undo();

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lThird->name());
	QCOMPARE(children.at(2)->name(), lFirst->name());
	QCOMPARE(children.at(3)->name(), lSecond->name());

	lSecond->undoStack()->redo();

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lSecond->name());
	QCOMPARE(children.at(2)->name(), lThird->name());
	QCOMPARE(children.at(3)->name(), lFirst->name());
}

void WorksheetElementTest::moveElementAfter() {
	Project project;

	AspectTreeModel treemodel(&project, this);

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* lFirst = new TextLabel(QStringLiteral("first"));
	ws->addChild(lFirst);

	auto* lSecond = new TextLabel(QStringLiteral("second"));
	ws->addChild(lSecond);

	auto* lThird = new TextLabel(QStringLiteral("third"));
	ws->addChild(lThird);

	auto children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);

	// First is background
	QCOMPARE(children.at(1)->name(), lFirst->name());
	QCOMPARE(children.at(2)->name(), lSecond->name());
	QCOMPARE(children.at(3)->name(), lThird->name());

	QAction action;
	action.setData(3);
	lFirst->execMoveInFrontOf(&action);

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lSecond->name());
	QCOMPARE(children.at(2)->name(), lThird->name());
	QCOMPARE(children.at(3)->name(), lFirst->name());

	lThird->undoStack()->undo();

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lFirst->name());
	QCOMPARE(children.at(2)->name(), lSecond->name());
	QCOMPARE(children.at(3)->name(), lThird->name());

	action.setData(400); // higher than maximum, it will be limited automatically
	lFirst->execMoveInFrontOf(&action);

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lSecond->name());
	QCOMPARE(children.at(2)->name(), lThird->name());
	QCOMPARE(children.at(3)->name(), lFirst->name());

	action.setData(2); // in front of lThird
	lSecond->execMoveInFrontOf(&action);

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lThird->name());
	QCOMPARE(children.at(2)->name(), lSecond->name());
	QCOMPARE(children.at(3)->name(), lFirst->name());

	lSecond->undoStack()->undo();

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lSecond->name());
	QCOMPARE(children.at(2)->name(), lThird->name());
	QCOMPARE(children.at(3)->name(), lFirst->name());

	lSecond->undoStack()->redo();

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(children.at(1)->name(), lThird->name());
	QCOMPARE(children.at(2)->name(), lSecond->name());
	QCOMPARE(children.at(3)->name(), lFirst->name());
}

void WorksheetElementTest::prepareDrawingMenu() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* lFirst = new TextLabel(QStringLiteral("first"));
	ws->addChild(lFirst);

	auto* lSecond = new TextLabel(QStringLiteral("second"));
	ws->addChild(lSecond);

	auto* lThird = new TextLabel(QStringLiteral("third"));
	ws->addChild(lThird);

	auto children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);

	// First is background
	QCOMPARE(children.at(1)->name(), lFirst->name());
	QCOMPARE(children.at(2)->name(), lSecond->name());
	QCOMPARE(children.at(3)->name(), lThird->name());

	QAction action;
	action.setData(1);
	lThird->execMoveBehind(&action);

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	// DEBUG_ELEMENT_NAMES(children);
	lThird->createContextMenu();
	lThird->prepareDrawingOrderMenu();
	QCOMPARE(lThird->m_moveBehindMenu->actions().count(), 0);

	QCOMPARE(lThird->m_moveInFrontOfMenu->actions().count(), 2);
}

void WorksheetElementTest::moveTreeModelInteraction() {
	Project project;
	AspectTreeModel aspectTreeModel(&project, this);

	std::unique_ptr<ProjectExplorer> projectExplorer = std::make_unique<ProjectExplorer>(nullptr);
	projectExplorer->setModel(&aspectTreeModel);
	projectExplorer->setProject(&project);
	projectExplorer->setCurrentAspect(&project);

	auto* ws = new Worksheet(QStringLiteral("Worksheet"));
	project.addChild(ws);

	auto* lFirst = new TextLabel(QStringLiteral("first"));
	ws->addChild(lFirst);

	auto* lSecond = new TextLabel(QStringLiteral("second"));
	ws->addChild(lSecond);

	auto* lThird = new TextLabel(QStringLiteral("third"));
	ws->addChild(lThird);

	auto children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);

	// First is background
	QCOMPARE(children.at(1)->name(), lFirst->name());
	QCOMPARE(children.at(2)->name(), lSecond->name());
	QCOMPARE(children.at(3)->name(), lThird->name());

	QAction action;
	action.setData(1);
	lThird->execMoveBehind(&action);

	ws->setItemSelectedInView(lFirst->graphicsItem(), false);
	ws->setItemSelectedInView(lThird->graphicsItem(), true);
	const auto& indices = projectExplorer->m_treeView->selectionModel()->selectedIndexes();
	QCOMPARE(indices.length(), 4);
	const auto& aspectIndex = indices.at(0);
	const auto* selectedAspect = static_cast<AbstractAspect*>(aspectIndex.internalPointer());
	QCOMPARE(selectedAspect->name(), lThird->name()); // The selectionModel() got updated correctly

	children = ws->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	// DEBUG_ELEMENT_NAMES(children);
	lThird->createContextMenu();
	lThird->prepareDrawingOrderMenu();
	QCOMPARE(lThird->m_moveBehindMenu->actions().count(), 0);
	QCOMPARE(lThird->m_moveInFrontOfMenu->actions().count(), 2);
}

void WorksheetElementTest::mouseMoveRelative() {
	// WORKSHEETELEMENT_TEST()
	SETUP_PROJECT
	auto* element = new TextLabel(QStringLiteral("element"));                                                                                
	p->addChild(element);                                                                                                                                  
	auto* dock = new LabelWidget(nullptr);                                                                                                                    
	//MACRO_NAME(element, dockSetElementsMethodName);
	dock->setLabels({element});


	// #define WORKSHEETELEMENT_MOUSE_MOVE(element, dockSetElementsMethodName)                                                                                        
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      
	auto pp = element->position();                                                                                                                             
	pp.point = QPointF(0, 0);                                                                                                                                  
	element->setPosition(pp);                                                                                                                                  
	element->setCoordinateBindingEnabled(true);                                                                                                                
                                                                                                                                                               
	QCOMPARE(element->positionLogical().x(), 0.5);                                                                                                             
	QCOMPARE(element->positionLogical().y(), 0.5);

	QCOMPARE(element->horizontalAlignment(), HorizontalAlignment::Center);
	QCOMPARE(element->verticalAlignment(), VerticalAlignment::Center);

	pp = element->position();
	pp.horizontalPosition = HorizontalPosition::Relative;
	pp.verticalPosition = VerticalPosition::Relative;
	element->setPosition(pp);

	QCOMPARE(element->positionLogical().x(), 0.5);                                                                                                             
	QCOMPARE(element->positionLogical().y(), 0.5);                                                                                                    
                                                                                                                                                               
	/* Simulate mouse move */                                                                                                                                  
	element->d_ptr->setPos(QPointF(25, -10)); /* item change will be called (negative value is up) */                                                          
	QCOMPARE(element->positionLogical().x(), 0.75); /* 25/50 * 0.5 + 0.5 */                                                                                    
	QCOMPARE(element->positionLogical().y(), 0.6);

	QCOMPARE(dock->ui.sbPositionX->value(), 0.75 * 100.0);
	QCOMPARE(dock->ui.sbPositionY->value(), 0.6 * 100.0); 
}

void WorksheetElementTest::positionRelative() {
		// WORKSHEETELEMENT_TEST()
	SETUP_PROJECT
	auto* element = new TextLabel(QStringLiteral("element"));                                                                                
	p->addChild(element);                                                                                                                                  
	auto* dock = new LabelWidget(nullptr);                                                                                                                    
	//MACRO_NAME(element, dockSetElementsMethodName); 
	dock->setLabels({element});

	// #define WORKSHEETELEMENT_MOUSE_MOVE(element, dockSetElementsMethodName)                                                                                        
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      
	auto pp = element->position();                                                                                                                             
	pp.point = QPointF(0, 0);                                                                                                                                  
	element->setPosition(pp);                                                                                                                                  
	element->setCoordinateBindingEnabled(true);                                                                                                                
                                                                                                                                                               
	QCOMPARE(element->positionLogical().x(), 0.5);                                                                                                             
	QCOMPARE(element->positionLogical().y(), 0.5);

	QCOMPARE(element->horizontalAlignment(), HorizontalAlignment::Center);
	QCOMPARE(element->verticalAlignment(), VerticalAlignment::Center);

	pp = element->position();
	pp.horizontalPosition = HorizontalPosition::Relative;
	pp.verticalPosition = VerticalPosition::Relative;
	pp.point = QPointF(0.2, 0.8) // 20% and 80%
	element->setPosition(pp);

	QCOMPARE(element->positionLogical().x(), 0.2);                                                                                                             
	QCOMPARE(element->positionLogical().y(), 0.8);

	QCOMPARE(dock->ui.sbPositionX->value(), 0.2 * 100.0);
	QCOMPARE(dock->ui.sbPositionY->value(), 0.8 * 100.0);                                                                                                 	
}

void WorksheetElementTest::positionRelativeDock() {
		// WORKSHEETELEMENT_TEST()
	SETUP_PROJECT
	auto* element = new TextLabel(QStringLiteral("element"));                                                                                
	p->addChild(element);                                                                                                                                  
	auto* dock = new LabelWidget(nullptr);                                                                                                                    
	//MACRO_NAME(element, dockSetElementsMethodName); 
	dock->setLabels({element});

	// #define WORKSHEETELEMENT_MOUSE_MOVE(element, dockSetElementsMethodName)                                                                                        
	element->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());                                                                                      
	auto pp = element->position();                                                                                                                             
	pp.point = QPointF(0, 0);                                                                                                                                  
	element->setPosition(pp);                                                                                                                                  
	element->setCoordinateBindingEnabled(true);                                                                                                                
                                                                                                                                                               
	QCOMPARE(element->positionLogical().x(), 0.5);                                                                                                             
	QCOMPARE(element->positionLogical().y(), 0.5);

	QCOMPARE(element->horizontalAlignment(), WorksheetElement::HorizontalAlignment::Center);
	QCOMPARE(element->verticalAlignment(), WorksheetElement::VerticalAlignment::Center);

	element->setPositionHorizontal(WorksheetElement::HorizontalAlignment::Relative);
	element->setPositionVertical(WorksheetElement::VerticalAlignment::Relative);

	// Set values in dock
	dock->ui.sbPositionX->setValue(20);
	dock->ui.sbPositionX->setValue(80);

	QCOMPARE(element->positionLogical().x(), 0.2);                                                                                                       
	QCOMPARE(element->positionLogical().y(), 0.8);

	QCOMPARE(dock->ui.sbPositionX->value(), 0.2 * 100.0);
	QCOMPARE(dock->ui.sbPositionY->value(), 0.8 * 100.0);       	
}

// TODO: create test with reference range with nonlinear ranges!
// Zooming in cartesianplot leads to move the worksheetelement
// Testing without setCoordinateBindingEnabled
// Undo redo of moving the position, keyboard, mousemove, manual setting

QTEST_MAIN(WorksheetElementTest)
