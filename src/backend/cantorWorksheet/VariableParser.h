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

#ifndef VARIABLEPARSER_H
#define VARIABLEPARSER_H

#include <QString>
#include <QVector>

class VariableParser {
	public:
		VariableParser(const QString& name, const QString& value);
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
		void parseValues(const QStringList&);
};

#endif // VARIABLEPARSER_H
