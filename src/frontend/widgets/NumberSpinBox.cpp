/*
	File                 : NumberSpinBox.cpp
	Project              : LabPlot
	Description          : widget for setting numbers with a spinbox
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "NumberSpinBox.h"

#include "backend/lib/macrosWarningStyle.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
#include <QLocale>
#include <QString>
#include <QStringView>

#include <cmath>
#include <limits>

NumberSpinBox::NumberSpinBox(QWidget* parent)
	: QDoubleSpinBox(parent) {
	init(0, false);
}

NumberSpinBox::NumberSpinBox(double initValue, QWidget* parent)
	: QDoubleSpinBox(parent) {
	init(initValue, false);
}

NumberSpinBox::NumberSpinBox(double initValue, bool feedback, QWidget* parent)
	: QDoubleSpinBox(parent) {
	init(initValue, feedback);
}

void NumberSpinBox::init(double initValue, bool feedback) {
	setFocusPolicy(Qt::StrongFocus);
	setValue(initValue);
	m_feedback = feedback; // must be after setValue()!
	setInvalid(Errors::NoError);
	setDecimals(2);
	lineEdit()->setValidator(nullptr);
}

QString NumberSpinBox::errorToString(Errors e) {
	switch (e) {
	case Errors::Min:
		return i18n("Minimum allowed value: %1").arg(QString::number(minimum()));
	case Errors::Max:
		return i18n("Maximum allowed value: %1").arg(QString::number(maximum()));
	case Errors::Invalid:
		return i18n("The value does not represent a valid number");
	case Errors::NoNumber:
		return i18n("No number entered");
	case Errors::NoError:
		return {};
	}
	return i18n("Unhandled error");
}

void NumberSpinBox::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
	case Qt::Key_Down:
		decreaseValue();
		return;
	case Qt::Key_Up:
		increaseValue();
		return;
	default: {
		if (lineEdit()->selectionLength() > 0) {
			int selectionStart = qMax(lineEdit()->selectionStart(), prefix().length());
			selectionStart = qMin(selectionStart, lineEdit()->text().length() - suffix().length());

			int selectionEnd = qMax(lineEdit()->selectionEnd(), prefix().length());
			selectionEnd = qMin(selectionEnd, lineEdit()->text().length() - suffix().length());

			lineEdit()->setSelection(selectionStart, selectionEnd - selectionStart);
		} else {
			int cursorPos = qMax(lineEdit()->cursorPosition(), prefix().length());
			cursorPos = qMin(cursorPos, lineEdit()->text().length() - suffix().length());
			lineEdit()->setCursorPosition(cursorPos);
		}
		QDoubleSpinBox::keyPressEvent(event);
		break;
	}
	}
	QString text = lineEdit()->text();
	double v;
	QString valueStr;
	Errors e = validate(text, v, valueStr);
	setInvalid(e);
	if (e == Errors::NoError && v != m_value && m_valueStr != valueStr) {
		m_valueStr = valueStr;
		m_value = v;
		valueChanged();
	}
}

void NumberSpinBox::wheelEvent(QWheelEvent* event) {
	if (m_strongFocus && !hasFocus())
		event->ignore();
	else
		QDoubleSpinBox::wheelEvent(event);
}

void NumberSpinBox::stepBy(int steps) {
	// used when scrolling
	Errors e = step(steps);
	if (e == Errors::Min || e == Errors::Max)
		setInvalid(Errors::NoError);
	else if (e == Errors::NoError) {
		setInvalid(e);
		valueChanged();
	} else
		setInvalid(e);
}

void NumberSpinBox::increaseValue() {
	stepBy(1);
}

void NumberSpinBox::decreaseValue() {
	stepBy(-1);
}

/*!
 * \brief NumberSpinBox::properties
 * Determine the properties of a numeric value represented in a string
 * \param v_str string representation of a numeric value
 * \param p properties of the value
 * \return
 */
bool NumberSpinBox::properties(const QString& v_str, NumberProperties& p) const {
	const auto decimalpoint = locale().decimalPoint();
	p.fractionPos = v_str.indexOf(decimalpoint);
	p.exponentPos = v_str.indexOf(QLatin1Char('e'), p.fractionPos > 0 ? p.fractionPos : 0, Qt::CaseInsensitive);
	p.groupSeparators = v_str.indexOf(locale().groupSeparator()) != -1 ? true : false;
	const auto number_length = v_str.length();

	bool ok;

	if (v_str.at(0) == QLatin1Char('+') || v_str.at(0) == QLatin1Char('-'))
		p.integerSign = v_str.at(0);

	p.fraction = false;
	// integer properties
	if (p.fractionPos >= 0) {
		p.fraction = true;
		const auto integer_str = v_str.mid(!p.integerSign.isNull(), p.fractionPos - !p.integerSign.isNull());
		p.integer = locale().toInt(integer_str, &ok);
		if (!ok)
			return false;
		p.intergerDigits = integer_str.length() - p.groupSeparators;

		QString fraction_str;
		if (number_length - 1 > p.fractionPos) {
			int end = number_length;
			if (p.exponentPos > 0)
				end = p.exponentPos;
			fraction_str = v_str.mid(p.fractionPos + 1, end - (p.fractionPos + 1));
		}
		p.fractionDigits = fraction_str.length();
	} else if (p.exponentPos > 0) {
		const auto integer_str = v_str.mid(!p.integerSign.isNull(), p.exponentPos - !p.integerSign.isNull());
		p.integer = locale().toInt(integer_str, &ok);
		if (!ok)
			return false;
		p.intergerDigits = integer_str.length() - p.groupSeparators;
	} else {
		const auto integer_str = v_str.mid(!p.integerSign.isNull(), number_length - !p.integerSign.isNull());
		p.integer = locale().toInt(integer_str, &ok);
		if (!ok)
			return false;
		p.intergerDigits = integer_str.length() - p.groupSeparators;
	}

	if (p.exponentPos > 0) {
		if (v_str.at(p.exponentPos + 1) == QLatin1Char('+') || v_str.at(p.exponentPos + 1) == QLatin1Char('-'))
			p.exponentSign = v_str.at(p.exponentPos + 1);
		const QString& e = v_str.mid(p.exponentPos + 1 + !p.exponentSign.isNull(), number_length - (p.exponentPos + 1 + !p.exponentSign.isNull()));
		p.exponentDigits = e.length();
		p.exponent = e.toInt(&ok);
		if (!ok)
			return false;
		p.exponentLetter = v_str.at(p.exponentPos);
	}
	return true;
}

/*!
 * \brief NumberSpinBox::createStringNumber
 * Create a string with integer, fraction and exponent part but with the constraint to match
 * the properties of \p p
 * \param integerFraction integer and fraction part of the numeric value
 * \param exponent exponent part of the numeric value
 * \param p value properties
 * \return
 */
QString NumberSpinBox::createStringNumber(double integerFraction, int exponent, const NumberProperties& p) const {
	QString number;
	if (p.fraction) {
		number = locale().toString(integerFraction, 'f', p.fractionDigits);
		if (p.fractionDigits == 0)
			number.append(locale().decimalPoint());
	} else {
		if (p.groupSeparators)
			number = locale().toString(int(integerFraction));
		else
			number = QStringLiteral("%1").arg(int(integerFraction));
	}

	if (p.exponentLetter != QChar::Null) {
		const auto e = QStringLiteral("%L1").arg(exponent, p.exponentDigits + (p.exponentSign == QLatin1Char('-')), 10, QLatin1Char('0'));
		QString sign;
		if (exponent >= 0 && !p.exponentSign.isNull())
			sign = QLatin1Char('+');
		number += p.exponentLetter + sign + e;
	}

	if (p.integerSign == QLatin1Char('+'))
		number.prepend(QLatin1Char('+'));

	return number;
}

QString NumberSpinBox::strip(const QString& t) const {
	// Copied from QAbstractSpinBox.cpp
	QStringView text(t);

	int size = text.size();
	const QString p = prefix();
	const QString s = suffix();
	bool changed = false;
	int from = 0;
	if (p.size() && text.startsWith(p)) {
		from += p.size();
		size -= from;
		changed = true;
	}
	if (s.size() && text.endsWith(s)) {
		size -= s.size();
		changed = true;
	}
	if (changed)
		text = text.mid(from, size);
	text = text.trimmed();
	return text.toString();
}

QString NumberSpinBox::textFromValue(double value) const {
	Q_UNUSED(value);
	return m_valueStr;
}

/*!
 * \brief NumberSpinBox::valueFromText
 * Will be called when value() is called
 * \param text
 * \return
 */
double NumberSpinBox::valueFromText(const QString& text) const {
	QString t = strip(text);
	double v = locale().toDouble(t);
	return v;
}

NumberSpinBox::Errors NumberSpinBox::step(int steps) {
	int cursorPos = lineEdit()->cursorPosition() - prefix().size();
	if (cursorPos < 0)
		cursorPos = 0;
	int end = lineEdit()->text().length() - suffix().size() - prefix().size();
	if (cursorPos > end)
		cursorPos = end;

	if (cursorPos == 0)
		return Errors::NoError;

	QString v_str = strip(lineEdit()->text());

	NumberProperties p;
	bool ok;
	const double origValue = locale().toDouble(v_str, &ok);
	if (!ok)
		return Errors::Invalid;
	if (!properties(v_str, p))
		return Errors::Invalid;

	const auto comma = p.fractionPos;
	const auto exponentialIndex = p.exponentPos;

	// cursor behind the integer sign
	// cursor behind the comma
	// cursor behind the exponent letter
	// cursor behind the exponent sign
	if ((cursorPos == 1 && !p.integerSign.isNull()) || (comma >= 0 && cursorPos - 1 == comma)
		|| (exponentialIndex > 0 && (cursorPos - 1 == exponentialIndex || (cursorPos - 1 == exponentialIndex + 1 && !p.exponentSign.isNull()))))
		return Errors::NoError;

	bool before_comma = comma >= 0 && cursorPos - 1 < comma;
	bool before_exponent = exponentialIndex >= 0 && cursorPos - 1 < exponentialIndex;

	const auto& l = v_str.split(QLatin1Char('e'), Qt::KeepEmptyParts, Qt::CaseInsensitive);

	const auto& integerString = l.at(0);
	double integerFraction = locale().toDouble(integerString);
	int exponent = 0;
	if (l.length() > 1)
		exponent = l.at(1).toInt();

	double increase;
	if (before_comma || (comma == -1 && before_exponent) || (comma == -1 && exponentialIndex == -1)) {
		// integer
		int initial;
		if (comma >= 0)
			initial = comma;
		else if (exponentialIndex >= 0)
			initial = exponentialIndex;
		else
			initial = end;
		if (!p.groupSeparators)
			increase = steps * std::pow(10, initial - cursorPos);
		else {
			const auto groupSeparator = locale().groupSeparator();
			int separatorsCount = 0;
			int separator_pos = integerString.indexOf(groupSeparator);
			while (separator_pos != -1) {
				if (separator_pos >= cursorPos)
					separatorsCount++;
				if (separator_pos + 1 < integerString.length())
					separator_pos = integerString.indexOf(groupSeparator, separator_pos + 1);
				else
					break;
			}
			increase = steps * std::pow(10, initial - cursorPos - separatorsCount);
		}

		// from 0.1 with step -1 the desired result shall be -1.1 not -0.9
		if ((integerFraction > 0 && integerFraction + increase > 0) || (integerFraction < 0 && integerFraction + increase < 0))
			integerFraction += increase;
		else {
			int integer = static_cast<int>(integerFraction);
			integerFraction = integer + increase + (integer - integerFraction);
		}
	} else if (comma >= 0 && (exponentialIndex == -1 || before_exponent)) {
		// fraction
		increase = steps * std::pow(10, -(cursorPos - 1 - comma));
		integerFraction += increase;
	} else {
		// exponent
		increase = end - cursorPos;
		const auto calc = steps * std::pow(10, increase);
		exponent += calc;

		// double max value 1.7976931348623157E+308
		if (std::abs(exponent) > 307) {
			if (abs(integerFraction) > 1.7976931348623157)
				exponent = exponent > 0 ? 307 : -307;
			else
				exponent = exponent > 0 ? 308 : -308;
		}
	}
	if (integerFraction > std::numeric_limits<int>::max())
		integerFraction = std::numeric_limits<int>::max();
	else if (integerFraction < std::numeric_limits<int>::min())
		integerFraction = std::numeric_limits<int>::min();

	double v = integerFraction * std::pow(10, exponent);

	if (v > maximum())
		return Errors::Max;

	if (v < minimum())
		return Errors::Min;

	QString number = createStringNumber(integerFraction, exponent, p);
	setText(number);

	// Set cursor position
	auto newPos = number.length() - (end - cursorPos);
	if ((newPos == 0 && number.length() > 0))
		newPos = 1;
	if (newPos == 1 && !p.integerSign.isNull() && number.length() > 1 && origValue < 0 && v < 0)
		newPos = 2;

	lineEdit()->setCursorPosition(newPos + prefix().size());

	m_value = v;
	return Errors::NoError;
}

NumberSpinBox::Errors NumberSpinBox::validate(QString& input, double& value, QString& valueStr) const {
	valueStr = strip(input);
	if (valueStr.isEmpty())
		return Errors::NoNumber;
	NumberProperties p;
	bool ok;
	value = locale().toDouble(valueStr, &ok);
	if (!ok)
		return Errors::Invalid;
	if (!properties(valueStr, p))
		return Errors::Invalid;

	if (value > maximum() || value >= maximumNotEqual())
		return Errors::Max;

	if (value < minimum() || value <= minimumNotEqual())
		return Errors::Min;
	return Errors::NoError;
}

/*!
 * \brief NumberSpinBox::validate
 * Function which validates the user input. Reimplemented from QDoubleSpinBox
 * \param input
 * \param pos
 * \return
 */
QValidator::State NumberSpinBox::validate(QString& input, int& pos) const {
	Q_UNUSED(pos);
	double value;
	QString valueStr;
	const auto e = validate(input, value, valueStr);
	return e == Errors::NoError ? QValidator::State::Acceptable : QValidator::State::Intermediate;
}

void NumberSpinBox::setText(const QString& text) {
	m_valueStr = text;
	lineEdit()->setText(prefix() + text + suffix());
}

bool NumberSpinBox::setValue(double v) {
	if (m_feedback && m_waitFeedback) {
		m_waitFeedback = false;
		if (!qFuzzyCompare(v, value())) {
			setInvalid(i18n("Invalid value entered. Valid value: %1", v));
			return false;
		}
		return true;
	}

	setText(locale().toString(v, 'g'));
	m_value = v;
	valueChanged();
	return true;
}

double NumberSpinBox::minimum() const {
	return m_minimum;
}

void NumberSpinBox::setMinimum(double min) {
	m_minimum = min;
}

double NumberSpinBox::maximum() const {
	return m_maximum;
}

void NumberSpinBox::setMaximum(double max) {
	m_maximum = max;
}

double NumberSpinBox::minimumNotEqual() const {
	return m_minimumNotEqual;
}

void NumberSpinBox::setMinimumNotEqual(double min) {
	m_minimumNotEqual = min;
}

double NumberSpinBox::maximumNotEqual() const {
	return m_maximumNotEqual;
}

void NumberSpinBox::setMaximumNotEqual(double max) {
	m_maximumNotEqual = max;
}

void NumberSpinBox::setFeedback(bool enable) {
	m_feedback = enable;
}

bool NumberSpinBox::feedback() {
	return m_feedback;
}

void NumberSpinBox::setStrongFocus(bool enable) {
	m_strongFocus = enable;
	if (enable)
		setFocusPolicy(Qt::StrongFocus);
	else
		setFocusPolicy(Qt::WheelFocus);
}

double NumberSpinBox::value() {
	return m_value;
}

void NumberSpinBox::setClearButtonEnabled(bool enable) {
	lineEdit()->setClearButtonEnabled(enable);
}

QAbstractSpinBox::StepEnabled NumberSpinBox::stepEnabled() const {
	return QAbstractSpinBox::StepEnabledFlag::StepUpEnabled | QAbstractSpinBox::StepEnabledFlag::StepDownEnabled; // for testing
}

void NumberSpinBox::valueChanged() {
	if (m_feedback)
		m_waitFeedback = true;
	Q_EMIT valueChanged(value());
	m_waitFeedback = false;
}

void NumberSpinBox::setInvalid(const QString& str) {
	if (!str.isEmpty())
		SET_WARNING_PALETTE
	else
		setPalette(qApp->palette());
	setToolTip(str);
}

void NumberSpinBox::setInvalid(Errors e) {
	setInvalid(errorToString(e));
}
