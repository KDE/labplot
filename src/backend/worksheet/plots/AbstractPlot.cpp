/***************************************************************************
    File                 : AbstractPlot.cpp
    Project              : LabPlot
    Description          : Base class for plots of different types
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
	Copyright            : (C) 2011-2017 by Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "backend/worksheet/plots/AbstractPlot.h"
#include "backend/worksheet/plots/AbstractPlotPrivate.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/AbstractCoordinateSystem.h"
#include "backend/worksheet/WorksheetElementContainerPrivate.h"
#include "backend/lib/commandtemplates.h"

#include <KLocalizedString>

/**
 * \class AbstractPlot
 * \brief Second level container in a Worksheet for logical grouping
 *
 * TODO: decide the exact role of AbstractPlot
 *
 */

AbstractPlot::AbstractPlot(const QString &name):WorksheetElementContainer(name, new AbstractPlotPrivate(this)),
	m_coordinateSystem(nullptr), m_plotArea(nullptr), m_title(nullptr){
	init();
}

AbstractPlot::AbstractPlot(const QString &name, AbstractPlotPrivate *dd)
	: WorksheetElementContainer(name, dd),
	m_coordinateSystem(nullptr), m_plotArea(nullptr), m_title(nullptr){
	init();
}

void AbstractPlot::init(){
	graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, true);
}

PlotArea* AbstractPlot::plotArea(){
	return m_plotArea;
}

AbstractCoordinateSystem* AbstractPlot::coordinateSystem() const{
	return m_coordinateSystem;
}

TextLabel* AbstractPlot::title(){
	return m_title;
}

void AbstractPlot::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	DEBUG("AbstractPlot::handleResize()");
	Q_D(AbstractPlot);

	double ratio = 0;
	if (horizontalRatio > 1.0 || verticalRatio > 1.0)
		ratio = qMax(horizontalRatio, verticalRatio);
	else
		ratio = qMin(horizontalRatio, verticalRatio);

	d->horizontalPadding *= ratio;
	d->verticalPadding *= ratio;

	WorksheetElementContainer::handleResize(horizontalRatio, verticalRatio, pageResize);
}

BASIC_SHARED_D_READER_IMPL(AbstractPlot, float, horizontalPadding, horizontalPadding)
BASIC_SHARED_D_READER_IMPL(AbstractPlot, float, verticalPadding, verticalPadding)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(AbstractPlot, SetHorizontalPadding, float, horizontalPadding, retransform)
void AbstractPlot::setHorizontalPadding(float padding) {
	Q_D(AbstractPlot);
	if (padding != d->horizontalPadding)
		exec(new AbstractPlotSetHorizontalPaddingCmd(d, padding, ki18n("%1: set horizontal padding")));
}

STD_SETTER_CMD_IMPL_F_S(AbstractPlot, SetVerticalPadding, float, verticalPadding, retransform)
void AbstractPlot::setVerticalPadding(float padding) {
	Q_D(AbstractPlot);
	if (padding != d->verticalPadding)
		exec(new AbstractPlotSetVerticalPaddingCmd(d, padding, ki18n("%1: set vertical padding")));
}

//################################################################
//################### Private implementation #####################
//################################################################
AbstractPlotPrivate::AbstractPlotPrivate(AbstractPlot *owner)
	:WorksheetElementContainerPrivate(owner){
}

QString AbstractPlotPrivate::name() const{
	return q->name();
}
