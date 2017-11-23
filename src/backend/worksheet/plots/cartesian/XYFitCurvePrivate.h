/***************************************************************************
    File                 : XYFitCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYFitCurve
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)

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

#ifndef XYFITCURVEPRIVATE_H
#define XYFITCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"

class XYFitCurve;
class Column;

extern "C" {
#include <gsl/gsl_multifit_nlin.h>
}

class XYFitCurvePrivate: public XYCurvePrivate {
	public:
		explicit XYFitCurvePrivate(XYFitCurve*);
		~XYFitCurvePrivate() override;

		void recalculate();

		const AbstractColumn* xDataColumn; //<! column storing the values for the x-data to be fitted
		const AbstractColumn* yDataColumn; //<! column storing the values for the y-data to be fitted
		const AbstractColumn* xErrorColumn; //<! column storing the values for the x-error to be used in the fit
		const AbstractColumn* yErrorColumn; //<! column storing the values for the y-error to be used in the fit
		QString xDataColumnPath;
		QString yDataColumnPath;
		QString xErrorColumnPath;
		QString yErrorColumnPath;

		XYFitCurve::FitData fitData;
		XYFitCurve::FitResult fitResult;
		QStringList solverOutput;

		Column* xColumn; //<! column used internally for storing the x-values of the result fit curve
		Column* yColumn; //<! column used internally for storing the y-values of the result fit curve
		Column* residualsColumn;
		QVector<double>* xVector;
		QVector<double>* yVector;
		QVector<double>* residualsVector;

		XYFitCurve* const q;

	private:
		void writeSolverState(gsl_multifit_fdfsolver* s);
};

#endif
