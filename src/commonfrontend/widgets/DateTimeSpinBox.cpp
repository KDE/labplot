/*
	File                 : DateTimeSpinBox.cpp
	Project              : LabPlot
	Description          : widget for setting datetimes with a spinbox
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DateTimeSpinBox.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QRegularExpressionValidator>

DateTimeSpinBox::DateTimeSpinBox(QWidget* parent)
	: QAbstractSpinBox(parent) {
	lineEdit()->setText(QStringLiteral("0000.00.00 00:00:00.001"));
	DateTimeSpinBox::stepEnabled();

	m_regularExpressionValidator = new QRegularExpressionValidator();

	QRegularExpression regExp(QStringLiteral(
		R"(([0-9]+)\.(0[0-9]|1[0-2]|[0-9])\.(0[0-9]|[0-2][0-9]|30|[0-9]) ([0-1][0-9]|2[0-3]|[0-9])\:([0-5][0-9]|[0-9])\:([0-5][0-9]|[0-9])\.[0-9]{0,3})"));
	m_regularExpressionValidator->setRegularExpression(regExp);

	lineEdit()->setValidator(m_regularExpressionValidator);
}

DateTimeSpinBox::~DateTimeSpinBox() {
	delete m_regularExpressionValidator;
}

void DateTimeSpinBox::keyPressEvent(QKeyEvent* event) {
	if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
		int cursorPos = lineEdit()->cursorPosition();
		int textLenght = lineEdit()->text().length();
		QAbstractSpinBox::keyPressEvent(event);
		getValue();
		if (lineEdit()->text().length() != textLenght)
			lineEdit()->setCursorPosition(cursorPos + 1);
		else
			lineEdit()->setCursorPosition(cursorPos);
	} else if (event->key() == Qt::Key_Up) {
		Type type = determineType(lineEdit()->cursorPosition());
		increaseValue(type, 1);
		writeValue();
		setCursorPosition(type);
	} else if (event->key() == Qt::Key_Down) {
		Type type = determineType(lineEdit()->cursorPosition());
		increaseValue(type, -1);
		writeValue();
		setCursorPosition(type);
	} else {
		QAbstractSpinBox::keyPressEvent(event);
		getValue();
	}
}

QAbstractSpinBox::StepEnabled DateTimeSpinBox::stepEnabled() const {
	return QAbstractSpinBox::StepEnabledFlag::StepUpEnabled | QAbstractSpinBox::StepEnabledFlag::StepDownEnabled; // for testing
}

void DateTimeSpinBox::stepBy(int steps) {
	Type type = determineType(lineEdit()->cursorPosition());
	increaseValue(type, steps);
	writeValue();
	setCursorPosition(type);
}

/*!
 * Write value to lineEdit of the spinbox
 */
void DateTimeSpinBox::writeValue() {
	lineEdit()->setText(QString::number(mDateTime.year) + QLatin1Char('.') + QStringLiteral("%1").arg(mDateTime.month, 2, 10, QLatin1Char('0'))
						+ QLatin1Char('.') + QStringLiteral("%1").arg(mDateTime.day, 2, 10, QLatin1Char('0')) + QLatin1Char(' ')
						+ QStringLiteral("%1").arg(mDateTime.hour, 2, 10, QLatin1Char('0')) + QLatin1Char(':')
						+ QStringLiteral("%1").arg(mDateTime.minute, 2, 10, QLatin1Char('0')) + QLatin1Char(':')
						+ QStringLiteral("%1").arg(mDateTime.second, 2, 10, QLatin1Char('0')) + QLatin1Char('.')
						+ QStringLiteral("%1").arg(mDateTime.millisecond, 3, 10, QLatin1Char('0')));
	Q_EMIT valueChanged();
}

void DateTimeSpinBox::setValue(qint64 increment) {
	mDateTime = DateTime::dateTime(increment);
	writeValue();
}

qint64 DateTimeSpinBox::value() {
	return DateTime::createValue(mDateTime.year, mDateTime.month, mDateTime.day, mDateTime.hour, mDateTime.minute, mDateTime.second, mDateTime.millisecond);
}

/*!
 * Read value from lineEdit of the spinbox
 */
void DateTimeSpinBox::getValue() {
	QString text = lineEdit()->text();

	int counter = 0;
	int startIndex = 0;
	for (int i = 0; i < text.length(); i++) {
		if (text[i] == QLatin1Char('.') || text[i] == QLatin1Char(':') || text[i] == QLatin1Char(' ') || i == text.length() - 1) {
			switch (counter) {
			case Type::year:
				mDateTime.year = text.mid(startIndex, i - startIndex).toInt();
				break;
			case Type::month:
				mDateTime.month = text.mid(startIndex, i - startIndex).toInt();
				break;
			case Type::day:
				mDateTime.day = text.mid(startIndex, i - startIndex).toInt();
				break;
			case Type::hour:
				mDateTime.hour = text.mid(startIndex, i - startIndex).toInt();
				break;
			case Type::minute:
				mDateTime.minute = text.mid(startIndex, i - startIndex).toInt();
				break;
			case Type::second:
				mDateTime.second = text.mid(startIndex, i - startIndex).toInt();
				break;
			case Type::millisecond:
				mDateTime.millisecond = text.mid(startIndex, i - startIndex + 1).toInt(); // because of the condition (i == text.length()-1)
				break;
			}
			startIndex = i + 1;
			counter++;
		}
	}

	Q_EMIT valueChanged();
}

void DateTimeSpinBox::setCursorPosition(Type type) {
	QString text = lineEdit()->text();
	int counter = 0;
	for (int i = 0; i < text.length(); i++) {
		if (text[i] == QLatin1Char('.') || text[i] == QLatin1Char(':') || text[i] == QLatin1Char(' '))
			counter++;

		if (counter - 1 == type) {
			lineEdit()->setCursorPosition(i);
			break;
		}
	}
}

bool DateTimeSpinBox::valid() {
	return true;
}

// step can also be negative
bool DateTimeSpinBox::increaseValue(DateTimeSpinBox::Type type, int step) {
	switch (type) {
	case Type::year: {
		if (mDateTime.year + step < 0 && step < 0) {
			mDateTime.year = 0;
			return false;
		}
		mDateTime.year += step;
		return true;
	} break;
	case Type::month:
		return changeValue(mDateTime.month, Type::year, step);
		break;
	case Type::day:
		return changeValue(mDateTime.day, Type::month, step);
		break;
	case Type::hour:
		return changeValue(mDateTime.hour, Type::day, step);
		break;
	case Type::minute:
		return changeValue(mDateTime.minute, Type::hour, step);
		break;
	case Type::second:
		return changeValue(mDateTime.second, Type::minute, step);
		break;
	case Type::millisecond:
		return changeValue(mDateTime.millisecond, Type::second, step);
		break;
	default:
		return false;
		break;
	}
}

bool DateTimeSpinBox::changeValue(qint64& thisType, DateTimeSpinBox::Type nextTypeType, int step) {
	int maxValue = 1;
	switch (nextTypeType) {
	case (Type::year):
		maxValue = 12;
		break;
	case (Type::month):
		maxValue = 30;
		break;
	case (Type::day):
		maxValue = 24;
		break;
	case (Type::hour):
		maxValue = 60;
		break;
	case (Type::minute):
		maxValue = 60;
		break;
	case (Type::second):
		maxValue = 1000;
		break;
	case (Type::millisecond):
		return false;
	}

	int nextTypeCounter = step / maxValue;
	step -= nextTypeCounter * maxValue;
	if (thisType + step < 0 && step < 0) {
		nextTypeCounter--;
		if (increaseValue(nextTypeType, nextTypeCounter)) {
			step += maxValue;
			thisType += step;
			return true;
		} else {
			thisType = 0;
			return false;
		}
	} else if (thisType + step > maxValue - 1 && step > 0) {
		step -= nextTypeCounter * maxValue;
		if (thisType + step > maxValue - 1) {
			nextTypeCounter++;
			step -= maxValue;
			thisType += step;
		} else
			thisType += step;

		return increaseValue(nextTypeType, nextTypeCounter);
	}
	thisType += step;
	return true;
}

DateTimeSpinBox::Type DateTimeSpinBox::determineType(int cursorPos) const {
	QString text = lineEdit()->text();

	if (cursorPos > text.length())
		cursorPos = text.length();

	int counter = 0;
	for (int i = 0; i < cursorPos; i++) {
		if (text[i] == QLatin1Char('.') || text[i] == QLatin1Char(':') || text[i] == QLatin1Char(' '))
			counter++;
	}

	if (counter <= Type::millisecond)
		return static_cast<Type>(counter);

	return Type::millisecond;
}
