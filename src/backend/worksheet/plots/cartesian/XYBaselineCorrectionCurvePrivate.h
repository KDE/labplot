/*
	File             : XYBaselineCorrectionCurvePrivate.h
	Project          : LabPlot
	Description      : Private members of XYBaselineCorrectionCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYBASELINECORRECTIONCURVEPRIVATE_H
#define XYBASELINECORRECTIONCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYBaselineCorrectionCurve.h"

class XYBaselineCorrectionCurve;

class XYBaselineCorrectionCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYBaselineCorrectionCurvePrivate(XYBaselineCorrectionCurve*);
	~XYBaselineCorrectionCurvePrivate() override;

	virtual bool recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) override;
	virtual void resetResults() override;

	XYBaselineCorrectionCurve::BaselineData baselineData;
	XYBaselineCorrectionCurve::BaselineResult baselineResult;

	XYBaselineCorrectionCurve* const q;
};

#endif
