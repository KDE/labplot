/***************************************************************************
    File                 : XYFitCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYFitCurve
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2017 Alexander Semke (alexander.semke@web.de)

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

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"

class XYFitCurve;
class Column;

extern "C" {
#include <gsl/gsl_multifit_nlin.h>
}

class XYFitCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYFitCurvePrivate(XYFitCurve*);
	~XYFitCurvePrivate() override;

	void recalculate();
	void evaluate(bool preview = false);

	const AbstractColumn* xErrorColumn{nullptr}; //<! column storing the values for the x-error to be used in the fit
	const AbstractColumn* yErrorColumn{nullptr}; //<! column storing the values for the y-error to be used in the fit
	QString xErrorColumnPath;
	QString yErrorColumnPath;

	XYFitCurve::FitData fitData;
	XYFitCurve::FitResult fitResult;
	QStringList solverOutput;

	Column* residualsColumn{nullptr};
	QVector<double>* residualsVector{nullptr};

	XYFitCurve* const q;

private:
	void prepareResultColumns();
	void writeSolverState(gsl_multifit_fdfsolver*, double chi2 = NAN);
};

#endif
