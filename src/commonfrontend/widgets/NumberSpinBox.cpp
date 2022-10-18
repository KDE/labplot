#include "NumberSpinBox.h"

#include "backend/lib/macros.h"

#include <limits>

#include <QKeyEvent>
#include <QLineEdit>
#include <QString>
#include <QtMath>

NumberSpinBox::NumberSpinBox(QWidget* parent)
	: QAbstractSpinBox(parent) {
	setInvalid(true, tr("No number"));
}

void NumberSpinBox::keyPressEvent(QKeyEvent* event) {
	lineEdit()->cursorPosition();

	if (mInvalid) {
		QAbstractSpinBox::keyPressEvent(event);
		bool ok;
		double v = value(&ok);
		setInvalid(!ok, tr("Invalid number"));
		if (ok)
			emit valueChanged(v);
		return;
	}

	switch (event->key()) {
	case Qt::Key_Down:
		setInvalid(!decreaseValue(), tr("Invalid number"));
		return;
	case Qt::Key_Up:
		setInvalid(!increaseValue(), tr("Invalid number"));
		return;
	default:
		QAbstractSpinBox::keyPressEvent(event);
		break;
	}
	bool ok;
	double v = value(&ok);
	setInvalid(!ok, tr("Invalid number"));
	if (ok)
		emit valueChanged(v);
}

void NumberSpinBox::stepBy(int steps) {
	// used when scrolling

	setInvalid(!step(steps), tr("Invalid number"));
}

bool NumberSpinBox::increaseValue() {
	return step(1);
}

bool NumberSpinBox::decreaseValue() {
	return step(-1);
}

/*!
 * \brief NumberSpinBox::properties
 * Determine the properties of a numeric value represented in a string
 * \param v_str string representation of a numeric value
 * \param p properties of the value
 * \return
 */
bool NumberSpinBox::properties(const QString& v_str, NumberProperties& p) {
	p.fractionPos = v_str.indexOf('.'); // TODO: locale??
	p.exponentPos = v_str.indexOf('e', p.fractionPos > 0 ? p.fractionPos : 0, Qt::CaseInsensitive);
	const auto number_length = v_str.length();

	bool ok;
	p.value = v_str.toDouble(&ok);
	if (!ok)
		return false;

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
QString NumberSpinBox::createStringNumber(double integerFraction, int exponent, const NumberProperties& p) {
	QString number;
	if (p.fraction) {
		number = QString("%1").arg(integerFraction, 0, 'f', p.fractionDigits);
		if (p.fractionDigits == 0)
			number.append('.');
	} else {
		number = QString("%1").arg(int(integerFraction));
	}

	if (p.exponentLetter != QChar::Null) {
		const auto e = QString("%1").arg(exponent, p.exponentDigits + (p.exponentSign == '-'), 10, QLatin1Char('0'));
		QString sign = "";
		if (exponent >= 0 && !p.exponentSign.isNull())
			sign = "+";
		number += p.exponentLetter + sign + e;
	}

	if (p.integerSign == '+')
		number.prepend('+');

	return number;
}

bool NumberSpinBox::step(int steps) {
	int cursorPos = lineEdit()->cursorPosition();
	int textLenght = lineEdit()->text().length();

	if (cursorPos == 0)
		return true;

	QString v_str = lineEdit()->text().trimmed();

	NumberProperties p;
	if (!properties(v_str, p))
		return false;

	const auto comma = p.fractionPos;
	const auto exponentialIndex = p.exponentPos;

	// cursor behind the integer sign
	// cursor behind the comma
	// cursor behind the exponent letter
	// cursor behind the exponent sign
	if ((cursorPos == 1 && !p.integerSign.isNull()) || (comma >= 0 && cursorPos - 1 == comma)
		|| (exponentialIndex > 0 && (cursorPos - 1 == exponentialIndex || (cursorPos - 1 == exponentialIndex + 1 && !p.exponentSign.isNull()))))
		return true;

	bool before_comma = comma >= 0 && cursorPos - 1 < comma;
	bool before_exponent = exponentialIndex >= 0 && cursorPos - 1 < exponentialIndex;

	const auto& l = v_str.split('e', 0, Qt::CaseInsensitive);

	double integerFraction = l.at(0).toDouble();
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
			initial = textLenght;
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
		increase = textLenght - cursorPos;
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

	QString number = createStringNumber(integerFraction, exponent, p);

	lineEdit()->setText(number);

	// Set cursor position
	auto newPos = number.length() - (textLenght - cursorPos);
	if ((newPos == 0 && number.length() > 0))
		newPos = 1;
	if (newPos == 1 && !p.integerSign.isNull() && number.length() > 1)
		newPos = 2;
	lineEdit()->setCursorPosition(newPos);

	emit valueChanged(value());
	return true;
}

QAbstractSpinBox::StepEnabled NumberSpinBox::stepEnabled() const {
	return QAbstractSpinBox::StepEnabledFlag::StepUpEnabled | QAbstractSpinBox::StepEnabledFlag::StepDownEnabled; // for testing
}

void NumberSpinBox::setInvalid(bool invalid, const QString& tooltip) {
	mInvalid = invalid;
	if (invalid) {
		SET_WARNING_PALETTE

		setToolTip(tooltip);
	} else {
		setPalette(qApp->palette());
		setToolTip("");
	}
}

double NumberSpinBox::value(bool* ok) const {
	const auto t = lineEdit()->text();
	const double v = t.toDouble(ok);
	return v;
}
