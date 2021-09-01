/*
    File                 : XYSmoothCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYSmoothCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYSMOOTHCURVEPRIVATE_H
#define XYSMOOTHCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"

class XYSmoothCurve;
class Column;

class XYSmoothCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYSmoothCurvePrivate(XYSmoothCurve*);
	~XYSmoothCurvePrivate() override;

	void recalculate();

	XYSmoothCurve::SmoothData smoothData;
	XYSmoothCurve::SmoothResult smoothResult;

	Column* roughColumn{nullptr};
	QVector<double>* roughVector{nullptr};

	XYSmoothCurve* const q;
};

#endif
