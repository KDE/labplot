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
#include <QRegularExpressionMatch>
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

QVector<int>& VariableParser::integers() {
	return *static_cast<QVector<int>*>(m_values);
}

QVector<qint64>& VariableParser::bigInt() {
	return *static_cast<QVector<qint64>*>(m_values);
}

QVector<double>& VariableParser::doublePrecision() {
	return *static_cast<QVector<double>*>(m_values);
}

QVector<QDateTime>& VariableParser::dateTime() {
	return *static_cast<QVector<QDateTime>*>(m_values);
}

QVector<QString>& VariableParser::text() {
	return *static_cast<QVector<QString>*>(m_values);
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
	QString datatype = "text";
	m_string = m_string.trimmed();
	if (m_string.startsWith(QStringLiteral("array"))) {
		//parse numpy arrays, string representation like array([1,2,3,4,5]) or
		// array([1, 2,3], dtype=uint32)
		QRegularExpressionMatch match;
		auto numpyDatatypeRegex = QStringLiteral("\\s*,\\s*dtype='{0,1}[a-zA-Z0-9\\[\\]]*'{0,1}");
		m_string.indexOf(QRegularExpression(numpyDatatypeRegex), 0, &match);
		if (match.isValid() && match.captured() != "")
			datatype = match.captured().replace("'", "").replace(", dtype=", "");
		else
			datatype = "float64";
		m_string = m_string.replace(QStringLiteral("array(["), QString());
		m_string = m_string.replace(QRegExp(numpyDatatypeRegex), QString());
		m_string = m_string.replace(QStringLiteral("])"), QString());
	} else if (m_string.startsWith(QStringLiteral("["))) {
		//parse python's lists
		m_string = m_string.replace(QStringLiteral("["), QString());
		m_string = m_string.replace(QStringLiteral("]"), QString());
		datatype = "float64";
	} else if (m_string.startsWith(QStringLiteral("("))) {
		//parse python's tuples
		m_string = m_string.replace(QStringLiteral("("), QString());
		m_string = m_string.replace(QStringLiteral(")"), QString());
		datatype = "float64";
	} else {
		return;
	}

	// Fast method to determine the separator. It is assumed if at least one
	// commas exist, the comma is the separator
	if (m_string.indexOf(",") != -1)
		valueStringList = m_string.split(QStringLiteral(","));
	else
		valueStringList = m_string.split(QStringLiteral(" "));

	parsePythonValues(valueStringList, convertNumpyDatatype(datatype));
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

void VariableParser::clearValues() {
	switch(m_datatype) {
		case AbstractColumn::ColumnMode::Integer:
			delete static_cast<QVector<int>*>(m_values);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			delete static_cast<QVector<qlonglong>*>(m_values);
			break;
		case AbstractColumn::ColumnMode::Double:
			delete static_cast<QVector<qreal>*>(m_values);
			break;
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::DateTime:
			delete static_cast<QVector<QDateTime>*>(m_values);
			break;
		case AbstractColumn::ColumnMode::Text:
			delete static_cast<QVector<QString>*>(m_values);
			break;
	}
}

VariableParser::Datatype VariableParser::convertNumpyDatatype(const QString& d) {
	if (d == "uint8")
		return Datatype::uint8;
	else if (d == "int8")
		return Datatype::int8;
	else if (d == "uint16")
		return Datatype::uint16;
	else if (d == "int16")
		return Datatype::int16;
	else if (d == "uint32")
		return Datatype::uint32;
	else if (d == "int32")
		return Datatype::int32;
	else if (d == "uint64")
		return Datatype::uint64;
	else if (d == "int64")
		return Datatype::int64;
	else if (d == "float32")
		return Datatype::float32;
	else if (d == "float64")
		return Datatype::float64;
	else if (d == "datetime64[ms]")
		return Datatype::datetime64_ms;
	else if (d == "datetime64[s]")
		return Datatype::datetime64_s;
	else if (d == "datetime64[m]")
		return Datatype::datetime64_m;
	else if (d == "datetime64[h]")
		return Datatype::datetime64_h;
	else if (d == "datetime64[D]" || d == "datetime64")
		return Datatype::datetime64_D;

	// as default text is used
	return Datatype::text;
}

void VariableParser::parsePythonValues(QStringList& values, VariableParser::Datatype datatype) {
	PERFTRACE("parsing variable values string list");
	switch(datatype) {
		case Datatype::uint8:
		case Datatype::int8:
		case Datatype::uint16:
		case Datatype::int16:
		case Datatype::int32:
			m_values = new QVector<int>(values.size());
			m_datatype = AbstractColumn::ColumnMode::Integer;
			break;
		case Datatype::uint32:
		case Datatype::int64:
			m_values = new QVector<qint64>(values.size());
			m_datatype = AbstractColumn::ColumnMode::BigInt;
			break;
		case Datatype::uint64: // larger than qint64!
		case Datatype::float32:
		case Datatype::float64:
			m_values = new QVector<double>(values.size());
			m_datatype = AbstractColumn::ColumnMode::Double;
			break;
		case Datatype::datetime64_D:
		case Datatype::datetime64_h:
		case Datatype::datetime64_m:
		case Datatype::datetime64_s:
		case Datatype::datetime64_ms:
			m_values = new QVector<QDateTime>(values.size());
			m_datatype = AbstractColumn::ColumnMode::DateTime;
			break;
		case Datatype::text:
			m_values = new QVector<QString>(values.size());
			m_datatype = AbstractColumn::ColumnMode::Text;
	}

	int i = 0;
	for (QString& v : values) {
		switch(datatype) {
			case Datatype::uint8:
			case Datatype::int8:
			case Datatype::uint16:
			case Datatype::int16:
			case Datatype::int32: {
				bool isNumber = false;
				int value = v.trimmed().toUInt(&isNumber);

				//accept the variable only if there is at least one numerical value in the array.
				if (isNumber) {
					if (!m_parsed)
						m_parsed = true;
				} else
					value = 0;

				integers()[i] = value;
				break;
			}
			case Datatype::uint32:
			case Datatype::int64: {
				bool isNumber = false;
				qint64 value = v.trimmed().toLongLong(&isNumber);

				//accept the variable only if there is at least one numerical value in the array.
				if (isNumber) {
					if (!m_parsed)
						m_parsed = true;
				} else
					value = 0;

				bigInt()[i] = value;
				break;
			}
			case Datatype::uint64:
			case Datatype::float32:
			case Datatype::float64: {
				bool isNumber = false;
				double value = v.trimmed().toDouble(&isNumber);

				//accept the variable only if there is at least one numerical value in the array.
				if (isNumber) {
					if (!m_parsed)
						m_parsed = true;
				} else
					value = NAN;

				doublePrecision()[i] = value;
				break;
			}
			case Datatype::datetime64_D:
				dateTime()[i] = QDateTime::fromString(v.trimmed().replace("'", ""), "yyyy-MM-dd");
				m_parsed = true;
				break;
			case Datatype::datetime64_h:
				dateTime()[i] = QDateTime::fromString(v.trimmed().replace("'", ""), "yyyy-MM-ddThh");
				m_parsed = true;
				break;
			case Datatype::datetime64_m:
				dateTime()[i] = QDateTime::fromString(v.trimmed().replace("'", ""), "yyyy-MM-ddThh:mm");
				m_parsed = true;
				break;
			case Datatype::datetime64_s:
				dateTime()[i] = QDateTime::fromString(v.trimmed().replace("'", ""), "yyyy-MM-ddThh:mm:ss");
				m_parsed = true;
				break;
			case Datatype::datetime64_ms:
				dateTime()[i] = QDateTime::fromString(v.trimmed().replace("'", ""), "yyyy-MM-ddThh:mm:ss.zzz");
				m_parsed = true;
				break;
			case Datatype::text:
				text()[i] = v;
				break;
		}
		i++;
	}
}

void VariableParser::parseValues(const QStringList& values) {
	PERFTRACE("parsing variable values string list");
	m_datatype = AbstractColumn::ColumnMode::Double;
	m_values = new QVector<double>();
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

		static_cast<QVector<double>*>(m_values)->append(value);
	}
}
