/***************************************************************************
    File                 : XYFourierTransformCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYFourierTransformCurve
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"

class XYFourierTransformCurve;
class Column;

class XYFourierTransformCurvePrivate: public XYCurvePrivate {
	public:
		explicit XYFourierTransformCurvePrivate(XYFourierTransformCurve*);
		~XYFourierTransformCurvePrivate() override;
		void recalculate();

		const AbstractColumn* xDataColumn; //<! column storing the values for the x-data to be fitted
		const AbstractColumn* yDataColumn; //<! column storing the values for the y-data to be fitted
		QString xDataColumnPath;
		QString yDataColumnPath;

		XYFourierTransformCurve::TransformData transformData;
		XYFourierTransformCurve::TransformResult transformResult;

		Column* xColumn; //<! column used internally for storing the x-values of the result fit curve
		Column* yColumn; //<! column used internally for storing the y-values of the result fit curve
		QVector<double>* xVector;
		QVector<double>* yVector;

		XYFourierTransformCurve* const q;

//	private:
//		void writeSolverState(gsl_multifit_fdfsolver* s);
};

#endif
