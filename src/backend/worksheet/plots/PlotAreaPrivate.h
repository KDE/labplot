/*
	File                 : PlotAreaPrivate.h
	Project              : LabPlot
	Description          : Private members of PlotArea.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLOTAREAPRIVATE_H
#define PLOTAREAPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"
#include <QPen>

class Background;
class PlotArea;
class QBrush;

class PlotAreaPrivate : public WorksheetElementPrivate {
public:
	explicit PlotAreaPrivate(PlotArea* owner);

	bool toggleClipping(bool on);
	bool clippingEnabled() const;
	void setRect(const QRectF&);
	void update();
	void retransform() override {
	}

	// QGraphicsItem's virtual functions
	void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	virtual void recalcShapeAndBoundingRect() override {
	}

	QRectF rect;
	Background* background{nullptr};

	PlotArea::BorderType borderType;
	Line* borderLine{nullptr};
	qreal borderCornerRadius{0.0};

	PlotArea* const q;
};

#endif
