/*
	File                 : XYPiecewiseLinearFitCurvePrivate.h
	Project              : LabPlot
	Description          : Private members of XYPiecewiseLinearFitCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYPIECEWISELINEARFITCURVEPRIVATE_H
#define XYPIECEWISELINEARFITCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYPiecewiseLinearFitCurve.h"

class XYPiecewiseLinearFitCurve;

class XYPiecewiseLinearFitCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYPiecewiseLinearFitCurvePrivate(XYPiecewiseLinearFitCurve*);
	~XYPiecewiseLinearFitCurvePrivate() override;

	virtual bool recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) override;
	virtual void resetResults() override;

	XYPiecewiseLinearFitCurve::FitData fitData;
	XYPiecewiseLinearFitCurve::FitResult fitResult;

	XYPiecewiseLinearFitCurve* const q;

private:
	void fitSegment(const QVector<double>& x, const QVector<double>& y, double& slope, double& intercept, double& rsquare);
};

#endif
