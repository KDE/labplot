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

#ifndef VARIABLEPARSER_H
#define VARIABLEPARSER_H

#include <QtCore>

class VariableParser {
	public:
		VariableParser(const QString& name, const QString& value);
		int valuesCount();
		QVector< double > values();
		bool isParsed();
		
	private:
		QString m_backendName;
		QString m_string;
		QVector< double > m_values;
		bool m_parsed = false;
		
		void init();
		void parseMaximaValues();
		void parsePythonValues();
		void parseRValues();
};

#endif // VARIABLEPARSER_H
