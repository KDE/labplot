/*
    File                 : XYEquationCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYEquationCurve
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYEQUATIONCURVEPRIVATE_H
#define XYEQUATIONCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"

class XYEquationCurve;
class Column;

class XYEquationCurvePrivate: public XYCurvePrivate {
public:
	explicit XYEquationCurvePrivate(XYEquationCurve*);
	~XYEquationCurvePrivate() override;

	void recalculate();

	XYEquationCurve::EquationData equationData;
	Column* xColumn;
	Column* yColumn;
	QVector<double>* xVector;
	QVector<double>* yVector;

	XYEquationCurve* const q;
};

#endif
