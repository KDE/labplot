/*
	File                 : SpinBoxTest.h
	Project              : LabPlot
	Description          : More tests for spinbox widgets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPINBOXTEST_H
#define SPINBOXTEST_H

#include "tests/CommonTest.h"

class SpinBoxTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase() {
		KLocalizedString::setApplicationDomain("labplot");
		QLocale::setDefault(QLocale(QLocale::Language::English));
	}

	void numberSpinBoxProperties();
	void numberSpinBoxCreateStringNumber();
	void numberSpinBoxChangingValueKeyPress();
	void numberSpinBoxLimit();
	void numberSpinBoxPrefixSuffix();
	void numberSpinBoxSuffixFrontToBackSelection();
	void numberSpinBoxSuffixSetCursor();
	void numberSpinBoxSuffixBackToFrontSelection();
	void numberSpinBoxPrefixFrontToBackSelection();
	void numberSpinBoxPrefixBackToFrontSelection();
	void numberSpinBoxPrefixSetCursorPosition();
	void numberSpinBoxEnterNumber();
	void numberSpinBoxFeedback();
	void numberSpinBoxFeedback2();
	void numberSpinBoxFeedbackCursorPosition();
	void numberSpinBoxFeedbackCursorPosition2();
	void numberSpinBoxDecimals2();
	void numberSpinBoxScrollingNegToPos();
	void numberSpinBoxScrollingNegToPos2();
	void numberSpinBoxScrollingNegativeValues();
	void numberSpinBoxMinimumFeedback();
	void numberSpinBoxDecimalsMinMax();

	void thousandSeparator();
	void thousandSeparatorNegative();
	void thousandSeparatorScrolling();
	void thousandSeparatorScrolling2();
	void thousandSeparatorScrollingSeparatorPosition();
};
#endif // SPINBOXTEST_H
