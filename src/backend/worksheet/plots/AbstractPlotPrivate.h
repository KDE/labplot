/*
	File                 : AbstractPlotPrivate.h
	Project              : LabPlot
	Description          : Private members of AbstractPlot
	-------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2015 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACTPLOTPRIVATE_H
#define ABSTRACTPLOTPRIVATE_H

#include "backend/worksheet/WorksheetElementContainerPrivate.h"

class AbstractPlot;

class AbstractPlotPrivate : public WorksheetElementContainerPrivate {
public:
	explicit AbstractPlotPrivate(AbstractPlot* owner);
	~AbstractPlotPrivate() override = default;
	void retransform() override {
	}

	double horizontalPadding; // horiz. offset between the plot area and the area defining the coordinate system, in scene units
	double verticalPadding; // vert. offset between the plot area and the area defining the coordinate system, in scene units
	double rightPadding;
	double leftPadding;
	double bottomPadding;
	bool symmetricPadding;

	AbstractPlot* q{nullptr};
};

#endif
