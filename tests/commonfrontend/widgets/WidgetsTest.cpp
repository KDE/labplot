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

#define VALUES_EQUAL_OK(v, ref) \
    do { \
    const auto value = v; \
    QVERIFY(ok); \
    VALUES_EQUAL(value, ref); \
    } while (false)

void WidgetsTest::test1() {
	NumberSpinBox sb(nullptr);
    bool ok{false};

	sb.lineEdit()->setText("5.0");
    VALUES_EQUAL_OK(sb.value(&ok), 5.0);

	sb.lineEdit()->setText("5e12");
    VALUES_EQUAL_OK(sb.value(&ok), 5e12);

	sb.lineEdit()->setText("5.34890823e-3");
    VALUES_EQUAL_OK(sb.value(&ok), 5.34890823e-3);

	sb.lineEdit()->setText("-5.34890823e-3");
    VALUES_EQUAL_OK(sb.value(&ok), -5.34890823e-3);
}

void WidgetsTest::testChangingValueKeyPress() {

    {
        NumberSpinBox sb(nullptr);
        bool ok;
        sb.lineEdit()->setText("3");
        sb.lineEdit()->setCursorPosition(1);
        QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 2);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 1);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 0);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), -1);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), -2);

        event = QKeyEvent(QKeyEvent::Type::Enter, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), -1);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 0);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 1);
    }

    {
        NumberSpinBox sb(nullptr);
        bool ok;
        sb.lineEdit()->setText("3000");
        sb.lineEdit()->setCursorPosition(2);
        QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 2900);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 2800);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 2700);

        event = QKeyEvent(QKeyEvent::Type::Enter, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 2800);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 2900);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 3000);
        sb.stepBy(71);
        VALUES_EQUAL_OK(sb.value(&ok), 10100);
    }

    {
        NumberSpinBox sb(nullptr);
        bool ok;
        sb.lineEdit()->setText("3.133");
        sb.lineEdit()->setCursorPosition(4);
        QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 3.123);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 3.113);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 3.103);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 3.093);

        event = QKeyEvent(QKeyEvent::Type::Enter, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 3.103);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 3.113);

        sb.lineEdit()->setCursorPosition(1);
        event = QKeyEvent(QKeyEvent::Type::Enter, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 4.113);

        event = QKeyEvent(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 3.113);
    }

    // Even if it crosses an integer it should still lower with the position set
    {
        NumberSpinBox sb(nullptr);
        bool ok;
        sb.lineEdit()->setText("4.1");
        sb.lineEdit()->setCursorPosition(4);
        QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 4.0);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 3.9);
    }

    {
        NumberSpinBox sb(nullptr);
        bool ok;
        sb.lineEdit()->setText("3.34890823e-3");
        sb.lineEdit()->setCursorPosition(1);
		QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 2.34890823e-3);
		sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 1.34890823e-3);
		sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 0.34890823e-3);
		sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 0.34890823e-3);

        // Minus selected
        sb.lineEdit()->setCursorPosition(12);
        event =  QKeyEvent(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 0.34890823e-3);

        sb.lineEdit()->setCursorPosition(13);
        event =  QKeyEvent(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
        VALUES_EQUAL_OK(sb.value(&ok), 0.34890823e-4);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 0.34890823e-5);

        sb.lineEdit()->setCursorPosition(5);
        sb.keyPressEvent(&event);
        VALUES_EQUAL_OK(sb.value(&ok), 0.34790823e-5);
	}
}

QTEST_MAIN(WidgetsTest)
