/*
	File                 : Plot.h
	Project              : LabPlot
	Description          : Base class for all plots like scatter plot, box plot, etc.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Plot.h"
#include "PlotPrivate.h"

Plot::Plot(const QString& name, PlotPrivate* dd, AspectType type)
	: WorksheetElement(name, dd, type)
	, d_ptr(dd) {
}

Plot::~Plot() = default;

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
PlotPrivate::PlotPrivate(Plot* owner)
	: WorksheetElementPrivate(owner),
	q(owner) {
}

