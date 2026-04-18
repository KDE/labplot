/*
	File                 : Plot.h
	Project              : LabPlot
	Description          : Base class for all plots like scatter plot, box plot, etc.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2026 Alexander Semke <alexander.semke@web.de>
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// documentation of pure virtual functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \fn bool Plot::hasData()
 * \brief returns \c true if a valid data column is set, returns \c false otherwise.
 * Used in CartesianPlot to determine whether the curve needs to be taken into account
 * when calculating the data ranges of the plot area.
 */

/*!
 * \fn int dataCount(Dimension dim) const
 * \brief dataCount
 * Number of elements in a specific direction
 * \param dim
 * \return Number of data or -1 if the plot is invalid (so it will not be considered for autoscale)
 */

/*!
 * \fn bool minMax(Dimension dim, const Range<int>& indexRange, Range<double>& rOut, bool includeErrorBars = true) const
 * \brief minMax
 * \param dim
 * \param indexRange
 * \param rOut
 * \param includeErrorBars
 * \return The minimum and maximum in the range \p rOut between the indices \p indexRange for dimension \p dim.
 */

/*!
 * \fn double minimum(Dimension dim) const
 * \brief minimum
 * \param dim
 * \return Returns the absolute minimum value for the dimension \p dim
 */

/*!
 * \fn double maximum(Dimension dim) const
 * \brief maximum
 * \param dim
 * \return Returns the absolute maximum value for the dimension \p dim
 */

/*!
 * \fn bool indicesMinMax(Dimension dim, double v1, double v2, int& start, int& end) const
 * \brief indicesMinMax
 * \param dim
 * \param v1 Start value
 * \param v2 End value
 * \param start Found start index
 * \param end Found end index
 * \return true if the indices can be found otherwise false. Return false, for example if a required column is not available, ...
 */

/*!
 * \fn bool usingColumn(const AbstractColumn*, bool indirect = true) const
 * returns \c true if the column is used internally in the plot for the visualisation, returns \c false otherwise.
 * If \p indirect is true it returns true also if a depending curve uses that column
 */

/*!
 * \fn void recalc()
 * recalculates the internal structures (additional data containers, drawing primitives, etc.) on data changes in the source data columns.
 * these structures are used in the plot during the actual drawing of the plot on geometry changes.
 */

Plot::Plot(const QString& name, PlotPrivate* dd, AspectType type)
	: WorksheetElement(name, dd, type)
	, d_ptr(dd) {
}

Plot::~Plot() = default;

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

/*!
 * for performance reasons and not to run into uninialized objects, the retransform call shouldb be suppressed sometimes -
 * either because explicitly requested or some other conditions like being unvisible, etc. are fullfilled.
 * This helper function returns \c true if the retransform() call should be skipped and returns \c false, otherwise.
 */
bool PlotPrivate::retransformSuppressed() const {
	return (!isVisible() || q->isLoading() || suppressRetransform || !m_plot);
}

void PlotPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	if (q->activatePlot(event->pos())) {
		q->createContextMenu()->exec(event->screenPos());
		return;
	}
	QGraphicsItem::contextMenuEvent(event);
}
