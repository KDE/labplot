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

class VariableParser {
public:
	VariableParser(QString name, QString value);
	QVector<double> values();
	bool isParsed();

private:
	QString m_backendName;
	QString m_string;
	QVector<double> m_values;
	bool m_parsed{false};

	void parseMaximaValues();
	void parsePythonValues();
	void parseRValues();
	void parseOctaveValues();
	void parseValues(const QStringList&);
};

#endif // VARIABLEPARSER_H
