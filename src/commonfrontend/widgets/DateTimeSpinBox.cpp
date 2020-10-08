/***************************************************************************
	File                 : DateTimeSpinBox.cpp
	Project              : LabPlot
	Description          : widget for setting datetimes with a spinbox
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "DateTimeSpinBox.h"

#include <QLineEdit>
#include <QKeyEvent>
#include <QRegularExpressionValidator>

DateTimeSpinBox::DateTimeSpinBox(QWidget* parent) : QAbstractSpinBox(parent) {
	lineEdit()->setText("0000.00.00 00:00:00.001");
	DateTimeSpinBox::stepEnabled();

	m_regularExpressionValidator = new QRegularExpressionValidator();

	QRegularExpression regExp(R"(([0-9]+)\.(0[0-9]|1[0-2]|[0-9])\.(0[0-9]|[0-2][0-9]|30|[0-9]) ([0-1][0-9]|2[0-3]|[0-9])\:([0-5][0-9]|[0-9])\:([0-5][0-9]|[0-9])\.[0-9]{0,3})");
	m_regularExpressionValidator->setRegularExpression(regExp);

	lineEdit()->setValidator(m_regularExpressionValidator);
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
	lineEdit()->setText(QString::number(m_year) + '.' +
						QString("%1").arg(m_month, 2, 10, QLatin1Char('0')) + QLatin1Char('.') +
						QString("%1").arg(m_day, 2, 10, QLatin1Char('0')) + QLatin1Char(' ') +
						QString("%1").arg(m_hour, 2, 10, QLatin1Char('0')) + QLatin1Char(':') +
						QString("%1").arg(m_minute, 2, 10, QLatin1Char('0')) + QLatin1Char(':') +
						QString("%1").arg(m_second, 2, 10, QLatin1Char('0')) + QLatin1Char('.') +
						QString("%1").arg(m_millisecond, 3, 10, QLatin1Char('0')));
	emit valueChanged();
}

void DateTimeSpinBox::setValue(qint64 increment) {
	qint64 divisor = qint64(12) * 30 * 24 * 60 * 60 * 1000;
	qint64 rest;
	m_year = increment / divisor;
	rest = increment - m_year * divisor;
	divisor = qint64(30) * 24 * 60 * 60 * 1000;
	m_month = rest / divisor;
	rest = rest - m_month * divisor;
	divisor = qint64(24) * 60 * 60 * 1000;
	m_day = rest / divisor;
	rest = rest - m_day * divisor;
	divisor = qint64(60) * 60 * 1000;
	m_hour = rest / divisor;
	rest -= m_hour * divisor;
	divisor = qint64(60)* 1000;
	m_minute = rest / divisor;
	rest -= m_minute * divisor;
	divisor = qint64(1000);
	m_second = rest /divisor;
	rest -= m_second * divisor;
	m_millisecond = rest;

	writeValue();
}

qint64 DateTimeSpinBox::value() {
	return m_millisecond
			+ 1000 * (m_second
			+ 60 * (m_minute
			+ 60 * (m_hour
			+ 24 * (m_day
			+ 30 * (m_month
			+ 12 * m_year)))));
}

/*!
 * Read value from lineEdit of the spinbox
 */
void DateTimeSpinBox::getValue() {
	QString text = lineEdit()->text();

	int counter = 0;
	int startIndex = 0;
	for (int i=0; i< text.length(); i++) {
		if (text[i] == '.' || text[i] == ':' || text[i] == ' ' || i == text.length()-1)	{
			switch(counter) {
				case Type::year:
					m_year = text.midRef(startIndex, i - startIndex).toInt();
					break;
				case Type::month:
					m_month = text.midRef(startIndex, i - startIndex).toInt();
					break;
				case Type::day:
					m_day = text.midRef(startIndex, i - startIndex).toInt();
					break;
				case Type::hour:
					m_hour = text.midRef(startIndex, i - startIndex).toInt();
					break;
				case Type::minute:
					m_minute = text.midRef(startIndex, i - startIndex).toInt();
					break;
				case Type::second:
					m_second = text.midRef(startIndex, i - startIndex).toInt();
					break;
				case Type::millisecond:
					m_millisecond = text.midRef(startIndex, i - startIndex + 1).toInt(); // because of the condition (i == text.length()-1)
					break;
			}
			startIndex = i+1;
			counter ++;
		}
	}

	emit valueChanged();
}

void DateTimeSpinBox::setCursorPosition(Type type) {
	QString text = lineEdit()->text();
	int counter = 0;
	for (int i = 0; i < text.length(); i++) {
		if (text[i] == '.' || text[i] == ':' || text[i] == ' ')
			counter ++;

		if (counter-1 == type) {
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
		if (m_year + step < 0 && step < 0) {
			if (m_year + step < 0) {
				m_year = 0;
				return false;
			}
		}
		m_year += step;
		return true;
	}
		break;
	case Type::month:
		return changeValue(m_month, Type::year, step);
		break;
	case Type::day:
		return changeValue(m_day, Type::month, step);
		break;
	case Type::hour:
		return changeValue(m_hour, Type::day, step);
		break;
	case Type::minute:
		return changeValue(m_minute, Type::hour, step);
		break;
	case Type::second:
		return changeValue(m_second, Type::minute, step);
		break;
	case Type::millisecond:
		return changeValue(m_millisecond, Type::second, step);
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
		nextTypeCounter --;
		if (increaseValue(nextTypeType, nextTypeCounter)) {
			step += maxValue;
			thisType += step;
			return true;
		} else {
			thisType = 0;
			return false;
		}
	} else if ( thisType + step > maxValue-1 && step > 0) {
		step -= nextTypeCounter * maxValue;
		if (thisType + step > maxValue-1) {
			nextTypeCounter ++;
			step -= maxValue;
			thisType += step;
		} else
			thisType += step;


		return increaseValue(nextTypeType, nextTypeCounter);
	}
	thisType += step;
	return true;
}

DateTimeSpinBox::Type DateTimeSpinBox::determineType(int cursorPos) const{
	QString text = lineEdit()->text();

	if (cursorPos > text.length())
		cursorPos = text.length();

	int counter = 0;
	for (int i = 0; i < cursorPos; i++) {
		if (text[i] == '.' || text[i] == ':' || text[i] == ' ')
			counter ++;
	}

	if (counter <= Type::millisecond)
		return static_cast<Type>(counter);

	return Type::millisecond;
}
