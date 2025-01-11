/*
	File                 : SpinBoxTest.cpp
	Project              : LabPlot
	Description          : Tests for spinbox widgets
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpinBoxTest.h"
#include "backend/core/Project.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "frontend/dockwidgets/CartesianPlotDock.h"
#include "frontend/widgets/NumberSpinBox.h"

#include <QLineEdit>
#include <QLocale>

void SpinBoxTest::numberSpinBoxProperties() {
	{
		NumberSpinBox sb;
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("133"), p), true);
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
		QCOMPARE(sb.properties(QStringLiteral("+133"), p), true);
		QCOMPARE(p.integerSign, QLatin1Char('+'));
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
		QCOMPARE(sb.properties(QStringLiteral("-122129"), p), true);
		QCOMPARE(p.integerSign, QLatin1Char('-'));
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
		QCOMPARE(sb.properties(QStringLiteral("123.348"), p), true);
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
		QCOMPARE(sb.properties(QStringLiteral("+13832.283"), p), true);
		QCOMPARE(p.integerSign, QLatin1Char('+'));
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
		QCOMPARE(sb.properties(QStringLiteral("-13832.283"), p), true);
		QCOMPARE(p.integerSign, QLatin1Char('-'));
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
		QCOMPARE(sb.properties(QStringLiteral("14323."), p), true);
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
		QCOMPARE(sb.properties(QStringLiteral("+1334832."), p), true);
		QCOMPARE(p.integerSign, QLatin1Char('+'));
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
		QCOMPARE(sb.properties(QStringLiteral("-13823432."), p), true);
		QCOMPARE(p.integerSign, QLatin1Char('-'));
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
		QCOMPARE(sb.properties(QStringLiteral("123.348E001"), p), true);
		QCOMPARE(p.integerSign, QChar::Null);
		QCOMPARE(p.integer, 123);
		QCOMPARE(p.intergerDigits, 3);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 3);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, QLatin1Char('E'));
		QCOMPARE(p.exponentPos, 7);
		QCOMPARE(p.exponentSign, QChar::Null);
		QCOMPARE(p.exponent, 1);
		QCOMPARE(p.exponentDigits, 3);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("+13832.283E012"), p), true);
		QCOMPARE(p.integerSign, QLatin1Char('+'));
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, QLatin1Char('E'));
		QCOMPARE(p.exponentPos, 10);
		QCOMPARE(p.exponentSign, QChar::Null);
		QCOMPARE(p.exponent, 12);
		QCOMPARE(p.exponentDigits, 3);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("-13832.283E99"), p), true);
		QCOMPARE(p.integerSign, QLatin1Char('-'));
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, QLatin1Char('E'));
		QCOMPARE(p.exponentPos, 10);
		QCOMPARE(p.exponentSign, QChar::Null);
		QCOMPARE(p.exponent, 99);
		QCOMPARE(p.exponentDigits, 2);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("123.348e-001"), p), true);
		QCOMPARE(p.integerSign, QChar::Null);
		QCOMPARE(p.integer, 123);
		QCOMPARE(p.intergerDigits, 3);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 3);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, QLatin1Char('e'));
		QCOMPARE(p.exponentPos, 7);
		QCOMPARE(p.exponentSign, QLatin1Char('-'));
		QCOMPARE(p.exponent, 1);
		QCOMPARE(p.exponentDigits, 3);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("+13832.283e-012"), p), true);
		QCOMPARE(p.integerSign, QLatin1Char('+'));
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, QLatin1Char('e'));
		QCOMPARE(p.exponentPos, 10);
		QCOMPARE(p.exponentSign, QLatin1Char('-'));
		QCOMPARE(p.exponent, 12);
		QCOMPARE(p.exponentDigits, 3);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("-13832.283e-123"), p), true);
		QCOMPARE(p.integerSign, QLatin1Char('-'));
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, QLatin1Char('e'));
		QCOMPARE(p.exponentPos, 10);
		QCOMPARE(p.exponentSign, QLatin1Char('-'));
		QCOMPARE(p.exponent, 123);
		QCOMPARE(p.exponentDigits, 3);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("-13832.283e0"), p), true);
		QCOMPARE(p.integerSign, QLatin1Char('-'));
		QCOMPARE(p.integer, 13832);
		QCOMPARE(p.intergerDigits, 5);
		QCOMPARE(p.fraction, true);
		QCOMPARE(p.fractionPos, 6);
		QCOMPARE(p.fractionDigits, 3);
		QCOMPARE(p.exponentLetter, QLatin1Char('e'));
		QCOMPARE(p.exponentPos, 10);
		QCOMPARE(p.exponentSign, QChar::Null);
		QCOMPARE(p.exponent, 0);
		QCOMPARE(p.exponentDigits, 1);
	}
}

void SpinBoxTest::numberSpinBoxCreateStringNumber() {
	{
		NumberSpinBox sb;
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("133"), p), true);
		QCOMPARE(sb.createStringNumber(143, 0, p), QStringLiteral("143"));
	}

	{
		NumberSpinBox sb;
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("+133"), p), true);
		QCOMPARE(sb.createStringNumber(143, 0, p), QStringLiteral("+143"));
	}

	{
		NumberSpinBox sb;
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("-122129"), p), true);
		QCOMPARE(sb.createStringNumber(-122129, 0, p), QStringLiteral("-122129"));
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("123.348"), p), true);
		QCOMPARE(sb.createStringNumber(23.348345, 0, p), QStringLiteral("23.348"));
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("+13832.283"), p), true);
		QCOMPARE(sb.createStringNumber(+13732.28, 0, p), QStringLiteral("+13,732.280"));
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("-13832.283"), p), true);
		QCOMPARE(sb.createStringNumber(-13812.28, 0, p), QStringLiteral("-13,812.280"));
	}

	// without any number after comma
	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("14323."), p), true);
		QCOMPARE(sb.createStringNumber(345., 0, p), QStringLiteral("345."));
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("+1334832."), p), true);
		QCOMPARE(sb.createStringNumber(+345., 0, p), QStringLiteral("+345."));
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("-13823432."), p), true);
		QCOMPARE(sb.createStringNumber(-45., 0, p), QStringLiteral("-45."));
	}

	// exponent
	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("999.348E001"), p), true);
		QCOMPARE(sb.createStringNumber(1000.348, 12, p), QStringLiteral("1,000.348E012"));
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("+13832.283E012"), p), true);
		QCOMPARE(sb.createStringNumber(+123.283, 2, p), QStringLiteral("+123.283E002"));
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("-13832.283E99"), p), true);
		QCOMPARE(sb.createStringNumber(-13832.283, 99, p), QStringLiteral("-13,832.283E99"));
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("123.348e-001"), p), true);
		QCOMPARE(sb.createStringNumber(123.348, -12, p), QStringLiteral("123.348e-012"));
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("+13832.283e-012"), p), true);
		QCOMPARE(sb.createStringNumber(+13832.283, -12, p), QStringLiteral("+13,832.283e-012"));
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("-13832.283e-123"), p), true);
		QCOMPARE(sb.createStringNumber(-123.283, -3, p), QStringLiteral("-123.283e-003"));
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		NumberSpinBox::NumberProperties p;
		QCOMPARE(sb.properties(QStringLiteral("-13832.283e00"), p), true);
		QCOMPARE(sb.createStringNumber(-123.283, 0, p), QStringLiteral("-123.283e00"));
	}
}

void SpinBoxTest::numberSpinBoxChangingValueKeyPress() {
	{
		NumberSpinBox sb;
		sb.lineEdit()->setText(QStringLiteral("3"));
		sb.lineEdit()->setCursorPosition(1);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2.);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 1.);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 0.);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1.);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -2.);

		QKeyEvent upEvent(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&upEvent);
		VALUES_EQUAL(sb.value(), -1.);
		sb.keyPressEvent(&upEvent);
		VALUES_EQUAL(sb.value(), 0.);
		sb.keyPressEvent(&upEvent);
		VALUES_EQUAL(sb.value(), 1.);
	}

	{
		NumberSpinBox sb;
		sb.lineEdit()->setText(QStringLiteral("3000"));
		sb.lineEdit()->setCursorPosition(2);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2900.);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2800.);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2700.);

		QKeyEvent upEvent(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&upEvent);
		VALUES_EQUAL(sb.value(), 2800.);
		sb.keyPressEvent(&upEvent);
		VALUES_EQUAL(sb.value(), 2900.);
		sb.keyPressEvent(&upEvent);
		VALUES_EQUAL(sb.value(), 3000.);
		sb.stepBy(71);
		VALUES_EQUAL(sb.value(), 10100.);
	}

	{
		NumberSpinBox sb;
		sb.lineEdit()->setText(QStringLiteral("5e1"));
		sb.lineEdit()->setCursorPosition(3);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5.e0);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5.e-1);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5.e-2);

		QKeyEvent upEvent(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&upEvent);
		VALUES_EQUAL(sb.value(), 5.e-1);
		sb.keyPressEvent(&upEvent);
		VALUES_EQUAL(sb.value(), 5.e0);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText(QStringLiteral("5.5e+300"));
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5.5e+307);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText(QStringLiteral("1.6e+300"));
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 1.6e+308);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText(QStringLiteral("5.5e-300"));
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 5.5e-307);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText(QStringLiteral("1.6e-300"));
		sb.lineEdit()->setCursorPosition(7);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 1.6e-308);
	}

	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText(QStringLiteral("3.133"));
		sb.lineEdit()->setCursorPosition(4);
		QKeyEvent downEvent(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&downEvent);
		VALUES_EQUAL(sb.value(), 3.123);
		sb.keyPressEvent(&downEvent);
		VALUES_EQUAL(sb.value(), 3.113);
		sb.keyPressEvent(&downEvent);
		VALUES_EQUAL(sb.value(), 3.103);
		sb.keyPressEvent(&downEvent);
		VALUES_EQUAL(sb.value(), 3.093);

		QKeyEvent upEvent(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&upEvent);
		VALUES_EQUAL(sb.value(), 3.103);
		sb.keyPressEvent(&upEvent);
		VALUES_EQUAL(sb.value(), 3.113);

		sb.lineEdit()->setCursorPosition(1);
		sb.keyPressEvent(&upEvent);
		VALUES_EQUAL(sb.value(), 4.113);

		sb.keyPressEvent(&downEvent);
		VALUES_EQUAL(sb.value(), 3.113);
	}

	// Even if it crosses an integer it should still lower with the position set
	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::Language::English));
		sb.lineEdit()->setText(QStringLiteral("4.1"));
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
		sb.lineEdit()->setText(QStringLiteral("3.34890823e-03"));
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
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1.34890823e-3);

		// Minus selected do nothing!
		sb.lineEdit()->setCursorPosition(13);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1.34890823e-3);

		sb.lineEdit()->setCursorPosition(14);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1.34890823e-13);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -1.34890823e-23);
	}

	// Jump to next valid position with the cursor
	// 13 -> 3 -> 2 -> ... (cursor will be automatically set behind the 3)
	{
		NumberSpinBox sb;
		sb.lineEdit()->setText(QStringLiteral("13"));
		sb.lineEdit()->setCursorPosition(1);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 3.);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), 2.);
	}

	// Jump to next valid position with the cursor
	// -13 -> -3 -> -2 -> ... (cursor will be automatically set behind the 3)
	{
		NumberSpinBox sb;
		sb.lineEdit()->setText(QStringLiteral("-13"));
		sb.lineEdit()->setCursorPosition(2);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -3.);
		sb.keyPressEvent(&event);
		VALUES_EQUAL(sb.value(), -2.);
	}
}

void SpinBoxTest::numberSpinBoxLimit() {
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
	VALUES_EQUAL(sb.value(), 6.);
	QCOMPARE(valueChangedCounter, 1);
	QCOMPARE(lastValue, 6);
	sb.increaseValue();
	VALUES_EQUAL(sb.value(), 7.);
	QCOMPARE(valueChangedCounter, 2);
	QCOMPARE(lastValue, 7);
	sb.increaseValue();
	VALUES_EQUAL(sb.value(), 7.); // unable to go beyond
	QCOMPARE(valueChangedCounter, 2);

	sb.setValue(4);
	QCOMPARE(valueChangedCounter, 3);
	sb.lineEdit()->setCursorPosition(1);
	VALUES_EQUAL(sb.value(), 4.);
	QCOMPARE(valueChangedCounter, 3);
	sb.decreaseValue();
	VALUES_EQUAL(sb.value(), 3.);
	QCOMPARE(valueChangedCounter, 4);
	QCOMPARE(lastValue, 3);
	sb.decreaseValue();
	VALUES_EQUAL(sb.value(), 3.);
	QCOMPARE(valueChangedCounter, 4);

	// Try to insert a number
	sb.lineEdit()->setCursorPosition(1);
	// Important: the event text is used by the lineedit. So it has to be filled!
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_1, Qt::KeyboardModifier::NoModifier, QStringLiteral("1"));
	sb.keyPressEvent(&event);
	QCOMPARE(valueChangedCounter, 4);
	VALUES_EQUAL(sb.value(), 3.);
}

void SpinBoxTest::numberSpinBoxPrefixSuffix() {
	NumberSpinBox sb(5.01);
	sb.setMaximum(7);
	sb.setMinimum(3);

	int valueChangedCounter = 0;
	double lastValue = NAN;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&valueChangedCounter, &lastValue](double value) {
		lastValue = value;
		valueChangedCounter++;
	});

	sb.setPrefix(QStringLiteral("Prefix "));
	sb.setSuffix(QStringLiteral(" Suffix"));

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("Prefix 5.01 Suffix"));

	sb.lineEdit()->setCursorPosition(14); // In suffix text
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&event);

	QCOMPARE(valueChangedCounter, 1);
	QCOMPARE(sb.value(), 5.02);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("Prefix 5.02 Suffix"));
}

void SpinBoxTest::numberSpinBoxSuffixFrontToBackSelection() {
	// Select from front to back
	NumberSpinBox sb;
	sb.setSuffix(QStringLiteral(" cm")); // whitespace is important!

	sb.setValue(5);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("5 cm"));

	int counter = 0;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&counter](double value) {
		QCOMPARE(value, 51);
		counter++;
	});

	sb.lineEdit()->setSelection(1, 1); // select space
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_1, Qt::KeyboardModifier::NoModifier, QStringLiteral("1"));
	sb.keyPressEvent(&event);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("51 cm"));
	QCOMPARE(counter, 1);
}

void SpinBoxTest::numberSpinBoxSuffixBackToFrontSelection() {
	// Select from back to front
	NumberSpinBox sb;
	sb.setSuffix(QStringLiteral(" cm")); // whitespace is important!

	sb.setValue(55);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("55 cm"));

	int counter = 0;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&counter](double value) {
		QCOMPARE(value, 51);
		counter++;
	});

	sb.lineEdit()->setSelection(3, -2);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_1, Qt::KeyboardModifier::NoModifier, QStringLiteral("1"));
	sb.keyPressEvent(&event);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("51 cm"));
	QCOMPARE(counter, 1);
}

void SpinBoxTest::numberSpinBoxSuffixSetCursor() {
	// Just set cursor into suffix
	NumberSpinBox sb;
	sb.setSuffix(QStringLiteral(" cm")); // whitespace is important!

	sb.setValue(5);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("5 cm"));

	int counter = 0;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&counter](double value) {
		QCOMPARE(value, 51);
		counter++;
	});

	sb.lineEdit()->setCursorPosition(2);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_1, Qt::KeyboardModifier::NoModifier, QStringLiteral("1"));
	sb.keyPressEvent(&event);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("51 cm")); // 1 appended
	QCOMPARE(counter, 1);
}

void SpinBoxTest::numberSpinBoxPrefixFrontToBackSelection() {
	// from front to back
	NumberSpinBox sb;
	sb.setPrefix(QStringLiteral("prefix ")); // whitespace is important!

	sb.setValue(5);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("prefix 5"));

	sb.lineEdit()->setSelection(1, 7); // ref selected
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_1, Qt::KeyboardModifier::NoModifier, QStringLiteral("1"));
	sb.keyPressEvent(&event);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("prefix 1"));
}

void SpinBoxTest::numberSpinBoxPrefixBackToFrontSelection() {
	// from back to front
	NumberSpinBox sb;
	sb.setPrefix(QStringLiteral("prefix ")); // whitespace is important!

	sb.setValue(5);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("prefix 5"));

	sb.lineEdit()->setSelection(8, -5); // "fix 5"
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_1, Qt::KeyboardModifier::NoModifier, QStringLiteral("1"));
	sb.keyPressEvent(&event);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("prefix 1"));
}

void SpinBoxTest::numberSpinBoxPrefixSetCursorPosition() {
	// set cursor
	NumberSpinBox sb;
	sb.setPrefix(QStringLiteral("prefix ")); // whitespace is important!

	sb.setValue(5);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("prefix 5"));

	// For some reason the cursor position is not applied
	sb.lineEdit()->setCursorPosition(2);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_1, Qt::KeyboardModifier::NoModifier, QStringLiteral("1"));
	sb.keyPressEvent(&event);

	// QCOMPARE(sb.lineEdit()->text(), QStringLiteral("prefix 15"));
}

void SpinBoxTest::numberSpinBoxEnterNumber() {
	NumberSpinBox sb;
	sb.setDecimals(0);
	sb.setValue(5);
	sb.setMaximum(100);
	sb.setMinimum(0);

	int valueChangedCounter = 0;
	double lastValue = NAN;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&valueChangedCounter, &lastValue](double value) {
		lastValue = value;
		valueChangedCounter++;
	});

	QCOMPARE(sb.toolTip(), QString());
	sb.lineEdit()->setCursorPosition(1);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_1, Qt::KeyboardModifier::NoModifier, QStringLiteral("1"));
	sb.keyPressEvent(&event);
	QCOMPARE(sb.toolTip(), QString());
	QCOMPARE(sb.value(), 51);
	QCOMPARE(valueChangedCounter, 1);

	// 511
	sb.keyPressEvent(&event);
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::Max));
	QCOMPARE(sb.value(), 51);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("511"));
	QCOMPARE(valueChangedCounter, 1);

	// 51
	QKeyEvent event2(QKeyEvent::Type::KeyPress, Qt::Key_Backspace, Qt::KeyboardModifier::NoModifier, QString());
	sb.keyPressEvent(&event2);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 2);
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.value(), 51);
	QCOMPARE(valueChangedCounter, 1);

	// 5
	QKeyEvent event3(QKeyEvent::Type::KeyPress, Qt::Key_Backspace, Qt::KeyboardModifier::NoModifier, QString());
	sb.keyPressEvent(&event3);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 1);
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.value(), 5);
	QCOMPARE(valueChangedCounter, 2);

	// nothing
	QKeyEvent event4(QKeyEvent::Type::KeyPress, Qt::Key_Backspace, Qt::KeyboardModifier::NoModifier, QString());
	sb.keyPressEvent(&event4);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 0);
	QCOMPARE(sb.lineEdit()->text(), QString());
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoNumber));
	QCOMPARE(sb.value(), 5);
	QCOMPARE(valueChangedCounter, 2);

	// -
	QKeyEvent event5(QKeyEvent::Type::KeyPress, Qt::Key_Minus, Qt::KeyboardModifier::NoModifier, QStringLiteral("-"));
	sb.keyPressEvent(&event5);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 1);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-"));
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::Invalid));
	QCOMPARE(sb.value(), 5);
	QCOMPARE(valueChangedCounter, 2);

	// -5
	QKeyEvent event6(QKeyEvent::Type::KeyPress, Qt::Key_5, Qt::KeyboardModifier::NoModifier, QStringLiteral("5"));
	sb.setMinimum(-100);
	sb.keyPressEvent(&event6);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 2);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-5"));
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.value(), -5);
	QCOMPARE(valueChangedCounter, 3);

	// -5e
	QKeyEvent event7(QKeyEvent::Type::KeyPress, Qt::Key_E, Qt::KeyboardModifier::NoModifier, QStringLiteral("e"));
	sb.keyPressEvent(&event7);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 3);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-5e"));
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::Invalid));
	QCOMPARE(sb.value(), -5);
	QCOMPARE(valueChangedCounter, 3);

	// -5e-
	QKeyEvent event8(QKeyEvent::Type::KeyPress, Qt::Key_Minus, Qt::KeyboardModifier::NoModifier, QStringLiteral("-"));
	sb.keyPressEvent(&event8);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 4);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-5e-"));
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::Invalid));
	QCOMPARE(sb.value(), -5);
	QCOMPARE(valueChangedCounter, 3);

	// -5e-3
	QKeyEvent event9(QKeyEvent::Type::KeyPress, Qt::Key_3, Qt::KeyboardModifier::NoModifier, QStringLiteral("3"));
	sb.keyPressEvent(&event9);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 5);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-5e-3"));
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.value(), -5e-3);
	QCOMPARE(valueChangedCounter, 4);
}

// Testing feedback feature
void SpinBoxTest::numberSpinBoxFeedback() {
	NumberSpinBox sb(5);
	sb.setFeedback(true);

	int valueChangedCounter = 0;
	double lastValue = NAN;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&sb, &valueChangedCounter, &lastValue](double value) {
		lastValue = value;
		valueChangedCounter++;

		QCOMPARE(sb.m_waitFeedback, true);
		switch (valueChangedCounter) {
		case 1:
			QCOMPARE(sb.setValue(6), true);
			break;
		case 2:
			QCOMPARE(sb.setValue(0), false);
			break;
		case 3:
			QCOMPARE(sb.setValue(8), true);
			break;
		}
	});

	sb.lineEdit()->setCursorPosition(1);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);

	sb.keyPressEvent(&event);
	QCOMPARE(valueChangedCounter, 1);
	QCOMPARE(lastValue, 6);
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.m_waitFeedback, false);

	sb.keyPressEvent(&event);
	QCOMPARE(valueChangedCounter, 2);
	QCOMPARE(lastValue, 7);
	QCOMPARE(sb.toolTip(), QStringLiteral("Invalid value entered. Valid value: %1").arg(0));
	QCOMPARE(sb.m_waitFeedback, false);

	sb.keyPressEvent(&event);
	QCOMPARE(valueChangedCounter, 3);
	QCOMPARE(lastValue, 8);
	QCOMPARE(sb.toolTip(), sb.errorToString(NumberSpinBox::Errors::NoError));
	QCOMPARE(sb.m_waitFeedback, false);
}

// set value called directly when valueChanged() is called. This can happen if the other side directly sets another
// value, because the received value is invalid.
void SpinBoxTest::numberSpinBoxFeedback2() {
	NumberSpinBox sb(5);
	sb.setFeedback(true);

	int valueChangedCounter = 0;
	double lastValue = NAN;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&sb, &valueChangedCounter, &lastValue](double value) {
		valueChangedCounter++;
		lastValue = value; // value is 6
		QVERIFY(5 != value);
		sb.setValue(5); // Important other value
	});

	sb.lineEdit()->setCursorPosition(1);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&event);

	QCOMPARE(valueChangedCounter, 1);
	QCOMPARE(sb.toolTip(), i18n("Invalid value entered. Valid value: %1", 5));
}

void SpinBoxTest::numberSpinBoxFeedbackCursorPosition() {
	NumberSpinBox sb(5.11);
	sb.setFeedback(true);

	int valueChangedCounter = 0;
	// double lastValue = NAN;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&sb, &valueChangedCounter](double value) {
		valueChangedCounter++;
		sb.setValue(value);
	});

	sb.lineEdit()->setCursorPosition(3); // after first 1
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&event);

	QCOMPARE(valueChangedCounter, 1);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 3);
	QCOMPARE(sb.value(), 5.21);
	QCOMPARE(sb.toolTip(), QStringLiteral(""));

	// another time key up
	sb.keyPressEvent(&event);

	QCOMPARE(valueChangedCounter, 2);
	QCOMPARE(sb.lineEdit()->cursorPosition(), 3);
	QCOMPARE(sb.value(), 5.31);
	QCOMPARE(sb.toolTip(), QStringLiteral(""));
}

void SpinBoxTest::numberSpinBoxFeedbackCursorPosition2() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	p->setType(CartesianPlot::Type::TwoAxes); // Otherwise no axis are created
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* curve{new XYEquationCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(p->defaultCoordinateSystemIndex());
	p->addChild(curve);

	XYEquationCurve::EquationData data;
	data.min = QStringLiteral("1");
	data.max = QStringLiteral("2");
	data.count = 1000;
	data.expression1 = QStringLiteral("x");
	curve->setEquationData(data);
	curve->recalculate();

	CHECK_RANGE(p, curve, Dimension::X, 1.0, 2.0);
	CHECK_RANGE(p, curve, Dimension::Y, 1.0, 2.0);

	CartesianPlotDock d(nullptr);

	d.setPlots({p});
	d.setPlots({p}); // Important to do it a second time to see that the connections are cleared bevore connecting again

	QCOMPARE(d.ui.sbPaddingHorizontal->lineEdit()->text(), QStringLiteral("1.5 cm"));
	d.ui.sbPaddingHorizontal->lineEdit()->setCursorPosition(3);

	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	d.ui.sbPaddingHorizontal->keyPressEvent(&event);

	QCOMPARE(d.ui.sbPaddingHorizontal->lineEdit()->text(), QStringLiteral("1.6 cm"));
	QCOMPARE(d.ui.sbPaddingHorizontal->lineEdit()->cursorPosition(), 3);
}

// \brief SpinBoxTest::numberSpinBoxDecimals
// Check that application shows the correct value, even decimals is set to zero
void SpinBoxTest::numberSpinBoxDecimals2() {
	NumberSpinBox sb;
	sb.setMinimum(-10);
	sb.setDecimals(0);
	sb.setValue(-1);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-1"));
}

void SpinBoxTest::numberSpinBoxScrollingNegToPos() {
	NumberSpinBox sb;
	sb.setMinimum(-10);
	sb.setValue(-1.01);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-1.01"));

	sb.lineEdit()->setCursorPosition(2);
	QKeyEvent upEvent(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&upEvent);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-0.01"));
	QCOMPARE(sb.lineEdit()->cursorPosition(), 2); // cursor should not be at position of the comma but behind the 0

	QKeyEvent downEvent(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&downEvent);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-1.01"));
	QCOMPARE(sb.lineEdit()->cursorPosition(), 2); // cursor should not be at position of the minus but behind the 1
}

void SpinBoxTest::numberSpinBoxScrollingNegToPos2() {
	NumberSpinBox sb;
	sb.setMinimum(-10);
	sb.setValue(0.13);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("0.13"));

	sb.lineEdit()->setCursorPosition(1);
	QKeyEvent downEvent(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&downEvent);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-1.13")); // The first number should change not (0.13-1 = -0.87)
	QCOMPARE(sb.lineEdit()->cursorPosition(), 2);

	QKeyEvent upEvent(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&upEvent);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-0.13"));
	QCOMPARE(sb.lineEdit()->cursorPosition(), 2);

	sb.keyPressEvent(&upEvent);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("1.13"));
	QCOMPARE(sb.lineEdit()->cursorPosition(), 1);
}

void SpinBoxTest::numberSpinBoxScrollingNegativeValues() {
	NumberSpinBox sb;
	sb.setMinimum(-20);
	sb.setValue(-9.81);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-9.81"));

	sb.lineEdit()->setCursorPosition(2);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&event);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-10.81"));
	QCOMPARE(sb.lineEdit()->cursorPosition(), 3);

	sb.keyPressEvent(&event);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("-11.81"));
	QCOMPARE(sb.lineEdit()->cursorPosition(), 3);
}

void SpinBoxTest::numberSpinBoxMinimumFeedback() {
	// Limit to min/max when stepping in the spinbox
	// instead of raising an error
	NumberSpinBox sb;
	sb.setMinimum(0);
	sb.setValue(1.01);
	sb.setFeedback(true);

	int valueChangedCounter = 0;
	double lastValue = NAN;
	connect(&sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [&valueChangedCounter, &lastValue](double value) {
		valueChangedCounter++;
		lastValue = value;
	});

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("1.01"));

	sb.lineEdit()->setCursorPosition(1);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&event);

	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("0.01"));
	QCOMPARE(sb.lineEdit()->cursorPosition(), 1);
	QCOMPARE(valueChangedCounter, 1);
	QCOMPARE(sb.toolTip(), QString());

	sb.keyPressEvent(&event);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("0.01"));
	QCOMPARE(sb.lineEdit()->cursorPosition(), 1);
	QCOMPARE(valueChangedCounter, 1);
	QCOMPARE(sb.toolTip(), QString());

	sb.lineEdit()->setCursorPosition(4);
	sb.keyPressEvent(&event);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("0.00"));
	QCOMPARE(sb.lineEdit()->cursorPosition(), 4);
	QCOMPARE(valueChangedCounter, 2);
	QCOMPARE(sb.toolTip(), QString());

	sb.keyPressEvent(&event);
	QCOMPARE(sb.lineEdit()->text(), QStringLiteral("0.00"));
	QCOMPARE(sb.lineEdit()->cursorPosition(), 4);
	QCOMPARE(valueChangedCounter, 2);
	QCOMPARE(sb.toolTip(), QString());
}

// \brief SpinBoxTest::numberSpinBoxDecimalsMinMax
// In the default implementation of QDoubleSpinbox the maximum and
// minimums are round to the decimals. The numberspinbox should not do
// this
void SpinBoxTest::numberSpinBoxDecimalsMinMax() {
	NumberSpinBox sb;
	sb.setDecimals(2);
	sb.setMaximum(1.289343892e-10);
	QCOMPARE(sb.maximum(), 1.289343892e-10); // not rounded!

	sb.setMinimum(1.289343892e-15);
	QCOMPARE(sb.minimum(), 1.289343892e-15); // not rounded!
}

void SpinBoxTest::thousandSeparator() {
	{
		NumberSpinBox sb;
		sb.setLocale(QLocale(QLocale::German));
		QCOMPARE(sb.locale().decimalPoint(), QLatin1Char(','));
		QCOMPARE(sb.locale().groupSeparator(), QLatin1Char('.'));

		sb.setValue(50000);
		QCOMPARE(sb.text(), QStringLiteral("50.000"));

		sb.lineEdit()->setCursorPosition(0);
		QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_1, Qt::KeyboardModifier::NoModifier, QStringLiteral("1"));
		sb.keyPressEvent(&event);

		QCOMPARE(sb.value(), 150000);
		QCOMPARE(sb.text(), QStringLiteral("150.000"));
	}
}

void SpinBoxTest::thousandSeparatorNegative() {
	NumberSpinBox sb;
	sb.setLocale(QLocale(QLocale::German));
	QCOMPARE(sb.locale().decimalPoint(), QLatin1Char(','));
	QCOMPARE(sb.locale().groupSeparator(), QLatin1Char('.'));

	sb.setValue(-50000.1);
	QCOMPARE(sb.text(), QStringLiteral("-50.000,1"));

	sb.lineEdit()->setCursorPosition(0);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Delete, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&event);

	QCOMPARE(sb.value(), 50000.1);
	QCOMPARE(sb.text(), QStringLiteral("50.000,1"));
}

void SpinBoxTest::thousandSeparatorScrolling() {
	NumberSpinBox sb;
	sb.setLocale(QLocale(QLocale::German));
	QCOMPARE(sb.locale().decimalPoint(), QLatin1Char(','));
	QCOMPARE(sb.locale().groupSeparator(), QLatin1Char('.'));

	sb.setValue(90000.1);
	QCOMPARE(sb.text(), QStringLiteral("90.000,1"));

	sb.lineEdit()->setCursorPosition(1);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&event);

	QCOMPARE(sb.value(), 100000.1);
	QCOMPARE(sb.text(), QStringLiteral("100.000,1"));
}

void SpinBoxTest::thousandSeparatorScrolling2() {
	NumberSpinBox sb;
	sb.setLocale(QLocale(QLocale::German));
	QCOMPARE(sb.locale().decimalPoint(), QLatin1Char(','));
	QCOMPARE(sb.locale().groupSeparator(), QLatin1Char('.'));

	sb.setValue(24000.1);
	QCOMPARE(sb.text(), QStringLiteral("24.000,1"));

	sb.lineEdit()->setCursorPosition(2);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&event);

	QCOMPARE(sb.value(), 25000.1);
	QCOMPARE(sb.text(), QStringLiteral("25.000,1"));
}

void SpinBoxTest::thousandSeparatorScrollingSeparatorPosition() {
	QSKIP("Undecided what to do", QTest::SkipSingle);
	NumberSpinBox sb;
	sb.setLocale(QLocale(QLocale::German));
	QCOMPARE(sb.locale().decimalPoint(), QLatin1Char(','));
	QCOMPARE(sb.locale().groupSeparator(), QLatin1Char('.'));

	sb.setValue(50000);
	QCOMPARE(sb.text(), QStringLiteral("50.000"));

	sb.lineEdit()->setCursorPosition(3);
	QKeyEvent event(QKeyEvent::Type::KeyPress, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
	sb.keyPressEvent(&event);

	QCOMPARE(sb.value(), 50000);
	QCOMPARE(sb.text(), QStringLiteral("50.000"));
}

QTEST_MAIN(SpinBoxTest)
