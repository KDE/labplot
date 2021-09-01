/*
    File                 : XYConvolutionCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYConvolutionCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYCONVOLUTIONCURVEPRIVATE_H
#define XYCONVOLUTIONCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"

class XYConvolutionCurve;
class Column;

class XYConvolutionCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYConvolutionCurvePrivate(XYConvolutionCurve*);
	~XYConvolutionCurvePrivate() override;

	void recalculate();

	XYConvolutionCurve::ConvolutionData convolutionData;
	XYConvolutionCurve::ConvolutionResult convolutionResult;

	XYConvolutionCurve* const q;
};

#endif
