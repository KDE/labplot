/*
	File                 : XYEquationCurvePrivate.h
	Project              : LabPlot
	Description          : Private members of XYEquationCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYEQUATIONCURVE2PRIVATE_H
#define XYEQUATIONCURVE2PRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "XYEquationCurve2.h"

class XYEquationCurve2;
class XYCurve;

class XYEquationCurve2Private : public XYAnalysisCurvePrivate {
public:
	explicit XYEquationCurve2Private(XYEquationCurve2*);
	// no need to delete xColumn and yColumn, they are deleted
	// when the parent aspect is removed
	~XYEquationCurve2Private() override = default;

	QString equation() const;
	const QVector<XYEquationCurve2::EquationData>& equationData() const;
	void setEquationVariableCurvesPath(int index, const QString& path);
	void setEquationVariableCurve(int index, XYCurve*);
	void setEquationVariableCurve(XYCurve *);
	void setEquation(const QString& equation, const QVector<XYEquationCurve2::EquationData>& equationData);
	void setEquation(const QString& equation, const QStringList& variableNames, const QStringList& variableCurvePaths);
	void connectEquationCurve(const XYCurve*);
	void equationVariableCurveRemoved(const AbstractAspect*);
	void equationVariableCurveAdded(const AbstractAspect*);

	void resetResults() override; // Clear the results of the previous calculation
	bool recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) override;

	//Column* xColumn;
	Column* resultColumn;
	//QVector<double>* xVector;
	QVector<double>* resultVector;

	XYEquationCurve2* const q;
	XYEquationCurve2::Result m_result;

private:
	QString m_equation;
	QVector<XYEquationCurve2::EquationData> m_equationData;
	QVector<QMetaObject::Connection> m_connectionsUpdateEquation;
};

#endif
