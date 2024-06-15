/*
	File                 : XYEquationCurvePrivate.h
	Project              : LabPlot
	Description          : Private members of XYEquationCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYEQUATIONCURVE2PRIVATE_H
#define XYEQUATIONCURVE2PRIVATE_H

#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"

class XYEquationCurve2;
class Column;

class XYEquationCurve2Private : public XYCurvePrivate {
public:
	explicit XYEquationCurve2Private(XYEquationCurve2*);
	~XYEquationCurve2Private() override;

	void recalculate();

	XYEquationCurve2::EquationData equationData;
	Column* xColumn;
	Column* yColumn;
	QVector<double>* xVector;
	QVector<double>* yVector;

	XYEquationCurve2* const q;
};

#endif
