/*
	File                 : WidgetsTest.cpp
	Project              : LabPlot
	Description          : Tests for widgets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "WidgetsTest.h"
#include "commonfrontend/widgets/NumberSpinBox.h"

#include <QLineEdit>

void WidgetsTest::test1() {
	NumberSpinBox sb(nullptr);
	bool ok;

	sb.lineEdit()->setText("5.0");
	VALUES_EQUAL(sb.value(&ok), 5.0);

	sb.lineEdit()->setText("5e12");
	VALUES_EQUAL(sb.value(&ok), 5e12);

	sb.lineEdit()->setText("5.34890823e-3");
	VALUES_EQUAL(sb.value(&ok), 5.34890823e-3);

	sb.lineEdit()->setText("-5.34890823e-3");
	VALUES_EQUAL(sb.value(&ok), -5.34890823e-3);
}

void WidgetsTest::testChangingValueKeyPress() {
	NumberSpinBox sb(nullptr);
	bool ok;

	sb.lineEdit()->setText("3.34890823e-3");

	{
		sb.lineEdit()->setCursorPosition(0);
		QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(&ok), 2.34890823e-3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(&ok), 1.34890823e-3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(&ok), 0.34890823e-3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(&ok), 0.34890823e-3);
	}
}

QTEST_MAIN(WidgetsTest)
