/*
	File                 : ClevelandDotPlotPrivate.h
	Project              : LabPlot
	Description          : Cleveland Dot Plot - private implementation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLEVELANDDOTPRIVATE_H
#define CLEVELANDDOTPRIVATE_H

#include "backend/worksheet/plots/cartesian/LollipopPlotPrivate.h"
#include <QPen>

class CartesianCoordinateSystem;
class Line;
class Symbol;
class Value;

typedef QVector<QPointF> Points;

class ClevelandDotPlotPrivate : public LollipopPlotPrivate {
public:
	explicit ClevelandDotPlotPrivate(ClevelandDotPlot*);

};

#endif
