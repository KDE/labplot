/*
    File                 : XYDifferentiationCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYDifferentiationCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef XYDIFFERENTIATIONCURVEPRIVATE_H
#define XYDIFFERENTIATIONCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurvePrivate.h"
#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"

class XYDifferentiationCurve;
class Column;

class XYDifferentiationCurvePrivate : public XYAnalysisCurvePrivate {
public:
	explicit XYDifferentiationCurvePrivate(XYDifferentiationCurve*);
	~XYDifferentiationCurvePrivate() override;

	void recalculate();

	XYDifferentiationCurve::DifferentiationData differentiationData;
	XYDifferentiationCurve::DifferentiationResult differentiationResult;

	XYDifferentiationCurve* const q;
};

#endif
