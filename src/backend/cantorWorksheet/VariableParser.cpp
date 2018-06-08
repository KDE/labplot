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
#include "backend/lib/trace.h"
#include <cmath>

VariableParser::VariableParser(const QString& name, const QString& value)
	: m_backendName(name), m_string(value), m_parsed(false) {

	PERFTRACE("parsing variable");
	if(m_backendName.compare(QStringLiteral("Maxima"), Qt::CaseInsensitive) == 0)
		parseMaximaValues();
	else if(m_backendName.compare(QStringLiteral("Python 3"), Qt::CaseInsensitive) == 0)
		parsePythonValues();
	else if(m_backendName.compare(QStringLiteral("Python 2"), Qt::CaseInsensitive) == 0)
		parsePythonValues();
	else if(m_backendName.compare(QStringLiteral("Sage"), Qt::CaseInsensitive) == 0)
		parsePythonValues();
	else if(m_backendName.compare(QStringLiteral("R"), Qt::CaseInsensitive) == 0)
		parseRValues();
	else if(m_backendName.compare(QStringLiteral("Julia"), Qt::CaseInsensitive) == 0)
		parsePythonValues();
}

void VariableParser::parseMaximaValues() {
	if(m_string.count(QStringLiteral("[")) < 2) {
		m_string = m_string.replace(QStringLiteral("["), QStringLiteral(""));
		m_string = m_string.replace(QStringLiteral("]"), QStringLiteral(""));
		m_string = m_string.trimmed();

		const QStringList valueStringList = m_string.split(QStringLiteral(","));
		parseValues(valueStringList);
	}
}

void VariableParser::parsePythonValues() {
	QStringList valueStringList;
	m_string = m_string.trimmed();
	if (m_string.startsWith(QStringLiteral("array"))) {
		//parse numpy arrays, string representation like array([1,2,3,4,5])
		m_string = m_string.replace(QStringLiteral("array(["), QStringLiteral(""));
		m_string = m_string.replace(QStringLiteral("])"), QStringLiteral(""));
	} else if (m_string.startsWith(QStringLiteral("["))) {
		//parse python's lists
		m_string = m_string.replace(QStringLiteral("["), QStringLiteral(""));
		m_string = m_string.replace(QStringLiteral("]"), QStringLiteral(""));
	} else if(m_string.startsWith(QStringLiteral("("))) {
		//parse python's tuples
		m_string = m_string.replace(QStringLiteral("("), QStringLiteral(""));
		m_string = m_string.replace(QStringLiteral(")"), QStringLiteral(""));
	} else {
		return;
	}

	if(m_string.count(QStringLiteral(","))>1)
		valueStringList = m_string.split(QStringLiteral(","));
	else
		valueStringList = m_string.split(QStringLiteral(" "));

	parseValues(valueStringList);
}

void VariableParser::parseRValues() {
	m_string = m_string.remove( QRegExp("\\[.*\\]"));
	m_string = m_string.trimmed();

	const QStringList valueStringList = m_string.split(QStringLiteral(" "));
	parseValues(valueStringList);
}

bool VariableParser::isParsed() {
	return m_parsed;
}

QVector< double > VariableParser::values() {
	return m_values;
}

void VariableParser::parseValues(const QStringList& values) {
	PERFTRACE("parsing variable values string list");
	for(const QString& v : values) {
		bool isNumber = false;
		double value = v.trimmed().toDouble(&isNumber);

		//accept the variable only if there is at least one numerical value in the array.
		if(isNumber) {
			if (!m_parsed)
				m_parsed = true;
		} else {
			value = NAN;
		}

		m_values << value;
	}
}
