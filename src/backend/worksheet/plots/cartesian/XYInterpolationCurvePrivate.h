/*
    File                 : XYInterpolationCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYInterpolationCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef XYINTERPOLATIONCURVEPRIVATE_H
#define XYINTERPOLATIONCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"

class XYInterpolationCurve;
class Column;

class XYInterpolationCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYInterpolationCurvePrivate(XYInterpolationCurve*);
	~XYInterpolationCurvePrivate() override;

	void recalculate();

	XYInterpolationCurve::InterpolationData interpolationData;
	XYInterpolationCurve::InterpolationResult interpolationResult;

	XYInterpolationCurve* const q;
};

#endif
