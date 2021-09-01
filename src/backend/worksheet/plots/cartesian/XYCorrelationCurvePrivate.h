/*
    File                 : XYCorrelationCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYCorrelationCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYCORRELATIONCURVEPRIVATE_H
#define XYCORRELATIONCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYCorrelationCurve.h"

class XYCorrelationCurve;
class Column;

class XYCorrelationCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYCorrelationCurvePrivate(XYCorrelationCurve*);
	~XYCorrelationCurvePrivate() override;

	void recalculate();

	XYCorrelationCurve::CorrelationData correlationData;
	XYCorrelationCurve::CorrelationResult correlationResult;

	XYCorrelationCurve* const q;
};

#endif
