/*
	File                 : WidgetsTest.h
	Project              : LabPlot
	Description          : Tests for widgets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WIDGETSTEST_H
#define WIDGETSTEST_H

#include "tests/CommonTest.h"

class WidgetsTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase() {
		QLocale::setDefault(QLocale(QLocale::Language::English));
	}
	void numberSpinBoxProperties();
	void numberSpinBoxCreateStringNumber();
	void numberSpinBoxChangingValueKeyPress();
	void numberSpinBoxLimit();
	void numberSpinBoxPrefixSuffix();
	void numberSpinBoxEnterNumber();
	void numberSpinBoxFeedback();
	void numberSpinBoxFeedback2();
	void numberSpinBoxDecimals();
	void numberSpinBoxScrollingNegToPos();
	void numberSpinBoxScrollingNegToPos2();
	void numberSpinBoxScrollingNegativeValues();
	void numberSpinBoxMinimumFeedback();
	void numberSpinBoxDecimalsMinMax();
};
#endif // WIDGETSTEST_H
