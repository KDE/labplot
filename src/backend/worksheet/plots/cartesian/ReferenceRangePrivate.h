/*
	File                 : ReferenceRangePrivate.h
	Project              : LabPlot
	Description          : Reference range on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REFERENCERANGEPRIVATE_H
#define REFERENCERANGEPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"

class CartesianCoordinateSystem;

class ReferenceRangePrivate : public WorksheetElementPrivate {
public:
	explicit ReferenceRangePrivate(ReferenceRange*);

	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void updateOrientation();

	bool m_hovered{false};
	bool m_visible{true}; // point inside the plot (visible) or not

	QRectF boundingRectangle;
	QPainterPath rangeShape;

	ReferenceRange::Orientation orientation{ReferenceRange::Orientation::Horizontal};
	QPointF positionLogicalStart;
	QPointF positionLogicalEnd;
	QRectF rect;

	Line* line{nullptr};
	Background* background{nullptr};

	// reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	void drawFilling(QPainter*) const;

	ReferenceRange* const q;

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};

#endif
