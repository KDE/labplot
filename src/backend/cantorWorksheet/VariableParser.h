/*
    File                 : VariableParser.h
    Project              : LabPlot
    Description          : Variable parser for different CAS backends
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
    SPDX-FileCopyrightText: 2016 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef VARIABLEPARSER_H
#define VARIABLEPARSER_H

class QString;
#include <QVector>
#include <QDateTime>

#include "backend/core/AbstractColumn.h"

class VariableParser {
public:
	VariableParser(QString name, QString value);
	~VariableParser() {clearValues(); m_parsed=false;}
	QVector<int>& integers();
	QVector<qint64>& bigInt();
	QVector<double>& doublePrecision();
	QVector<QDateTime>& dateTime();
	QVector<QString>& text();
	bool isParsed();

	enum class Datatype {
		uint8,
		int8,
		uint16,
		int16,
		uint32,
		int32,
		uint64,
		int64,
		float32,
		float64,
		datetime64_ms,
		datetime64_s,
		datetime64_m,
		datetime64_h, // hour is smallest unit
		datetime64_D, // day is smallest unit
		text,
	};

	AbstractColumn::ColumnMode dataType() {return m_dataType;}

private:
	QString m_backendName;
	QString m_string;
	void* m_values{nullptr};
	bool m_parsed{false};
	AbstractColumn::ColumnMode m_dataType{AbstractColumn::ColumnMode::Double};

	void parseMaximaValues();
	void parsePythonValues();
	void parseRValues();
	void parseOctaveValues();
	void parseValues(const QStringList&, Datatype type = Datatype::float64);
	Datatype convertNumpyDatatype(const QString& datatype);

	void clearValues();
};

#endif // VARIABLEPARSER_H
