/*
	File                 : WorksheetElementTest.h
	Project              : LabPlot
	Description          : Tests for WorksheetElements and positioning them on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETELEMENTTEST_H
#define WORKSHEETELEMENTTEST_H

#include "../../CommonTest.h"
#include "helperMacros.h"

#define ALL_WORKSHEET_TESTS_DEFINITION(WorksheetElementType)                                                                                                   \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_SETPOSITIONLOGICAL);                                                               \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_MOUSE_MOVE);                                                                       \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT_UNDO);                                                              \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_UP_UNDO);                                                                 \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT_NO_COORD_BINDING);                                                  \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_DOWN_NO_COORD_BINDING);                                                   \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT);                                                                   \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEY_PRESSLEFT);                                                                    \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_UP);                                                                      \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_DOWN);                                                                    \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_ENABLE_DISABLE_COORDBINDING);                                                      \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_COORDBINDING);                                                              \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_COORDBINDING);                                                              \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_NO_COORDBINDING);                                                           \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_NO_COORDBINDING);                                                           \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, MOUSE_MOVE_DATETIME);                                                                               \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, DOCK_CHANGE_DATETIME);

class WorksheetElementTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	ALL_WORKSHEET_TESTS_DEFINITION(CustomPoint)
	ALL_WORKSHEET_TESTS_DEFINITION(TextLabel)
	ALL_WORKSHEET_TESTS_DEFINITION(Image)

	// tests for ReferenceRange
	void testReferenceRangeInit();
	void testReferenceRangeDuplicate();
	void referenceRangeXMouseMove();
	void referenceRangeYMouseMove();
	void referenceRangeXClippingLeftMouse();
	void referenceRangeXClippingLeftSetStart();
	void referenceRangeXClippingRightSetEnd();
	void referenceRangeYClippingBottomSetEnd();
	void referenceRangeYClippingTopSetEnd();
	void referenceRangeYKeyPressUp();
	void referenceRangeSaveLoad();

	// tests for ReferenceLine
	void testReferenceLineInit();
	void testReferenceLineDuplicate();
	void referenceLineLinearScaling();
	void referenceLineLog10Scaling();
	void referenceLineSquareScaling();
	void referenceLineSqrtScaling();
	void referenceLineInverseScaling();

	// general tests
	void moveElementBefore();
	void moveElementAfter();
	void prepareDrawingMenu();

	void moveTreeModelInteraction();

	// relative positioning
	void mouseMoveRelative();
	void positionRelative();
	void positionRelativeDock();
};

#endif // WORKSHEETELEMENTTEST_H
