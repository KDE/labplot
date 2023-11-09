/*
	File                 : CustomPointPrivate.h
	Project              : LabPlot
	Description          : Custom user-defined point on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUSTOMPOINTPRIVATE_H
#define CUSTOMPOINTPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"

class CustomPoint;
class CartesianPlot;
class CartesianCoordinateSystem;
class Symbol;

class CustomPointPrivate : public WorksheetElementPrivate {
public:
	explicit CustomPointPrivate(CustomPoint*);

	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void updateData();

	QPointF positionScene; // position in scene coordinates
	Symbol* symbol{nullptr};

	// reimplemented from QGraphicsItem
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	CustomPoint* const q;

private:
	const CartesianPlot* plot();
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
};

#endif
