/*
    File             : ExpressionParser.h
    Project          : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : c++ wrapper for the bison generated parser.
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "backend/lib/Range.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include <QVector>

class QStringList;

class ExpressionParser {

public:
	static ExpressionParser* getInstance();
	static int functionArgumentCount(const QString& functionName);
	static QString functionArgumentString(const QString& functionName, const XYEquationCurve::EquationType);

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

	static ExpressionParser* m_instance;

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
