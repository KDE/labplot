/***************************************************************************
    File                 : VariableParser.h
    Project              : LabPlot
    Description          : Variable parser for different CAS backends
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Garvit Khatri (garvitdelhi@gmail.com)
    Copyright            : (C) 2016 by Alexander Semke (alexander.semke@web.de)

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

#include "VariableParser.h"

#include <QStringList>
#include <QTime>
#include <QDebug>

VariableParser::VariableParser(const QString& name, const QString& value)
	: m_backendName(name), m_string(value), m_parsed(false) {

	qDebug() << "Variable string is: " << m_string;

	QTime t = QTime::currentTime();
	if(m_backendName.compare(QString("Maxima"), Qt::CaseInsensitive) == 0)
		parseMaximaValues();
	else if(m_backendName.compare(QString("Python 3"), Qt::CaseInsensitive) == 0)
		parsePythonValues();
	else if(m_backendName.compare(QString("Python 2"), Qt::CaseInsensitive) == 0)
		parsePythonValues();
	else if(m_backendName.compare(QString("Sage"), Qt::CaseInsensitive) == 0)
		parsePythonValues();
	else if(m_backendName.compare(QString("R"), Qt::CaseInsensitive) == 0)
		parseRValues();

	qDebug() << "Time taken to parse: " << t.elapsed();
}

void VariableParser::parseMaximaValues() {
	if(m_string.count(QString("[")) < 2) {
		m_string = m_string.replace(QString("["), QString(""));
		m_string = m_string.replace(QString("]"), QString(""));
		m_string = m_string.trimmed();

		const QStringList valueStringList = m_string.split(',');
		foreach(QString valueString, valueStringList) {
			valueString = valueString.trimmed();
			bool isNumber = false;
			double value = valueString.toDouble(&isNumber);
			if(!isNumber) value = NAN;
			m_values << value;
		}

		m_parsed = true;
	}
}

void VariableParser::parsePythonValues() {
	QStringList valueStringList;
	if(m_string.count(QString("[")) < 2 && m_string.count(QString("[")) > 0 && m_string.count(QString("(")) == 0) {
		m_string = m_string.replace(QString("["), QString(""));
		m_string = m_string.replace(QString("]"), QString(""));
		m_string = m_string.trimmed();

		if(m_string.count(QString(","))>1)
			valueStringList = m_string.split(',');
		else
			valueStringList = m_string.split(' ');

		foreach(QString valueString, valueStringList) {
			valueString = valueString.trimmed();
			bool isNumber = false;
			double value = valueString.toDouble(&isNumber);
			if(!isNumber) value = NAN;
			m_values << value;
		}

		m_parsed = true;
	}
	else if(m_string.count(QString("(")) < 2 && m_string.count(QString("[")) == 0) {
		m_string = m_string.replace(QString("("), QString(""));
		m_string = m_string.replace(QString(")"), QString(""));
		m_string = m_string.trimmed();

		if(m_string.count(QString(","))>1)
			valueStringList = m_string.split(',');
		else
			valueStringList = m_string.split(' ');

		foreach(QString valueString, valueStringList) {
			valueString = valueString.trimmed();
			bool isNumber = false;
			double value = valueString.toDouble(&isNumber);
			if(!isNumber) value = NAN;
			m_values << value;
		}

		m_parsed = true;
	}
}

void VariableParser::parseRValues() {
	m_string = "[1] 1 2 3 4 5 6";
	m_string = m_string.remove( QRegExp("\\[.*\\]"));
	m_string.trimmed();

	const QStringList valueStringList = m_string.split(' ');
	foreach(QString valueString, valueStringList) {
		valueString = valueString.trimmed();
		bool isNumber = false;
		double value = valueString.toDouble(&isNumber);
		if(!isNumber) value = NAN;
		m_values << value;
	}

	m_parsed = true;
}

bool VariableParser::isParsed() {
	return m_parsed;
}

QVector< double > VariableParser::values() {
	return m_values;
}

int VariableParser::valuesCount() {
	return m_values.count();
}