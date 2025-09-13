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
 * when calculating the data ranges of the plot area.
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
