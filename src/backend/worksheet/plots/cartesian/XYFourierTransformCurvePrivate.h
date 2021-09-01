/*
    File                 : XYFourierTransformCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYFourierTransformCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYFOURIERTRANSFORMCURVEPRIVATE_H
#define XYFOURIERTRANSFORMCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"

class XYFourierTransformCurve;
class Column;

class XYFourierTransformCurvePrivate: public XYAnalysisCurvePrivate {
public:
	explicit XYFourierTransformCurvePrivate(XYFourierTransformCurve*);
	~XYFourierTransformCurvePrivate() override;
	void recalculate();

	XYFourierTransformCurve::TransformData transformData;
	XYFourierTransformCurve::TransformResult transformResult;

	XYFourierTransformCurve* const q;
};

#endif
