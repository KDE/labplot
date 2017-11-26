/***************************************************************************
    File                 : XYDataReductionCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYDataReductionCurve
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#ifndef XYDATAREDUCTIONCURVEPRIVATE_H
#define XYDATAREDUCTIONCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"

class XYDataReductionCurve;
class Column;

class XYDataReductionCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYDataReductionCurvePrivate(XYDataReductionCurve*);
	~XYDataReductionCurvePrivate() override;

	void recalculate();

	XYDataReductionCurve::DataReductionData dataReductionData;
	XYDataReductionCurve::DataReductionResult dataReductionResult;

	XYDataReductionCurve* const q;
};

#endif
