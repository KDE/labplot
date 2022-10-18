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

#define VALUES_EQUAL_OK(v, ref)                                                                                                                                \
	do {                                                                                                                                                       \
		const auto value = v;                                                                                                                                  \
		QVERIFY(ok);                                                                                                                                           \
		VALUES_EQUAL(value, ref);                                                                                                                              \
	} while (false)

void WidgetsTest::numberSpinBoxTest1() {
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

void WidgetsTest::numberSpinBoxProperties() {
	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("133", p), true);
		VALUES_EQUAL(p.value, 133);
		QCOMPARE(p.integerSign, QChar::Null);
		QCOMPARE(p.integer, 133);
		QCOMPARE(p.intergerDigits, 3);
		QCOMPARE(p.fraction, false);
		// QCOMPARE(p.fractionDigits, 0);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("+133", p), true);
		VALUES_EQUAL(p.value, 133);
		QCOMPARE(p.integerSign, '+');
		QCOMPARE(p.integer, 133);
		QCOMPARE(p.intergerDigits, 3);
		QCOMPARE(p.fraction, false);
		// QCOMPARE(p.fractionDigits, 0);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-122129", p), true);
		VALUES_EQUAL(p.value, -122129);
		QCOMPARE(p.integerSign, '-');
		QCOMPARE(p.integer, 122129);
		QCOMPARE(p.intergerDigits, 6);
		QCOMPARE(p.fraction, false);
		// QCOMPARE(p.fractionDigits, 0);
		QCOMPARE(p.exponentLetter, QChar::Null);
		// exponents are invalid
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("123.348", p), true);
		VALUES_EQUAL(p.value, 123.348);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("+13832.283", p), true);
		VALUES_EQUAL(p.value, 13832.283);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-13832.283", p), true);
		VALUES_EQUAL(p.value, -13832.283);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("14323.", p), true);
		VALUES_EQUAL(p.value, 14323);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("+1334832.", p), true);
		VALUES_EQUAL(p.value, 1334832);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-13823432.", p), true);
		VALUES_EQUAL(p.value, -13823432);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("123.348E001", p), true);
		VALUES_EQUAL(p.value, 123.348e1);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("+13832.283E012", p), true);
		VALUES_EQUAL(p.value, 13832.283e12);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-13832.283E99", p), true);
		VALUES_EQUAL(p.value, -13832.283E99);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("123.348e-001", p), true);
		VALUES_EQUAL(p.value, 123.348e-1);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("+13832.283e-012", p), true);
		VALUES_EQUAL(p.value, 13832.283e-12);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-13832.283e-123", p), true);
		VALUES_EQUAL(p.value, -13832.283e-123);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-13832.283e0", p), true);
		VALUES_EQUAL(p.value, -13832.283);
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
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("133", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(143, 0, p), "143");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("+133", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(143, 0, p), "+143");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-122129", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(-122129, 0, p), "-122129");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("123.348", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(23.348345, 0, p), "23.348");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("+13832.283", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(+13732.28, 0, p), "+13732.280");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-13832.283", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(-13812.28, 0, p), "-13812.280");
	}

	// without any number after comma
	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("14323.", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(345., 0, p), "345.");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("+1334832.", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(+345., 0, p), "+345.");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-13823432.", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(-45., 0, p), "-45.");
	}

	// exponent
	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("999.348E001", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(1000.348, 12, p), "1000.348E012");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("+13832.283E012", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(+123.283, 2, p), "+123.283E002");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-13832.283E99", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(-13832.283, 99, p), "-13832.283E99");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("123.348e-001", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(123.348, -12, p), "123.348e-012");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("+13832.283e-012", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(+13832.283, -12, p), "+13832.283e-012");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-13832.283e-123", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(-123.283, -3, p), "-123.283e-003");
	}

	{
		NumberSpinBox::NumberProperties p;
		QCOMPARE(NumberSpinBox::properties("-13832.283e00", p), true);
		QCOMPARE(NumberSpinBox::createStringNumber(-123.283, 0, p), "-123.283e00");
	}
}

void WidgetsTest::numberSpinBoxChangingValueKeyPress() {
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
		sb.lineEdit()->setText("5e1");
		sb.lineEdit()->setCursorPosition(3);
		QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 5e0);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 5e-1);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 5e-2);

		event = QKeyEvent(QKeyEvent::Type::Enter, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 5e-1);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 5e0);
	}

	{
		NumberSpinBox sb(nullptr);
		bool ok;
		sb.lineEdit()->setText("5.5e+300");
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 5.5e+307);
	}

	{
		NumberSpinBox sb(nullptr);
		bool ok;
		sb.lineEdit()->setText("1.6e+300");
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 1.6e+308);
	}

	{
		NumberSpinBox sb(nullptr);
		bool ok;
		sb.lineEdit()->setText("5.5e-300");
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 5.5e-307);
	}

	{
		NumberSpinBox sb(nullptr);
		bool ok;
		sb.lineEdit()->setText("1.6e-300");
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 1.6e-308);
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
		sb.lineEdit()->setText("3.34890823e-03");
		sb.lineEdit()->setCursorPosition(1);
		QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 2.34890823e-3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 1.34890823e-3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 0.34890823e-3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), -1.34890823e-3);

		// e selected do nothing!
		sb.lineEdit()->setCursorPosition(12);
		event = QKeyEvent(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), -1.34890823e-3);

		// Minus selected do nothing!
		sb.lineEdit()->setCursorPosition(13);
		event = QKeyEvent(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), -1.34890823e-3);

		sb.lineEdit()->setCursorPosition(14);
		event = QKeyEvent(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), -1.34890823e-13);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), -1.34890823e-23);
	}

	// Jump to next valid position with the cursor
	// 13 -> 3 -> 2 -> ... (cursor will be automatically set behind the 3)
	{
		NumberSpinBox sb(nullptr);
		bool ok;
		sb.lineEdit()->setText("13");
		sb.lineEdit()->setCursorPosition(1);
		QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), 2);
	}

	// Jump to next valid position with the cursor
	// -13 -> -3 -> -2 -> ... (cursor will be automatically set behind the 3)
	{
		NumberSpinBox sb(nullptr);
		bool ok;
		sb.lineEdit()->setText("-13");
		sb.lineEdit()->setCursorPosition(2);
		QKeyEvent event(QKeyEvent::Type::Enter, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), -3);
		sb.keyPressEvent(&event);
		VALUES_EQUAL_OK(sb.value(&ok), -2);
	}
}

QTEST_MAIN(WidgetsTest)
