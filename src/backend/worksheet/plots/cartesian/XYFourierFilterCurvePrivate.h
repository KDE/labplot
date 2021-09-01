/*
    File                 : XYFourierFilterCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYFourierFilterCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYFOURIERFILTERCURVEPRIVATE_H
#define XYFOURIERFILTERCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"

class XYFourierFilterCurve;
class Column;

class XYFourierFilterCurvePrivate: public XYAnalysisCurvePrivate {
public:
	explicit XYFourierFilterCurvePrivate(XYFourierFilterCurve*);
	~XYFourierFilterCurvePrivate() override;
	void recalculate();

	XYFourierFilterCurve::FilterData filterData;
	XYFourierFilterCurve::FilterResult filterResult;

	XYFourierFilterCurve* const q;
};

#endif
