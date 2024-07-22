/*
	File                 : XYFunctionCurvePrivate.h
	Project              : LabPlot
	Description          : Private members of XYFunctionCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYEFUNCTIONCURVEPRIVATE_H
#define XYFUNCTIONCURVEPRIVATE_H

#include "XYFunctionCurve.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"

class XYFunctionCurve;
class XYCurve;

class XYFunctionCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYFunctionCurvePrivate(XYFunctionCurve*);
	// no need to delete xColumn and yColumn, they are deleted
	// when the parent aspect is removed
	~XYFunctionCurvePrivate() override = default;

	QString function() const;
	const QVector<XYFunctionCurve::FunctionData>& functionData() const;
	void setFunctionVariableCurvesPath(int index, const QString& path);
	void setFunctionVariableCurve(int index, const XYCurve*);
	void setFunctionVariableCurve(const XYCurve*);
	void setFunction(const QString& function, const QVector<XYFunctionCurve::FunctionData>& functionData);
	void setFunction(const QString& function, const QStringList& variableNames, const QStringList& variableCurvePaths);
	void connectFunctionCurve(const XYCurve*);
	void functionVariableCurveRemoved(const AbstractAspect*);
	void functionVariableCurveAdded(const AbstractAspect*);
	bool preparationValid(const AbstractColumn*, const AbstractColumn*) override;
	void prepareTmpDataColumn(const AbstractColumn**, const AbstractColumn**) override;
	void handleAspectUpdated(const QString& aspectPath, const AbstractAspect* element);

	void resetResults() override; // Clear the results of the previous calculation
	bool recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) override;

	XYFunctionCurve* const q;
	XYFunctionCurve::Result m_result;

private:
	QString m_function;
	QVector<XYFunctionCurve::FunctionData> m_functionData;
	QVector<QMetaObject::Connection> m_connectionsUpdateFunction;

	friend class XYFunctionCurveTest;
};

#endif