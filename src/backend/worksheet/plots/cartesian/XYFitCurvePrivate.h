/*
    File                 : XYFitCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYFitCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYFITCURVEPRIVATE_H
#define XYFITCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"

class XYFitCurve;
class Column;
class Histogram;

extern "C" {
#include <gsl/gsl_multifit_nlin.h>
}

class XYFitCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYFitCurvePrivate(XYFitCurve*);
	~XYFitCurvePrivate() override;

	void recalculate();
	void evaluate(bool preview = false);

	const Histogram* dataSourceHistogram{nullptr};
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
	void writeSolverState(gsl_multifit_fdfsolver*, double chi = qQNaN());
};

#endif
