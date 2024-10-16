/*
	File                 : XYAnalysisCurvePrivate.h
	Project              : LabPlot
	Description          : Private members of XYAnalysisCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYANALYSISCURVEPRIVATE_H
#define XYANALYSISCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"

class XYAnalysisCurve;
class Column;
class AbstractColumn;

class XYAnalysisCurvePrivate : public XYCurvePrivate {
public:
	explicit XYAnalysisCurvePrivate(XYAnalysisCurve*);
	~XYAnalysisCurvePrivate() override;

	XYAnalysisCurve::DataSourceType dataSourceType{XYAnalysisCurve::DataSourceType::Spreadsheet};
	const XYCurve* dataSourceCurve{nullptr};
	QString dataSourceCurvePath;

	void recalculate();
	virtual bool recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) = 0;
	virtual void prepareTmpDataColumn(const AbstractColumn** tmpXDataColumn, const AbstractColumn** tmpYDataColumn);
	virtual void resetResults() = 0; // Clear the results of the previous calculation
	virtual bool preparationValid(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn);
	virtual void connectCurve(const XYCurve* curve);
	void connectColumn(const AbstractColumn* column, Dimension dim, bool second);
	void updateConnections();
	void sourceChanged();

	const AbstractColumn* xDataColumn{nullptr}; //<! column storing the values for the input x-data for the analysis function
	const AbstractColumn* yDataColumn{nullptr}; //<! column storing the values for the input y-data for the analysis function
	const AbstractColumn* y2DataColumn{nullptr}; //<! column storing the values for the optional second input y-data
	QString xDataColumnPath;
	QString yDataColumnPath;
	QString y2DataColumnPath;

	Column* xColumn{
		nullptr}; //<! column used internally for storing the x-values of the result analysis curve. This column is also equal to the column of xycurve
	Column* yColumn{
		nullptr}; //<! column used internally for storing the y-values of the result analysis curve. This column is also equal to the column of xycurve
	QVector<double>* xVector{nullptr};
	QVector<double>* yVector{nullptr};

	QVector<QMetaObject::Connection> m_connections;

	XYAnalysisCurve* const q;
};

#endif
