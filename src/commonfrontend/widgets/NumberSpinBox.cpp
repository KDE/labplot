#include "NumberSpinBox.h"

#include "backend/lib/macros.h"

#include <limits>

#include <QKeyEvent>
#include <QLineEdit>
#include <QLocale>
#include <QString>
#include <QtMath>

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
	mFeedback = feedback; // must be after setValue()!
	setInvalid(Errors::NoError);
	setMinimum(std::numeric_limits<double>::lowest());
	setMaximum(std::numeric_limits<double>::max());
	setDecimals(std::numeric_limits<int>::max()); // Important, because in QDoubleSpinBox round() with decimals will be done
}

QString NumberSpinBox::errorToString(Errors e) {
	switch (e) {
	case Errors::Min:
		return tr("Value lower than minimum allowed: %1").arg(QString::number(minimum()));
	case Errors::Max:
		return tr("Value higher than maximum allowed: %1").arg(QString::number(maximum()));
	case Errors::Invalid:
		return tr("The value does not represent a valid number");
	case Errors::NoNumber:
		return tr("No number entered");
	case Errors::NoError:
		return QStringLiteral("");
	}
}

void NumberSpinBox::keyPressEvent(QKeyEvent* event) {
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
	QString text = lineEdit()->text();
	double v;
	QString valueStr;
	Errors e = validate(text, v, valueStr);
	setInvalid(e);
	if (e == Errors::NoError) {
		mValueStr = valueStr;
		valueChanged();
	}
}

void NumberSpinBox::wheelEvent(QWheelEvent* event) {
	if (mStrongFocus && !hasFocus())
		event->ignore();
	else
		QDoubleSpinBox::wheelEvent(event);
}

void NumberSpinBox::stepBy(int steps) {
	// used when scrolling
	Errors e = step(steps);
	setInvalid(e);
	if (e == Errors::NoError)
		valueChanged();
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
	p.exponentPos = v_str.indexOf('e', p.fractionPos > 0 ? p.fractionPos : 0, Qt::CaseInsensitive);
	const auto number_length = v_str.length();

	bool ok;

	if (v_str.at(0) == '+' || v_str.at(0) == '-') {
		p.integerSign = v_str.at(0);
	}

	p.fraction = false;
	// integer properties
	if (p.fractionPos >= 0) {
		p.fraction = true;
		const auto integer_str = v_str.mid(!p.integerSign.isNull(), p.fractionPos - !p.integerSign.isNull());
		p.integer = integer_str.toInt(&ok);
		if (!ok)
			return false;
		p.intergerDigits = integer_str.length();

		QString fraction_str = "";
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
		if (v_str.at(p.exponentPos + 1) == '+' || v_str.at(p.exponentPos + 1) == '-')
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
		number = QString("%1").arg(int(integerFraction));
	}

	if (p.exponentLetter != QChar::Null) {
		const auto e = QString("%L1").arg(exponent, p.exponentDigits + (p.exponentSign == '-'), 10, QLatin1Char('0'));
		QString sign = "";
		if (exponent >= 0 && !p.exponentSign.isNull())
			sign = "+";
		number += p.exponentLetter + sign + e;
	}

	if (p.integerSign == '+')
		number.prepend('+');

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
	return mValueStr;
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
	locale().toDouble(v_str, &ok);
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

	const auto& l = v_str.split('e', 0, Qt::CaseInsensitive);

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
		increase = steps * qPow(10, initial - cursorPos);

		// from 0.1 with step -1 the desired result shall be -1.1 not -0.9
		if (integerFraction + increase > 0)
			integerFraction += increase;
		else {
			int integer = static_cast<int>(integerFraction);
			integerFraction = integer + increase + (integer - integerFraction);
		}
	} else if (comma >= 0 && (exponentialIndex == -1 || before_exponent)) {
		// fraction
		increase = steps * qPow(10, -(cursorPos - 1 - comma));
		integerFraction += increase;
	} else {
		// exponent
		increase = end - cursorPos;
		const auto calc = steps * qPow(10, increase);
		exponent += calc;

		// double max value 1.7976931348623157E+308
		if (abs(exponent) > 307) {
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

	double v = integerFraction * qPow(10, exponent);

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
	if (newPos == 1 && !p.integerSign.isNull() && number.length() > 1)
		newPos = 2;

	lineEdit()->setCursorPosition(newPos + prefix().size());

	QDoubleSpinBox::setValue(v);
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

	if (value > maximum())
		return Errors::Max;

	if (value < minimum())
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
	mValueStr = text;
	lineEdit()->setText(prefix() + text + suffix());
}

bool NumberSpinBox::setValue(double v) {
	if (mFeedback && mWaitFeedback) {
		mWaitFeedback = false;
		if (!qFuzzyCompare(v, value())) {
			setInvalid(tr("Invalid value entered. Valid value: %1").arg(v));
			return false;
		}
		return true;
	}

	setText(QString("%L1").arg(v));
	QDoubleSpinBox::setValue(v);
	return true;
}

void NumberSpinBox::setFeedback(bool enable) {
	mFeedback = enable;
}

void NumberSpinBox::setStrongFocus(bool enable) {
	mStrongFocus = enable;
	if (enable)
		setFocusPolicy(Qt::StrongFocus);
	else
		setFocusPolicy(Qt::WheelFocus);
}

QAbstractSpinBox::StepEnabled NumberSpinBox::stepEnabled() const {
	return QAbstractSpinBox::StepEnabledFlag::StepUpEnabled | QAbstractSpinBox::StepEnabledFlag::StepDownEnabled; // for testing
}

void NumberSpinBox::valueChanged() {
	if (mFeedback)
		mWaitFeedback = true;
	qDebug() << "Value: " << value();
	emit valueChanged(value());
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
