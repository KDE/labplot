/*
	File                 : PlotPrivate.h
	Project              : LabPlot
	Description          : Plot - private implementation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLOTPRIVATE_H
#define PLOTPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"

class PlotPrivate : public WorksheetElementPrivate {
public:
	explicit PlotPrivate(Plot*);

	Plot* const q;
};

#endif
