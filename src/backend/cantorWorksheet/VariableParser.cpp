/*
    File                 : VariableParser.h
    Project              : LabPlot
    Description          : Variable parser for different CAS backends
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
    SPDX-FileCopyrightText: 2016 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "VariableParser.h"
#include <QStringList>
#include "backend/lib/trace.h"
#include <cmath>	// NAN

VariableParser::VariableParser(QString name, QString value)
	: m_backendName(std::move(name)), m_string(std::move(value)) {

	PERFTRACE("parsing variable");
	if (m_backendName.compare(QStringLiteral("Maxima"), Qt::CaseInsensitive) == 0)
		parseMaximaValues();
	else if ( (m_backendName.compare(QStringLiteral("Python"), Qt::CaseInsensitive) == 0)
			|| (m_backendName.compare(QStringLiteral("Python 3"), Qt::CaseInsensitive) == 0)
			|| (m_backendName.compare(QStringLiteral("Python 2"), Qt::CaseInsensitive) == 0) )
		parsePythonValues();
	else if (m_backendName.compare(QStringLiteral("Sage"), Qt::CaseInsensitive) == 0)
		parsePythonValues();
	else if (m_backendName.compare(QStringLiteral("R"), Qt::CaseInsensitive) == 0)
		parseRValues();
	else if (m_backendName.compare(QStringLiteral("Julia"), Qt::CaseInsensitive) == 0)
		parsePythonValues();
	else if (m_backendName.compare(QStringLiteral("Octave"), Qt::CaseInsensitive) == 0)
		parseOctaveValues();
}

void VariableParser::parseMaximaValues() {
	if (m_string.count(QStringLiteral("[")) < 2) {
		m_string = m_string.replace(QStringLiteral("["), QString());
		m_string = m_string.replace(QStringLiteral("]"), QString());
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
		m_string = m_string.replace(QStringLiteral("array(["), QString());
		m_string = m_string.replace(QStringLiteral("])"), QString());
	} else if (m_string.startsWith(QStringLiteral("["))) {
		//parse python's lists
		m_string = m_string.replace(QStringLiteral("["), QString());
		m_string = m_string.replace(QStringLiteral("]"), QString());
	} else if (m_string.startsWith(QStringLiteral("("))) {
		//parse python's tuples
		m_string = m_string.replace(QStringLiteral("("), QString());
		m_string = m_string.replace(QStringLiteral(")"), QString());
	} else {
		return;
	}

	if (m_string.count(QStringLiteral(","))>1)
		valueStringList = m_string.split(QStringLiteral(","));
	else
		valueStringList = m_string.split(QStringLiteral(" "));

	parseValues(valueStringList);
}

void VariableParser::parseRValues() {
	m_string = m_string.trimmed();
	const QStringList valueStringList = m_string.split(QStringLiteral(", "));
	parseValues(valueStringList);
}

void VariableParser::parseOctaveValues() {
	m_string = m_string.trimmed();

	QStringList valueStringList;
	const QStringList tempStringList = m_string.split(QLatin1Char('\n'));
	for (const QString& values : tempStringList) {
		//TODO: in newer version of Cantor the rows with "Columns..." were removed already.
		//we can stop looking for this substring in some point in time later.
		if (!values.isEmpty() && !values.trimmed().startsWith(QStringLiteral("Columns")))
			valueStringList << values.split(QLatin1Char(' '));
	}

	valueStringList.removeAll(QString());
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
	for (const QString& v : values) {
		bool isNumber = false;
		double value = v.trimmed().toDouble(&isNumber);

		//accept the variable only if there is at least one numerical value in the array.
		if (isNumber) {
			if (!m_parsed)
				m_parsed = true;
		} else {
			value = NAN;
		}

		m_values << value;
	}
}
