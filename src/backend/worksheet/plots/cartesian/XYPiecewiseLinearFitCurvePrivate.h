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

class Note;
class XYPiecewiseLinearFitCurve;

class XYPiecewiseLinearFitCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYPiecewiseLinearFitCurvePrivate(XYPiecewiseLinearFitCurve*);
	~XYPiecewiseLinearFitCurvePrivate() override;

	void retransform() override;
	virtual bool recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) override;
	virtual void resetResults() override;
	void updateChangepointLines();
	void updateResultsNote();

	XYPiecewiseLinearFitCurve::FitData fitData;
	XYPiecewiseLinearFitCurve::FitResult fitResult;
	bool changepointLinesEnabled{false};
	Note* resultsNote{nullptr};

	XYPiecewiseLinearFitCurve* const q;

private:
	void fitSegment(const QVector<double>& x, const QVector<double>& y, size_t segmentIndex);
	void fitSegmentConstrained(const QVector<double>& x, const QVector<double>& y, double x0, double y0, size_t segmentIndex);
};

#endif
