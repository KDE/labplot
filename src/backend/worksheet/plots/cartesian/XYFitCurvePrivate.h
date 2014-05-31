/***************************************************************************
    File                 : XYFitCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYFitCurve
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke*web.de)

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

#ifndef XYEQUATIONCURVEPRIVATE_H
#define XYEQUATIONCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"

class XYFitCurve;
class Column;

class XYFitCurvePrivate: public XYCurvePrivate {
	public:
		explicit XYFitCurvePrivate(XYFitCurve*);
		~XYFitCurvePrivate();

		void recalculate();

		XYFitCurve::FitData fitData;
		Column* xColumn;
		Column* yColumn;
		QVector<double>* xVector;
		QVector<double>* yVector;

		XYFitCurve* const q;
};

#endif
