/***************************************************************************
    File                 : XYFourierFilterCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYFourierFilterCurve
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

#ifndef XYFOURIERFILTERCURVEPRIVATE_H
#define XYFOURIERFILTERCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"

class XYFourierFilterCurve;
class Column;

//#include <gsl/gsl_multifit_nlin.h>

class XYFourierFilterCurvePrivate: public XYCurvePrivate {
	public:
		explicit XYFourierFilterCurvePrivate(XYFourierFilterCurve*);
		~XYFourierFilterCurvePrivate();

//		void recalculate();

		const AbstractColumn* xDataColumn; //<! column storing the values for the x-data to be fitted
		const AbstractColumn* yDataColumn; //<! column storing the values for the y-data to be fitted
		const AbstractColumn* weightsColumn; //<! column storing the values for the weights to be used in the fit
		QString xDataColumnPath;
		QString yDataColumnPath;
		QString weightsColumnPath;

//		XYFitCurve::FitData fitData;
//		XYFitCurve::FitResult fitResult;
//		QStringList solverOutput;

		Column* xColumn; //<! column used internally for storing the x-values of the result fit curve
		Column* yColumn; //<! column used internally for storing the y-values of the result fit curve
		Column* residualsColumn;
		QVector<double>* xVector;
		QVector<double>* yVector;
		QVector<double>* residualsVector;

		bool sourceDataChangedSinceLastFilter; //<! \c true if the data in the source columns (x, y, or weights) was changed, \c false otherwise

		XYFourierFilterCurve* const q;

	private:
//		void writeSolverState(gsl_multifit_fdfsolver* s);
};

#endif
