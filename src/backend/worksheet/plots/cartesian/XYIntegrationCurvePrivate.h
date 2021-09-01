/*
    File                 : XYIntegrationCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYIntegrationCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYINTEGRATIONCURVEPRIVATE_H
#define XYINTEGRATIONCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"

class XYIntegrationCurve;
class Column;

class XYIntegrationCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYIntegrationCurvePrivate(XYIntegrationCurve*);
	~XYIntegrationCurvePrivate() override;

	void recalculate();

	XYIntegrationCurve::IntegrationData integrationData;
	XYIntegrationCurve::IntegrationResult integrationResult;

	XYIntegrationCurve* const q;
};

#endif
