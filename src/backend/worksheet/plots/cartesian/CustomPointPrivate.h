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

class CustomPointPrivate: public WorksheetElementPrivate {
public:
	explicit CustomPointPrivate(CustomPoint*);

	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void updatePosition();
	void updateData();

	bool m_hovered{false};
	bool m_visible{true}; //point inside the plot (visible) or not

	QPainterPath pointShape;

	QPointF positionScene; //position in scene coordinates
	Symbol* symbol{nullptr};

	//reimplemented from QGraphicsItem
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	CustomPoint* const q;

private:
	const CartesianPlot* plot();
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};

#endif
