/*
	File                 : WorksheetTest.h
	Project              : LabPlot
	Description          : Tests for Worksheets and positioning them on the plot
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
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_RIGHT);                                                                   \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEY_PRESSLEFT);                                                                    \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_UP);                                                                      \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_KEYPRESS_DOWN);                                                                    \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_ENABLE_DISABLE_COORDBINDING);                                                      \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_COORDBINDING);                                                              \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_COORDBINDING);                                                              \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_SHIFTX_NO_COORDBINDING);                                                           \
	WORKSHEETELEMENT_TEST_DEFINITION(WorksheetElementType, WORKSHEETELEMENT_SHIFTY_NO_COORDBINDING);

class WorksheetElementTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	ALL_WORKSHEET_TESTS_DEFINITION(CustomPoint)
	ALL_WORKSHEET_TESTS_DEFINITION(TextLabel)
	ALL_WORKSHEET_TESTS_DEFINITION(Image)

	void referenceRangeXMouseMove();
	void referenceRangeYMouseMove();

	void referenceRangeXClippingLeftMouse();
	void referenceRangeXClippingLeftSetStart();
	void referenceRangeXClippingRightSetEnd();

	void referenceRangeYClippingBottomSetEnd();
	void referenceRangeYClippingTopSetEnd();

	void referenceRangeYKeyPressUp();
};

#endif // WORKSHEETELEMENTTEST_H
