/***************************************************************************
    File                 : XYAnalysisCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYAnalysisCurve
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)

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

#ifndef XYANALYSISCURVEPRIVATE_H
#define XYANALYSISCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"

class XYAnalysisCurve;
class Column;

class XYAnalysisCurvePrivate : public XYCurvePrivate {
public:
	explicit XYAnalysisCurvePrivate(XYAnalysisCurve*);
	~XYAnalysisCurvePrivate() override;

	XYAnalysisCurve::DataSourceType dataSourceType;
	const XYCurve* dataSourceCurve;

	const AbstractColumn* xDataColumn; //<! column storing the values for the input x-data for the analysis function
	const AbstractColumn* yDataColumn; //<! column storing the values for the input y-data for the analysis function
	QString xDataColumnPath;
	QString yDataColumnPath;

	Column* xColumn; //<! column used internally for storing the x-values of the result analysis curve
	Column* yColumn; //<! column used internally for storing the y-values of the result analysis curve
	QVector<double>* xVector;
	QVector<double>* yVector;

	XYAnalysisCurve* const q;
};

#endif
