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

void WidgetsTest::numberSpinBoxProperties() {
	{
		NumberSpinBox sb;
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("133", p), true);
		QCOMPARE(p.integerSign, QChar::Null);
		QCOMPARE(p.integer, 133);
		QCOMPARE(p.intergerDigits, 3);
		QCOMPARE(p.fraction, false);
		// QCOMPARE(p.fractionDigits, 0);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	{
		NumberSpinBox sb;
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("+133", p), true);
		QCOMPARE(p.integerSign, '+');
		QCOMPARE(p.integer, 133);
		QCOMPARE(p.intergerDigits, 3);
		QCOMPARE(p.fraction, false);
		// QCOMPARE(p.fractionDigits, 0);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	{
		NumberSpinBox sb;
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-122129", p), true);
		QCOMPARE(p.integerSign, '-');
		QCOMPARE(p.integer, 122129);
		QCOMPARE(p.intergerDigits, 6);
		QCOMPARE(p.fraction, false);
		// QCOMPARE(p.fractionDigits, 0);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	{
		NumberSpinBox sb;
		NumberSpinBox::NumberProperties p;
		sb.setLocale(QLocale(QLocale::Language::English));
		QCOMPARE(sb.properties("123.348", p), true);
		QCOMPARE(p.integerSign, QChar::Null);
		QCOMPARE(p.integer, 123);
		QCOMPARE(p.intergerDigits, 3);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 3);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("+13832.283", p), true);
		QCOMPARE(p.integerSign, '+');
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-13832.283", p), true);
		QCOMPARE(p.integerSign, '-');
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	// without any number after comma
	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("14323.", p), true);
		QCOMPARE(p.integerSign, QChar::Null);
		QCOMPARE(p.integer, 14323);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 5);
		QCOMPARE(p.fractionDigits, 0);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("+1334832.", p), true);
		QCOMPARE(p.integerSign, '+');
		QCOMPARE(p.integer, 1334832);
		QCOMPARE(p.intergerDigits, 7);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 8);
		QCOMPARE(p.fractionDigits, 0);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-13823432.", p), true);
		QCOMPARE(p.integerSign, '-');
		QCOMPARE(p.integer, 13823432);
		QCOMPARE(p.intergerDigits, 8);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 9);
		QCOMPARE(p.fractionDigits, 0);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	// exponent
	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("123.348E001", p), true);
		QCOMPARE(p.integerSign, QChar::Null);
		QCOMPARE(p.integer, 123);
		QCOMPARE(p.intergerDigits, 3);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 3);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, 'E');
		QCOMPARE(p.exponentPos, 7);
		QCOMPARE(p.exponentSign, QChar::Null);
		QCOMPARE(p.exponent, 1);
		QCOMPARE(p.exponentDigits, 3);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("+13832.283E012", p), true);
		QCOMPARE(p.integerSign, '+');
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, 'E');
		QCOMPARE(p.exponentPos, 10);
		QCOMPARE(p.exponentSign, QChar::Null);
		QCOMPARE(p.exponent, 12);
		QCOMPARE(p.exponentDigits, 3);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-13832.283E99", p), true);
		QCOMPARE(p.integerSign, '-');
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, 'E');
		QCOMPARE(p.exponentPos, 10);
		QCOMPARE(p.exponentSign, QChar::Null);
		QCOMPARE(p.exponent, 99);
		QCOMPARE(p.exponentDigits, 2);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("123.348e-001", p), true);
		QCOMPARE(p.integerSign, QChar::Null);
		QCOMPARE(p.integer, 123);
		QCOMPARE(p.intergerDigits, 3);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 3);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, 'e');
		QCOMPARE(p.exponentPos, 7);
		QCOMPARE(p.exponentSign, '-');
		QCOMPARE(p.exponent, 1);
		QCOMPARE(p.exponentDigits, 3);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("+13832.283e-012", p), true);
		QCOMPARE(p.integerSign, '+');
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, 'e');
		QCOMPARE(p.exponentPos, 10);
		QCOMPARE(p.exponentSign, '-');
		QCOMPARE(p.exponent, 12);
		QCOMPARE(p.exponentDigits, 3);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-13832.283e-123", p), true);
		QCOMPARE(p.integerSign, '-');
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, 'e');
		QCOMPARE(p.exponentPos, 10);
		QCOMPARE(p.exponentSign, '-');
		QCOMPARE(p.exponent, 123);
		QCOMPARE(p.exponentDigits, 3);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-13832.283e0", p), true);
		QCOMPARE(p.integerSign, '-');
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, 'e');
		QCOMPARE(p.exponentPos, 10);
		QCOMPARE(p.exponentSign, QChar::Null);
		QCOMPARE(p.exponent, 0);
		QCOMPARE(p.exponentDigits, 1);
	}
}

void WidgetsTest::numberSpinBoxCreateStringNumber() {
	{
		NumberSpinBox sb;
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("133", p), true);
		QCOMPARE(sb.createStringNumber(143, 0, p), "143");
	}

	{
		NumberSpinBox sb;
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("+133", p), true);
		QCOMPARE(sb.createStringNumber(143, 0, p), "+143");
	}

	{
		NumberSpinBox sb;
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-122129", p), true);
		QCOMPARE(sb.createStringNumber(-122129, 0, p), "-122129");
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("123.348", p), true);
		QCOMPARE(sb.createStringNumber(23.348345, 0, p), "23.348");
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("+13832.283", p), true);
		QCOMPARE(sb.createStringNumber(+13732.28, 0, p), "+13,732.280");
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-13832.283", p), true);
		QCOMPARE(sb.createStringNumber(-13812.28, 0, p), "-13,812.280");
	}

	// without any number after comma
	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("14323.", p), true);
		QCOMPARE(sb.createStringNumber(345., 0, p), "345.");
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("+1334832.", p), true);
		QCOMPARE(sb.createStringNumber(+345., 0, p), "+345.");
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-13823432.", p), true);
		QCOMPARE(sb.createStringNumber(-45., 0, p), "-45.");
	}

	// exponent
	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("999.348E001", p), true);
		QCOMPARE(sb.createStringNumber(1000.348, 12, p), "1,000.348E012");
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("+13832.283E012", p), true);
		QCOMPARE(sb.createStringNumber(+123.283, 2, p), "+123.283E002");
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-13832.283E99", p), true);
		QCOMPARE(sb.createStringNumber(-13832.283, 99, p), "-13,832.283E99");
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("123.348e-001", p), true);
		QCOMPARE(sb.createStringNumber(123.348, -12, p), "123.348e-012");
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("+13832.283e-012", p), true);
		QCOMPARE(sb.createStringNumber(+13832.283, -12, p), "+13,832.283e-012");
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-13832.283e-123", p), true);
		QCOMPARE(sb.createStringNumber(-123.283, -3, p), "-123.283e-003");
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties("-13832.283e00", p), true);
		QCOMPARE(sb.createStringNumber(-123.283, 0, p), "-123.283e00");
	}
}

void WidgetsTest::numberSpinBoxChangingValueKeyPress() {
	{
		NumberSpinBox sb;
		sb.lineEdit()->setText("3");
		sb.lineEdit()->setCursorPosition(1);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 1);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 0);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -2);

		event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 0);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 1);
	}

	{
		NumberSpinBox sb;
		sb.lineEdit()->setText("3000");
		sb.lineEdit()->setCursorPosition(2);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2900);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2800);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2700);

		event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2800);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2900);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 3000);
		sb.stepBy(71);
		VALUES_EQUAL(sb.value(), 10100);
	}

	{
		NumberSpinBox sb;
		sb.lineEdit()->setText("5e1");
		sb.lineEdit()->setCursorPosition(3);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5e0);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5e-1);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5e-2);

		event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5e-1);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5e0);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText("5.5e+300");
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5.5e+307);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText("1.6e+300");
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 1.6e+308);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText("5.5e-300");
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5.5e-307);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText("1.6e-300");
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 1.6e-308);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText("3.133");
		sb.lineEdit()->setCursorPosition(4);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 3.123);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 3.113);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 3.103);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 3.093);

		event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 3.103);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 3.113);

		sb.lineEdit()->setCursorPosition(1);
		event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 4.113);

		event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 3.113);
	}

	// Even if it crosses an integer it should still lower with the position set
	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText("4.1");
		sb.lineEdit()->setCursorPosition(4);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 4.0);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 3.9);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText("3.34890823e-03");
		sb.lineEdit()->setCursorPosition(1);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2.34890823e-3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 1.34890823e-3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 0.34890823e-3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1.34890823e-3);

		// e selected do nothing!
		sb.lineEdit()->setCursorPosition(12);
		event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1.34890823e-3);

		// Minus selected do nothing!
		sb.lineEdit()->setCursorPosition(13);
		event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1.34890823e-3);

		sb.lineEdit()->setCursorPosition(14);
		event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1.34890823e-13);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1.34890823e-23);
	}

	// Jump to next valid position with the cursor
	// 13 -> 3 -> 2 -> ... (cursor will be automatically set behind the 3)
	{
		NumberSpinBox sb;
		sb.lineEdit()->setText("13");
		sb.lineEdit()->setCursorPosition(1);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2);
	}

	// Jump to next valid position with the cursor
	// -13 -> -3 -> -2 -> ... (cursor will be automatically set behind the 3)
	{
		NumberSpinBox sb;
		sb.lineEdit()->setText("-13");
		sb.lineEdit()->setCursorPosition(2);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -2);
	}
}

void WidgetsTest::numberSpinBoxLimit() {
	NumberSpinBox sb(5);
	sb.setMaximum(7);
	sb.setMinimum(3);

	int valueChangedCounter = 0;
	double lastValue = NAN;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&valueChangedCounter, &lastValue](double value) {
		lastValue = value;
		valueChangedCounter++;
	});

	sb.lineEdit()->setCursorPosition(1);
	sb.increaseValue();
	VALUES_EQUAL(sb.value(), 6);
	QCOMPARE(valueChangedCounter, 1);
	QCOMPARE(lastValue, 6);
	sb.increaseValue();
	VALUES_EQUAL(sb.value(), 7);
	QCOMPARE(valueChangedCounter, 2);
	QCOMPARE(lastValue, 7);
	sb.increaseValue();
	VALUES_EQUAL(sb.value(), 7); // unable to go beyond
	QCOMPARE(valueChangedCounter, 2);

	sb.setValue(4);
	sb.lineEdit()->setCursorPosition(1);
	VALUES_EQUAL(sb.value(), 4);
	QCOMPARE(valueChangedCounter, 2);
	sb.decreaseValue();
	VALUES_EQUAL(sb.value(), 3);
	QCOMPARE(valueChangedCounter, 3);
	QCOMPARE(lastValue, 3);
	sb.decreaseValue();
	VALUES_EQUAL(sb.value(), 3);
	QCOMPARE(valueChangedCounter, 3);

	// Try to insert a number
	sb.lineEdit()->setCursorPosition(1);
	// Important: the event text is used by the lineedit. So it has to be filled!
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_1, Qt::KeyboardModifier::NoModifier, "1");
	sb.keyPressEvent(&event);
	QCOMPARE(valueChangedCounter, 3);
	VALUES_EQUAL(sb.value(), 3);
}

void WidgetsTest::numberSpinBoxPrefixSuffix() {
	NumberSpinBox sb(5);
	sb.setMaximum(7);
	sb.setMinimum(3);

	int valueChangedCounter = 0;
	double lastValue = NAN;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&valueChangedCounter, &lastValue](double value) {
		lastValue = value;
		valueChangedCounter++;
	});

	sb.setPrefix("Prefix ");
	sb.setSuffix(" Suffix");

	QCOMPARE(sb.lineEdit()->text(), "Prefix 5 Suffix");

	sb.lineEdit()->setCursorPosition(10); // In suffix text
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&event);

	QCOMPARE(valueChangedCounter, 1);
	QCOMPARE(sb.value(), 6);
	QCOMPARE(sb.lineEdit()->text(), "Prefix 6 Suffix");
}

void WidgetsTest::numberSpinBoxEnterNumber() {
	NumberSpinBox sb(5);
	sb.setMaximum(100);
	sb.setMinimum(0);

	int valueChangedCounter = 0;
	double lastValue = NAN;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&valueChangedCounter, &lastValue](double value) {
		lastValue = value;
		valueChangedCounter++;
	});

	QCOMPARE(sb.toolTip(), "");
	sb.lineEdit()->setCursorPosition(1);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_1, Qt::KeyboardModifier::NoModifier, "1");
	sb.keyPressEvent(&event);
	QCOMPARE(sb.toolTip(), "");
	QCOMPARE(sb.value(), 51);
	QCOMPARE(valueChangedCounter, 1);

	// 511
	sb.keyPressEvent(&event);
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::Max));
	QCOMPARE(sb.value(), 51);
	QCOMPARE(sb.lineEdit()->text(), "511");
	QCOMPARE(valueChangedCounter, 1);

	// 51
	event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Backspace, Qt::KeyboardModifier::NoModifier, "");
	sb.keyPressEvent(&event);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 2);
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.value(), 51);
	QCOMPARE(valueChangedCounter, 2);

	// 5
	event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Backspace, Qt::KeyboardModifier::NoModifier, "");
	sb.keyPressEvent(&event);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 1);
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.value(), 5);
	QCOMPARE(valueChangedCounter, 3);

	// nothing
	event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Backspace, Qt::KeyboardModifier::NoModifier, "");
	sb.keyPressEvent(&event);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 0);
	QCOMPARE(sb.lineEdit()->text(), "");
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoNumber));
	QCOMPARE(sb.value(), 5);
	QCOMPARE(valueChangedCounter, 3);

	// -
	event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Minus, Qt::KeyboardModifier::NoModifier, "-");
	sb.keyPressEvent(&event);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 1);
	QCOMPARE(sb.lineEdit()->text(), "-");
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::Invalid));
	QCOMPARE(sb.value(), 5);
	QCOMPARE(valueChangedCounter, 3);

	// -5
	event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_5, Qt::KeyboardModifier::NoModifier, "5");
	sb.setMinimum(-100);
	sb.keyPressEvent(&event);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 2);
	QCOMPARE(sb.lineEdit()->text(), "-5");
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.value(), -5);
	QCOMPARE(valueChangedCounter, 4);

	// -5e
	event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_E, Qt::KeyboardModifier::NoModifier, "e");
	sb.keyPressEvent(&event);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 3);
	QCOMPARE(sb.lineEdit()->text(), "-5e");
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::Invalid));
	QCOMPARE(sb.value(), -5);
	QCOMPARE(valueChangedCounter, 4);

	// -5e-
	event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Minus, Qt::KeyboardModifier::NoModifier, "-");
	sb.keyPressEvent(&event);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 4);
	QCOMPARE(sb.lineEdit()->text(), "-5e-");
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::Invalid));
	QCOMPARE(sb.value(), -5);
	QCOMPARE(valueChangedCounter, 4);

	// -5e-3
	event = QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_3, Qt::KeyboardModifier::NoModifier, "3");
	sb.keyPressEvent(&event);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 5);
	QCOMPARE(sb.lineEdit()->text(), "-5e-3");
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.value(), -5e-3);
	QCOMPARE(valueChangedCounter, 5);
}

// Testing feedback feature
void WidgetsTest::numberSpinBoxFeedback() {
	NumberSpinBox sb(5);
	sb.setFeedback(true);

	int valueChangedCounter = 0;
	double lastValue = NAN;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&valueChangedCounter, &lastValue](double value) {
		lastValue = value;
		valueChangedCounter++;
	});

	sb.lineEdit()->setCursorPosition(1);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&event);

	QCOMPARE(valueChangedCounter, 1);
	QCOMPARE(lastValue, 6);
	QCOMPARE(sb.mWaitFeedback, true);
	QCOMPARE(sb.setValue(6), true);
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.mWaitFeedback, false);

	sb.keyPressEvent(&event);
	QCOMPARE(valueChangedCounter, 2);
	QCOMPARE(lastValue, 7);
	QCOMPARE(sb.mWaitFeedback, true);
	QCOMPARE(sb.setValue(0), false);
	QCOMPARE(sb.toolTip(), tr("Invalid value entered. Valid value: %1").arg(0));
	QCOMPARE(sb.mWaitFeedback, false);

	sb.keyPressEvent(&event);
	QCOMPARE(valueChangedCounter, 3);
	QCOMPARE(lastValue, 8);
	QCOMPARE(sb.mWaitFeedback, true);
	QCOMPARE(sb.setValue(8), true);
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.mWaitFeedback, false);
}

QTEST_MAIN(WidgetsTest)
