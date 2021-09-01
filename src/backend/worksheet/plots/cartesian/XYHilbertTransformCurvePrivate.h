/*
    File                 : XYHilbertTransformCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYHilbertTransformCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYHILBERTTRANSFORMCURVEPRIVATE_H
#define XYHILBERTTRANSFORMCURVEPRIVATE_H

#include "XYAnalysisCurvePrivate.h"
#include "XYHilbertTransformCurve.h"

class XYHilbertTransformCurve;
class Column;

class XYHilbertTransformCurvePrivate: public XYAnalysisCurvePrivate {
public:
	explicit XYHilbertTransformCurvePrivate(XYHilbertTransformCurve*);
	~XYHilbertTransformCurvePrivate() override;
	void recalculate();

	XYHilbertTransformCurve::TransformData transformData;
	XYHilbertTransformCurve::TransformResult transformResult;

	XYHilbertTransformCurve* const q;
};

#endif
