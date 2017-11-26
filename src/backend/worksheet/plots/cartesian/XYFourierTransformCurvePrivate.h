/***************************************************************************
    File                 : XYFourierTransformCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYFourierTransformCurve
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

#ifndef XYFOURIERTRANSFORMCURVEPRIVATE_H
#define XYFOURIERTRANSFORMCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"

class XYFourierTransformCurve;
class Column;

class XYFourierTransformCurvePrivate: public XYAnalysisCurvePrivate {
public:
	explicit XYFourierTransformCurvePrivate(XYFourierTransformCurve*);
	~XYFourierTransformCurvePrivate() override;
	void recalculate();

	XYFourierTransformCurve::TransformData transformData;
	XYFourierTransformCurve::TransformResult transformResult;

	XYFourierTransformCurve* const q;
};

#endif
