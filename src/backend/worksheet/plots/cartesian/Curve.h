/*
    File                 : Curve.h
    Project              : LabPlot
    Description          : Base class for curves (xy-curve, histogram, etc.).
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef CURVE_H
#define CURVE_H

class QPointF;

class Curve {
public:
	explicit Curve();
	virtual ~Curve();

	virtual bool activateCurve(QPointF mouseScenePos, double maxDist = -1) = 0;
	virtual void setHover(bool on) = 0;
};

#endif
