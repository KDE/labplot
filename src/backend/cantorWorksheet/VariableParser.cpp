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
	if (m_string.count(QStringLiteral("[")) > 2)
		return;

	Datatype dataType = Datatype::float64;
	if (m_string.startsWith(QLatin1String("[\""))) //Maxime uses " to quote string values in the output
		dataType = Datatype::text;

	m_string = m_string.replace(QStringLiteral("["), QString());
	m_string = m_string.replace(QStringLiteral("]"), QString());
	m_string = m_string.trimmed();

	const QStringList valueStringList = m_string.split(QStringLiteral(","));
	parseValues(valueStringList, dataType);
}

/*!
 * Python containers that can be parsed:
 * * List (a collection which is ordered and changeable, allows duplicate members)
 * * Tuple (collection which is ordered and unchangeable, allows duplicate members)
 * * Set (collection which is unordered, unchangeable and unindexed, no duplicate members)
 * * Numpy's array (with and without the explicit specification of the data type)
 * */
void VariableParser::parsePythonValues() {
	QStringList valueStringList;
	QString dataType = "float64";
	m_string = m_string.trimmed();
	if (m_string.startsWith(QStringLiteral("array"))) {
		//parse numpy arrays, string representation like array([1,2,3,4,5]) or
		// array([1, 2,3], dtype=uint32)

		//we don't handle array of arrays, e.g. the output of 'np.ones((2,2), dtype=np.int16)'
		//which is 'array([[1, 1], [1, 1]], dtype=int16)'
		if (m_string.count(QStringLiteral("[")) > 2)
			return;

		QRegularExpressionMatch match;
		auto numpyDatatypeRegex = QStringLiteral("\\s*,\\s*dtype='{0,1}[a-zA-Z0-9\\[\\]]*'{0,1}");
		m_string.indexOf(QRegularExpression(numpyDatatypeRegex), 0, &match);
		if (match.isValid() && match.captured() != "")
			dataType = match.captured().replace("'", "").replace(", dtype=", "");
		m_string = m_string.replace(QStringLiteral("array(["), QString());
		m_string = m_string.replace(QRegExp(numpyDatatypeRegex), QString());
		m_string = m_string.replace(QStringLiteral("])"), QString());
	} else if (m_string.startsWith(QStringLiteral("["))) {
		//parse python's lists
		if (m_string.startsWith(QLatin1String("['"))) //python uses ' to quote string values in the output
			dataType = "text";
		m_string = m_string.replace(QStringLiteral("["), QString());
		m_string = m_string.replace(QStringLiteral("]"), QString());
	} else if (m_string.startsWith(QStringLiteral("("))) {
		//parse python's tuples
		if (m_string.startsWith(QLatin1String("('")))
			dataType = "text";
		m_string = m_string.replace(QStringLiteral("("), QString());
		m_string = m_string.replace(QStringLiteral(")"), QString());
	} else if (m_string.startsWith(QStringLiteral("{"))) {
		//parse python's sets
		if (m_string.startsWith(QLatin1String("{'")))
			dataType = "text";
		m_string = m_string.replace(QStringLiteral("{"), QString());
		m_string = m_string.replace(QStringLiteral("}"), QString());
	} else {
		return;
	}

	// Fast method to determine the separator. It is assumed if at least one
	// commas exist, the comma is the separator
	if (m_string.indexOf(",") != -1)
		valueStringList = m_string.split(QStringLiteral(","));
	else
		valueStringList = m_string.split(QStringLiteral(" "));

	parseValues(valueStringList, convertNumpyDatatype(dataType));
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
	switch(m_dataType) {
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

void VariableParser::parseValues(const QStringList& values, VariableParser::Datatype dataType) {
	PERFTRACE("parsing variable values string list");
	switch(dataType) {
		case Datatype::uint8:
		case Datatype::int8:
		case Datatype::uint16:
		case Datatype::int16:
		case Datatype::int32:
			m_values = new QVector<int>(values.size());
			m_dataType = AbstractColumn::ColumnMode::Integer;
			break;
		case Datatype::uint32:
		case Datatype::int64:
			m_values = new QVector<qint64>(values.size());
			m_dataType = AbstractColumn::ColumnMode::BigInt;
			break;
		case Datatype::uint64: // larger than qint64!
		case Datatype::float32:
		case Datatype::float64:
			m_values = new QVector<double>(values.size());
			m_dataType = AbstractColumn::ColumnMode::Double;
			break;
		case Datatype::datetime64_D:
		case Datatype::datetime64_h:
		case Datatype::datetime64_m:
		case Datatype::datetime64_s:
		case Datatype::datetime64_ms:
			m_values = new QVector<QDateTime>(values.size());
			m_dataType = AbstractColumn::ColumnMode::DateTime;
			break;
		case Datatype::text:
			m_values = new QVector<QString>(values.size());
			m_dataType = AbstractColumn::ColumnMode::Text;
	}

	int i = 0;
	bool isNumber = false;
	switch(dataType) {
	case Datatype::uint8:
	case Datatype::int8:
	case Datatype::uint16:
	case Datatype::int16:
	case Datatype::int32: {
		for (const auto& v : values) {
			int value = v.trimmed().toUInt(&isNumber);

			//accept the variable only if there is at least one numerical value in the array.
			if (isNumber) {
				if (!m_parsed)
					m_parsed = true;
			} else
				value = 0;

			integers()[i] = value;
			i++;
		}
		break;
	}
	case Datatype::uint32:
	case Datatype::int64: {
		for (const auto& v : values) {
			qint64 value = v.trimmed().toLongLong(&isNumber);
			if (isNumber) {
				if (!m_parsed)
					m_parsed = true;
			} else
				value = 0;

			bigInt()[i] = value;
			i++;
		}
		break;
	}
	case Datatype::uint64:
	case Datatype::float32:
	case Datatype::float64: {
		for (const auto& v : values) {
			double value = v.trimmed().toDouble(&isNumber);
			if (isNumber) {
				if (!m_parsed)
					m_parsed = true;
			} else
				value = NAN;

			doublePrecision()[i] = value;
			i++;
		}
		break;
	}
	// Adding timezone indicator "Z" is necessary, because specific dates like
	// 2017-03-26T02:14:34.000 are not available in different timezones.
	// https://forum.qt.io/topic/133181/qdatetime-fromstring-returns-invalid-datetime
	case Datatype::datetime64_D:
		for (const auto& v : values) {
			dateTime()[i] = QDate::fromString(v.trimmed().replace("'", "") + "Z", Qt::ISODate).startOfDay(Qt::UTC);
			m_parsed = true;
			i++;
		}
		break;
	case Datatype::datetime64_h:
		for (const auto& v : values) {
			dateTime()[i] = QDateTime::fromString(v.trimmed().replace("'", "") + "Z", "yyyy-MM-ddThht"); // last t is important. It is the timezone
			m_parsed = true;
			i++;
		}
		break;
	case Datatype::datetime64_m:
		for (const auto& v : values) {
			dateTime()[i] = QDateTime::fromString(v.trimmed().replace("'", "") + "Z", "yyyy-MM-ddThh:mmt"); // last t is important. It is the timezone
			m_parsed = true;
			i++;
		}
		break;
	case Datatype::datetime64_s:
		for (const auto& v : values) {
			dateTime()[i] = QDateTime::fromString(v.trimmed().replace("'", "") + "Z", Qt::ISODate);
			m_parsed = true;
			i++;
		}
		break;
	case Datatype::datetime64_ms:
		for (const auto& v : values) {
			dateTime()[i] = QDateTime::fromString(v.trimmed().replace("'", "") + "Z", Qt::ISODateWithMs);
			m_parsed = true;
			i++;
		}
		break;
	case Datatype::text:
		for (const auto& v : values) {
			text()[i] = v;
			m_parsed = true;
			i++;
		}
		break;
	}
}

