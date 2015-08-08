/***************************************************************************
    File                 : CantorWorksheet.cpp
    Project              : LabPlot
    Description          : Aspect providing a Cantor Worksheets for Multiple backends
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Garvit Khatri (garvitdelhi@gmail.com)

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

VariableParser::VariableParser(const QString& name, const QString& value)
	: m_backendName(name), m_string(value){
	m_parsed = false;
	qDebug() << "String is: " << m_string;
	init();
}

void VariableParser::init() {
	if(m_backendName.compare(QString("Maxima"), Qt::CaseInsensitive) == 0) {
		return parseMaximaValues();
	}
	if(m_backendName.compare(QString("Python 3"), Qt::CaseInsensitive) == 0) {
		return parsePythonValues();
	}
}

void VariableParser::parseMaximaValues() {
	QTime t = QTime::currentTime();
	QStringList valueStringList;
	if(m_string.count(QString("[")) < 2) {
		m_string = m_string.replace(QString("["), QString(""));
		m_string = m_string.replace(QString("]"), QString(""));
		m_string = m_string.trimmed();
		valueStringList = m_string.split(',');
		foreach(QString valueString, valueStringList) {
			valueString = valueString.trimmed();
			bool isNumber = false;
			double value = valueString.toDouble(&isNumber);
			if(!isNumber) value = NAN;
			m_values << value;
		}
		m_parsed = true;
		qDebug() << "Time taken to parse: " << t.elapsed();
	}
}

void VariableParser::parsePythonValues() {
	QTime t = QTime::currentTime();
	QStringList valueStringList;
	qDebug() << "[ === " << m_string.count(QString("["));
	qDebug() << "( === " << m_string.count(QString("("));
	if(m_string.count(QString("[")) < 2 && m_string.count(QString("[")) > 0 && m_string.count(QString("(")) == 0) {
		m_string = m_string.replace(QString("["), QString(""));
		m_string = m_string.replace(QString("]"), QString(""));
		m_string = m_string.trimmed();
		valueStringList = m_string.split(',');
		foreach(QString valueString, valueStringList) {
			valueString = valueString.trimmed();
			bool isNumber = false;
			double value = valueString.toDouble(&isNumber);
			if(!isNumber) value = NAN;
			m_values << value;
		}
		m_parsed = true;
		qDebug() << "Time taken to parse: " << t.elapsed();
	}
	if(m_string.count(QString("(")) < 2 && m_string.count(QString("[")) == 0) {
		m_string = m_string.replace(QString("("), QString(""));
		m_string = m_string.replace(QString(")"), QString(""));
		m_string = m_string.trimmed();
		valueStringList = m_string.split(',');
		foreach(QString valueString, valueStringList) {
			valueString = valueString.trimmed();
			bool isNumber = false;
			double value = valueString.toDouble(&isNumber);
			if(!isNumber) value = NAN;
			m_values << value;
		}
		m_parsed = true;
		qDebug() << "Time taken to parse: " << t.elapsed();
	}
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