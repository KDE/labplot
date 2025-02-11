/*
	File                 : ReferenceLinePrivate.h
	Project              : LabPlot
	Description          : Reference line on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REFERENCELINEPRIVATE_H
#define REFERENCELINEPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"

class CartesianCoordinateSystem;

class ReferenceLinePrivate : public WorksheetElementPrivate {
public:
	explicit ReferenceLinePrivate(ReferenceLine*);

	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void updateOrientation();
	void updatePositionLimit();

	ReferenceLine::Orientation orientation{ReferenceLine::Orientation::Horizontal};
	double length{0.0}; // length of the line in graphic item's coordinates
	Line* line{nullptr};

	// reimplemented from QGraphicsItem
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	ReferenceLine* const q;
};

#endif
