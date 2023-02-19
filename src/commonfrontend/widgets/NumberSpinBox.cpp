/*
	File                 : NumberSpinBox.cpp
	Project              : LabPlot
	Description          : widget for setting numbers with a spinbox
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "NumberSpinBox.h"

#include "backend/gsl/ExpressionParser.h"
#include "backend/lib/Common.h"
#include "backend/lib/macrosWarningStyle.h"
#include "tools/EquationHighlighter.h"

#include "klocalizedstring.h"

#include <QApplication>
#include <QCompleter>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
#include <QLocale>
#include <QString>
#include <QStringRef>

#include <cmath>
#include <limits>

namespace {
const QChar tempSymbol = 26;
} // anonymous namespace

NumberSpinBox::NumberSpinBox(QWidget* parent)
	: QDoubleSpinBox(parent) {
	init(QLatin1String("0"), false);
}

NumberSpinBox::NumberSpinBox(double initValue, QWidget* parent)
	: QDoubleSpinBox(parent) {
	init(QString::number(initValue), false);
}

NumberSpinBox::NumberSpinBox(const QString& initExpression, QWidget* parent)
	: QDoubleSpinBox(parent) {
	init(initExpression, false);
}

NumberSpinBox::NumberSpinBox(const QString& initExpression, bool feedback, QWidget* parent)
	: QDoubleSpinBox(parent) {
	init(initExpression, feedback);
}

void NumberSpinBox::init(const QString& initExpression, bool feedback) {
	setFocusPolicy(Qt::StrongFocus);
	setValue(Common::ExpressionValue(initExpression, 0, Worksheet::Unit::None));
	m_feedback = feedback; // must be after setExpression()!
	setInvalid(Errors::NoErrorNumeric);
	setDecimals(2);

	// m_highlighter = new EquationHighlighter(this);

	QStringList list = ExpressionParser::getInstance()->functions();
	// append description
	for (auto& s : list)
		s.append(ExpressionParser::functionArgumentString(s, XYEquationCurve::EquationType::Cartesian) + QStringLiteral(" - ")
				 + ExpressionParser::getInstance()->functionDescription(s));
	QStringList constants = ExpressionParser::getInstance()->constants();
	for (auto& s : constants) {
		if (s != QLatin1String("..."))
			s.append(QStringLiteral(" - ") + ExpressionParser::getInstance()->constantDescription(s));
	}
	list.append(constants);

	// setTabChangesFocus(true);

	m_completer = new QCompleter(list, this);
	m_completer->setWidget(this);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);

	connect(m_completer, QOverload<const QString&>::of(&QCompleter::activated), this, &NumberSpinBox::insertCompletion);
	connect(this->lineEdit(), &QLineEdit::cursorPositionChanged, m_highlighter, &EquationHighlighter::rehighlight);
}

void NumberSpinBox::insertCompletion(const QString& completion) {
	// QTextCursor tc{lineEdit()->cursorPosition()};

	// // remove description
	int nameLength = completion.indexOf(QLatin1String(" - "));
	QString name = completion;
	name.truncate(nameLength);

	int extra = name.length() - m_completer->completionPrefix().length();
	// tc.movePosition(QTextCursor::Left);
	// tc.movePosition(QTextCursor::EndOfWord);
	// tc.insertText(name.right(extra));
	// setTextCursor(tc);
	const int cp = lineEdit()->cursorPosition();
	if (cp > 0)
		lineEdit()->setCursorPosition(cp - 1);
	lineEdit()->cursorWordForward(false);
	lineEdit()->insert(name.right(extra));
}

QString NumberSpinBox::errorToString(Errors e) {
	switch (e) {
	case Errors::Min:
		return i18n("Minimum allowed value: %1").arg(QString::number(minimum()));
	case Errors::Max:
		return i18n("Maximum allowed value: %1").arg(QString::number(maximum()));
	case Errors::Invalid:
		return i18n("The value does not represent a valid number or expression");
	case Errors::NoNumber:
		return i18n("No number entered");
	case Errors::NoErrorNumeric:
		// Fall through
	case Errors::NoErrorExpression:
		return {};
	}
	return i18n("Unhandled error");
}

void NumberSpinBox::keyPressEvent(QKeyEvent* event) {
	if (!m_expression) {
		switch (event->key()) {
		case Qt::Key_Down:
			decreaseValue();
			return;
		case Qt::Key_Up:
			increaseValue();
			return;
		default:
			QDoubleSpinBox::keyPressEvent(event);
			break;
		}
	} else
		QDoubleSpinBox::keyPressEvent(event);
	QString text = lineEdit()->text();
	Common::ExpressionValue v;
	Errors e = validate(text, v);
	setInvalid(e);
	if (!error(e) && m_value.expression() != v.expression()) {
		m_value = v;
		m_expression = e == Errors::NoErrorExpression;
		valueChanged();
	}
}

QString NumberSpinBox::convertToInternalRep(const QString& expression) const {
	const auto l = QLocale();
	QString e = expression;
	if (l.decimalPoint() != QLatin1Char('.')) {
		// 2.000,5 -> 2,000.5
		e.replace(QLatin1Char('.'), tempSymbol).replace(l.decimalPoint(), QLatin1Char('.')).replace(tempSymbol, QLatin1Char(','));
	}
	return e;
}

QString NumberSpinBox::convertFromInternalRep(const QString& expression) const {
	const auto l = QLocale();
	QString e = expression;
	if (l.decimalPoint() != QLatin1Char('.')) {
		// 2,000.5 -> 2.000,5
		e.replace(QLatin1Char(','), tempSymbol).replace(QLatin1Char('.'), l.decimalPoint()).replace(tempSymbol, QLatin1Char(','));
	}
	return e;
}

void NumberSpinBox::wheelEvent(QWheelEvent* event) {
	if ((m_strongFocus && !hasFocus()) || m_expression)
		event->ignore();
	else
		QDoubleSpinBox::wheelEvent(event);
}

void NumberSpinBox::stepBy(int steps) {
	// used when scrolling
	Errors e = step(steps);
	if (e == Errors::Min || e == Errors::Max)
		setInvalid(Errors::NoErrorNumeric);
	else if (!error(e)) {
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
	p.fractionPos = v_str.indexOf(locale().decimalPoint());
	p.exponentPos = v_str.indexOf(QLatin1Char('e'), p.fractionPos > 0 ? p.fractionPos : 0, Qt::CaseInsensitive);
	const auto number_length = v_str.length();

	bool ok;

	if (v_str.at(0) == QLatin1Char('+') || v_str.at(0) == QLatin1Char('-'))
		p.integerSign = v_str.at(0);

	p.fraction = false;
	// integer properties
	if (p.fractionPos >= 0) {
		p.fraction = true;
		const auto integer_str = v_str.mid(!p.integerSign.isNull(), p.fractionPos - !p.integerSign.isNull());
		p.integer = integer_str.toInt(&ok);
		if (!ok)
			return false;
		p.intergerDigits = integer_str.length();

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
		p.integer = integer_str.toInt(&ok);
		if (!ok)
			return false;
		p.intergerDigits = integer_str.length();
	} else {
		const auto integer_str = v_str.mid(!p.integerSign.isNull(), number_length - !p.integerSign.isNull());
		p.integer = integer_str.toInt(&ok);
		if (!ok)
			return false;
		p.intergerDigits = integer_str.length();
	}

	if (p.exponentPos > 0) {
		bool ok;
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
	QStringRef text(&t);

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
	return QStringLiteral("");
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
		return Errors::NoErrorNumeric;

	QString v_str = strip(lineEdit()->text());

	NumberProperties p;
	bool ok;
	double origValue = locale().toDouble(v_str, &ok);
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
		return Errors::NoErrorNumeric;

	bool before_comma = comma >= 0 && cursorPos - 1 < comma;
	bool before_exponent = exponentialIndex >= 0 && cursorPos - 1 < exponentialIndex;

	const auto& l = v_str.split(QLatin1Char('e'), Qt::KeepEmptyParts, Qt::CaseInsensitive);

	double integerFraction = locale().toDouble(l.at(0));
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
		increase = steps * std::pow(10, initial - cursorPos);

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
	return Errors::NoErrorNumeric;
}

/*!
 * \brief NumberSpinBox::validate
 * \param input input string including suffix and prefix
 * \param value output value
 * \param valueStr final expression
 * \return
 */
NumberSpinBox::Errors NumberSpinBox::validate(QString& input, Common::ExpressionValue& v) const {
	double value;
	const QString expression = convertToInternalRep(strip(input));
	if (expression.isEmpty())
		return Errors::NoNumber;
	NumberProperties p;
	bool ok;
	value = locale().toDouble(expression, &ok);
	bool expressionValid = false;
	if (!ok) {
		// might be an expression
		ExpressionParser* parser = ExpressionParser::getInstance();
		if (!parser->evaluateCartesian(expression, value))
			return Errors::Invalid;
		expressionValid = true;
	} else {
		if (!properties(expression, p))
			return Errors::Invalid;
	}

	if (value > maximum())
		return Errors::Max;

	if (value < minimum())
		return Errors::Min;

	v = Common::ExpressionValue(expression, value, Worksheet::Unit::None);

	if (expressionValid)
		return Errors::NoErrorExpression;
	return Errors::NoErrorNumeric;
}

/*!
 * \brief Validates the current expression if the text was changed and highlights the text field if the expression is invalid.
 * \param force forces the validation and highlighting when no text changes were made, used when new parameters/variables were provided
 */
bool NumberSpinBox::validateExpression(const QString& text, bool force) const {
	// check whether the expression was changed or only the formatting
	bool textChanged{(text != m_value.expression()) ? true : false};
	bool valid = false;
	if (textChanged || force) {
		valid = ExpressionParser::getInstance()->isValid(text, {});
		// m_currentExpression = text;
	}
	// if (textChanged)
	//     Q_EMIT expressionChanged();
	return valid;
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
	Common::ExpressionValue ve;
	const auto e = validate(input, ve);
	// Needed because when loosing focus the lineEdit() would reset to the previous value
	return !error(e) ? QValidator::State::Acceptable : QValidator::State::Intermediate;
}

void NumberSpinBox::setText(const QString& text) {
	lineEdit()->setText(prefix() + text + suffix());
}

void NumberSpinBox::setScaling(double scale) {
	if (m_value.expressionEmpty())
		m_value.setValue(m_value.value<double>() * scale);
	else
		m_value = Common::ExpressionValue(m_value.expression() + QStringLiteral(" * %1").arg(QString::number(scale)), 0, Worksheet::Unit::None);
}

bool NumberSpinBox::setValue(double value) {
	return setValue(Common::ExpressionValue(value));
}

bool NumberSpinBox::setValue(const Common::ExpressionValue& ev) {
	if (m_feedback && m_waitFeedback) {
		m_waitFeedback = false;

		if (ev.expressionEmpty()) {
			if (m_value.isDouble() != ev.isDouble()) {
				setInvalid(i18n("Internal error")); // Should never happen!
				return false;
			}
			if (m_value.isDouble()) {
				if (m_value.value<double>() != ev.value<double>()) {
					setInvalid(i18n("Invalid value entered. Valid value: %1", ev.toString()));
					return false;
				}
			} else if (m_value.value<qint64>() != ev.value<qint64>()) {
				setInvalid(i18n("Invalid value entered. Valid value: %1", ev.toString()));
				return false;
			}
		} else {
			const bool valid = validateExpression(ev.expression(), false);
			if (!valid || m_value.expression().compare(ev.expression()) != 0) {
				setInvalid(i18n("Invalid expression. Valid expression: ", m_value.expression()));
				return false;
			}
		}
		return true;
	}

	m_value = ev;
	setText(ev.toString());
	return true;
}

// bool NumberSpinBox::setExpression(const QString& expression, bool skipValidation) {
//	if (!validateExpression(expression, false))
//		return false;

//	const QString expr = convertFromInternalRep(expression);

//	if (m_feedback && m_waitFeedback) {
//		m_waitFeedback = false;
//		if (m_Value.expression.compare(expression) != 0) {
//			setInvalid(i18n("Invalid value entered. Valid value: %1", expr));
//			return false;
//		}
//		return true;
//	}

//	setText(expr);
//	return true;
//}

const Common::ExpressionValue& NumberSpinBox::value() const {
	return m_value;
}

QString NumberSpinBox::expression() {
	return strip(text());
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

void NumberSpinBox::setClearButtonEnabled(bool enable) {
	lineEdit()->setClearButtonEnabled(enable);
}

QAbstractSpinBox::StepEnabled NumberSpinBox::stepEnabled() const {
	return QAbstractSpinBox::StepEnabledFlag::StepUpEnabled | QAbstractSpinBox::StepEnabledFlag::StepDownEnabled; // for testing
}

void NumberSpinBox::valueChanged() {
	if (m_feedback)
		m_waitFeedback = true;
	qDebug() << "Value: " << m_value.value<double>();
	emit valueChanged(m_value);
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

bool NumberSpinBox::error(Errors e) const {
	if (e != Errors::NoErrorExpression && e != Errors::NoErrorNumeric)
		return true;
	return false;
}
