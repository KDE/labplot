/*
	File                 : XYFitCurvePrivate.h
	Project              : LabPlot
	Description          : Private members of XYFitCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2021 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYFITCURVEPRIVATE_H
#define XYFITCURVEPRIVATE_H

#include "backend/note/Note.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"

class XYFitCurve;
class Column;
class Histogram;

#include <gsl/gsl_multifit_nlin.h>

class XYFitCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYFitCurvePrivate(XYFitCurve*);
	~XYFitCurvePrivate() override;

	virtual bool recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) override;
	virtual void prepareTmpDataColumn(const AbstractColumn** tmpXDataColumn, const AbstractColumn** tmpYDataColumn) const override;
	virtual void resetResults() override;
	void updateResultsNote();
	void runLevenbergMarquardt(const AbstractColumn* xcol, const AbstractColumn* ycol, Range<double> xRange);
	void runMaximumLikelihood(const AbstractColumn* xcol, double normalization);
	bool evaluate(bool preview = false);

	const Histogram* dataSourceHistogram{nullptr};
	QString dataSourceHistogramPath;
	const AbstractColumn* xErrorColumn{nullptr}; //<! column storing the values for the x-error to be used in the fit
	const AbstractColumn* yErrorColumn{nullptr}; //<! column storing the values for the y-error to be used in the fit
	QString xErrorColumnPath;
	QString yErrorColumnPath;

	XYFitCurve::FitData fitData;
	XYFitCurve::FitResult fitResult;
	QStringList solverOutput;

	Column* residualsColumn{nullptr};
	QVector<double>* residualsVector{nullptr};
	Note* resultsNote{nullptr};

	XYFitCurve* const q;

private:
	void prepareResultColumns();
	void writeSolverState(gsl_multifit_fdfsolver*, double chi = qQNaN());
};

#endif
