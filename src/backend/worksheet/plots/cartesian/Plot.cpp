/*
	File                 : Plot.h
	Project              : LabPlot
	Description          : Base class for all plots like scatter plot, box plot, etc.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Plot.h"
#include "PlotPrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Background.h"

#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

/**
 * \fn bool Plot::hasData()
 * \brief returns \c true if a valid data column is set, returns \c false otherwise.
 * Used in CartesianPlot to determine whether the curve needs to be taken into account
 * when caclulating the data ranges of the plot area.
 */

Plot::Plot(const QString& name, PlotPrivate* dd, AspectType type)
	: WorksheetElement(name, dd, type)
	, d_ptr(dd) {
}

Plot::~Plot() = default;

// TODO: make this function pure abstract and implement it for all plot types
bool Plot::minMax(const CartesianCoordinateSystem::Dimension, const Range<int>&, Range<double>&, bool) const {
	return false;
}

/*!
 * \brief Plot::activatePlot
 * Checks if the mousepos distance to the plot is less than @p maxDist
 * \p mouseScenePos
 * \p maxDist Maximum distance the point lies away from the plot
 * \return Returns true if the distance is smaller than maxDist.
 */
bool Plot::activatePlot(QPointF mouseScenePos, double maxDist) {
	Q_D(Plot);
	return d->activatePlot(mouseScenePos, maxDist);
}

// general
BASIC_SHARED_D_READER_IMPL(Plot, bool, legendVisible, legendVisible)

STD_SETTER_CMD_IMPL_S(Plot, SetLegendVisible, bool, legendVisible)
void Plot::setLegendVisible(bool visible) {
	Q_D(Plot);
	if (visible != d->legendVisible)
		exec(new PlotSetLegendVisibleCmd(d, visible, ki18n("%1: legend visibility changed")));
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
PlotPrivate::PlotPrivate(Plot* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
}

bool PlotPrivate::activatePlot(QPointF mouseScenePos, double /*maxDist*/) {
	if (!isVisible())
		return false;

	return m_shape.contains(mouseScenePos);
}

void PlotPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	if (q->activatePlot(event->pos())) {
		q->createContextMenu()->exec(event->screenPos());
		return;
	}
	QGraphicsItem::contextMenuEvent(event);
}

void PlotPrivate::drawFillingPollygon(const QPolygonF& polygon, QPainter* painter, const Background* background) const {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	const QRectF& rect = polygon.boundingRect();

	if (background->type() == Background::Type::Color) {
		switch (background->colorStyle()) {
		case Background::ColorStyle::SingleColor: {
			painter->setBrush(QBrush(background->firstColor()));
			break;
		}
		case Background::ColorStyle::HorizontalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::VerticalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::TopLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::BottomLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
			linearGrad.setColorAt(0, background->firstColor());
			linearGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case Background::ColorStyle::RadialGradient: {
			QRadialGradient radialGrad(rect.center(), rect.width() / 2);
			radialGrad.setColorAt(0, background->firstColor());
			radialGrad.setColorAt(1, background->secondColor());
			painter->setBrush(QBrush(radialGrad));
			break;
		}
		}
	} else if (background->type() == Background::Type::Image) {
		if (!background->fileName().trimmed().isEmpty()) {
			QPixmap pix(background->fileName());
			switch (background->imageStyle()) {
			case Background::ImageStyle::ScaledCropped:
				pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case Background::ImageStyle::Scaled:
				pix = pix.scaled(rect.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case Background::ImageStyle::ScaledAspectRatio:
				pix = pix.scaled(rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
				break;
			case Background::ImageStyle::Centered: {
				QPixmap backpix(rect.size().toSize());
				backpix.fill();
				QPainter p(&backpix);
				p.drawPixmap(QPointF(0, 0), pix);
				p.end();
				painter->setBrush(QBrush(backpix));
				painter->setBrushOrigin(-pix.size().width() / 2, -pix.size().height() / 2);
				break;
			}
			case Background::ImageStyle::Tiled:
				painter->setBrush(QBrush(pix));
				break;
			case Background::ImageStyle::CenterTiled:
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width() / 2, pix.size().height() / 2);
			}
		}
	} else if (background->type() == Background::Type::Pattern)
		painter->setBrush(QBrush(background->firstColor(), background->brushStyle()));

	painter->drawPolygon(polygon);
}
