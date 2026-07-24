/*
	File                 : PlotAreaBackground.cpp
	Project              : LabPlot
	Description          : Graphics item for drawing plot area background and border
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PlotAreaBackground.h"
#include "CartesianPlot.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"

#include <QPainter>

PlotAreaBackgroundPrivate::PlotAreaBackgroundPrivate(CartesianPlot* plot)
	: QGraphicsItem()
	, q(plot) {
	setFlag(QGraphicsItem::ItemIsSelectable, false);
	setFlag(QGraphicsItem::ItemIsFocusable, false);
	setFlag(QGraphicsItem::ItemStacksBehindParent, true);
	setZValue(-1); // behind all other children (curves, axes, etc.)
}

QRectF PlotAreaBackgroundPrivate::boundingRect() const {
	return rect;
}

void PlotAreaBackgroundPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
	// draw the plot area background
	q->background()->draw(painter, rect, q->borderCornerRadius());

	// draw the plot area border
	const auto* borderLine = q->borderLine();
	if (borderLine->pen().style() != Qt::NoPen) {
		painter->save();
		painter->setPen(borderLine->pen());
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(borderLine->opacity());

		const auto borderType = q->borderType();
		if (qFuzzyIsNull(q->borderCornerRadius())) {
			const double w = rect.width();
			const double h = rect.height();
			const double x = rect.x();
			const double y = rect.y();
			if (borderType.testFlag(CartesianPlot::BorderTypeFlags::BorderLeft))
				painter->drawLine(x, y, x, y + h);
			if (borderType.testFlag(CartesianPlot::BorderTypeFlags::BorderTop))
				painter->drawLine(x, y, x + w, y);
			if (borderType.testFlag(CartesianPlot::BorderTypeFlags::BorderRight))
				painter->drawLine(x + w, y, x + w, y + h);
			if (borderType.testFlag(CartesianPlot::BorderTypeFlags::BorderBottom))
				painter->drawLine(x, y + h, x + w, y + h);
		} else {
			painter->drawRoundedRect(rect, q->borderCornerRadius(), q->borderCornerRadius());
		}
		painter->restore();
	}
}

void PlotAreaBackgroundPrivate::setRect(const QRectF& r) {
	prepareGeometryChange();
	rect = r;
}

void PlotAreaBackgroundPrivate::retransform() {
	// the rect is in the parent item's (CartesianPlotPrivate) coordinate system
	// CartesianPlotPrivate uses a centered coordinate system
	const auto& r = q->rect();
	setRect(QRectF(-r.width() / 2, -r.height() / 2, r.width(), r.height()));
}
