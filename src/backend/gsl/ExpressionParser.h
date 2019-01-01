/***************************************************************************
    File             : ExpressionParser.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Description      : c++ wrapper for the bison generated parser.

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

#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <QVector>

class QStringList;

class ExpressionParser {

public:
	static ExpressionParser* getInstance();

	bool isValid(const QString& expr, const QStringList& vars);
	QStringList getParameter(const QString& expr, const QStringList& vars);
	bool evaluateCartesian( const QString& expr, const QString& min, const QString& max,
					int count, QVector<double>* xVector, QVector<double>* yVector,
					const QStringList& paramNames, const QVector<double>& paramValues);
	bool evaluateCartesian(const QString& expr, const QString& min, const QString& max,
					int count, QVector<double>* xVector, QVector<double>* yVector);
	bool evaluateCartesian(const QString& expr, QVector<double>* xVector, QVector<double>* yVector);
	bool evaluateCartesian(const QString& expr, QVector<double>* xVector, QVector<double>* yVector,
					const QStringList& paramNames, const QVector<double>& paramValues);
	bool evaluateCartesian(const QString& expr, const QStringList& vars, const QVector<QVector<double>*>& xVectors, QVector<double>* yVector);
	bool evaluatePolar(const QString& expr, const QString& min, const QString& max,
					int count, QVector<double>* xVector, QVector<double>* yVector);
	bool evaluateParametric(const QString& expr1, const QString& expr2, const QString& min, const QString& max,
					int count, QVector<double>* xVector, QVector<double>* yVector);

	const QStringList& functions();
	const QStringList& functionsGroups();
	const QStringList& functionsNames();
	const QVector<int>& functionsGroupIndices();

	const QStringList& constants();
	const QStringList& constantsGroups();
	const QStringList& constantsNames();
	const QStringList& constantsValues();
	const QStringList& constantsUnits();
	const QVector<int>& constantsGroupIndices();

private:
	ExpressionParser();
	~ExpressionParser();

	void initFunctions();
	void initConstants();

	static ExpressionParser* instance;

	QStringList m_functions;
	QStringList m_functionsGroups;
	QStringList m_functionsNames;
	QVector<int> m_functionsGroupIndex;

	QStringList m_constants;
	QStringList m_constantsGroups;
	QStringList m_constantsNames;
	QStringList m_constantsValues;
	QStringList m_constantsUnits;
	QVector<int> m_constantsGroupIndex;
};
#endif
