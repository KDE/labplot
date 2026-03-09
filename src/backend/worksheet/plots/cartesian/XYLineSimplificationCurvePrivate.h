/*
	File                 : XYLineSimplificationCurvePrivate.h
	Project              : LabPlot
	Description          : Private members of XYLineSimplificationCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYLINESIMPLIFICATIONCURVEPRIVATE_H
#define XYLINESIMPLIFICATIONCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYLineSimplificationCurve.h"

class XYLineSimplificationCurve;
class Column;

class XYLineSimplificationCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYLineSimplificationCurvePrivate(XYLineSimplificationCurve*);
	~XYLineSimplificationCurvePrivate() override;

	virtual bool recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) override;
	virtual void resetResults() override;
	const XYAnalysisCurve::Result& result() const;

	XYLineSimplificationCurve::LineSimplificationData lineSimplificationData;
	XYLineSimplificationCurve::LineSimplificationResult lineSimplificationResult;

	XYLineSimplificationCurve* const q;
};

#endif
