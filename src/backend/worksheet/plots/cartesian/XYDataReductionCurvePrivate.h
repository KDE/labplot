/*
    File                 : XYDataReductionCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYDataReductionCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYDATAREDUCTIONCURVEPRIVATE_H
#define XYDATAREDUCTIONCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"

class XYDataReductionCurve;
class Column;

class XYDataReductionCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYDataReductionCurvePrivate(XYDataReductionCurve*);
	~XYDataReductionCurvePrivate() override;

	void recalculate();

	XYDataReductionCurve::DataReductionData dataReductionData;
	XYDataReductionCurve::DataReductionResult dataReductionResult;

	XYDataReductionCurve* const q;
};

#endif
