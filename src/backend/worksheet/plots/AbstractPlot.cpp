/***************************************************************************
    File                 : AbstractPlot.cpp
    Project              : LabPlot
    Description          : Base class for plots of different types
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2011-2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2020 Stefan Gerlach (stefan.gerlach@uni.kn)

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

AbstractPlot::AbstractPlot(const QString &name, AspectType type)
	: WorksheetElementContainer(name, new AbstractPlotPrivate(this), type) {

	init();
}

AbstractPlot::AbstractPlot(const QString &name, AbstractPlotPrivate *dd, AspectType type)
	: WorksheetElementContainer(name, dd, type) {

	init();
}

void AbstractPlot::init() {
	graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, true);
}

PlotArea* AbstractPlot::plotArea() {
	return m_plotArea;
}

AbstractCoordinateSystem* AbstractPlot::coordinateSystem(const int index) const {
	// TODO: use default when not specified?
	return m_coordinateSystems.at(index);
}

QVector<AbstractCoordinateSystem*> AbstractPlot::coordinateSystems() const {
	return m_coordinateSystems;
}

TextLabel* AbstractPlot::title() {
	return m_title;
}

void AbstractPlot::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	Q_UNUSED(pageResize)
	if (isLoading())
		return;
	DEBUG("AbstractPlot::handleResize()");
	Q_D(AbstractPlot);

// 	qDebug()<<name() << ": ratios - " << horizontalRatio << "  " << verticalRatio;

	if (horizontalRatio < 1 && horizontalRatio > 0.2) {
// 		qDebug()<<name() << ": old hor padding - " << d->horizontalPadding;
		d->horizontalPadding *= horizontalRatio;
// 		qDebug()<<name() << ": new hor padding - " << d->horizontalPadding;
		emit horizontalPaddingChanged(d->horizontalPadding);
	}

	if (verticalRatio < 1 && verticalRatio > 0.2) {
// 		qDebug()<<name() << ": old ver padding - " << d->verticalPadding;
		d->verticalPadding *= verticalRatio;
// 		qDebug()<<name() << ": new ver padding - " << d->verticalPadding;
		emit verticalPaddingChanged(d->verticalPadding);
	}

// 	WorksheetElementContainer::handleResize(horizontalRatio, verticalRatio, pageResize);
}

BASIC_SHARED_D_READER_IMPL(AbstractPlot, double, horizontalPadding, horizontalPadding)
BASIC_SHARED_D_READER_IMPL(AbstractPlot, double, verticalPadding, verticalPadding)
BASIC_SHARED_D_READER_IMPL(AbstractPlot, double, rightPadding, rightPadding)
BASIC_SHARED_D_READER_IMPL(AbstractPlot, double, bottomPadding, bottomPadding)
BASIC_SHARED_D_READER_IMPL(AbstractPlot, bool, symmetricPadding, symmetricPadding)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(AbstractPlot, SetHorizontalPadding, double, horizontalPadding, retransform)
void AbstractPlot::setHorizontalPadding(double padding) {
	Q_D(AbstractPlot);
	if (padding != d->horizontalPadding)
		exec(new AbstractPlotSetHorizontalPaddingCmd(d, padding, ki18n("%1: set horizontal padding")));
}

STD_SETTER_CMD_IMPL_F_S(AbstractPlot, SetVerticalPadding, double, verticalPadding, retransform)
void AbstractPlot::setVerticalPadding(double padding) {
	Q_D(AbstractPlot);
	if (padding != d->verticalPadding)
		exec(new AbstractPlotSetVerticalPaddingCmd(d, padding, ki18n("%1: set vertical padding")));
}

STD_SETTER_CMD_IMPL_F_S(AbstractPlot, SetRightPadding, double, rightPadding, retransform)
void AbstractPlot::setRightPadding(double padding) {
	Q_D(AbstractPlot);
	if (padding != d->rightPadding)
		exec(new AbstractPlotSetRightPaddingCmd(d, padding, ki18n("%1: set right padding")));
}

STD_SETTER_CMD_IMPL_F_S(AbstractPlot, SetBottomPadding, double, bottomPadding, retransform)
void AbstractPlot::setBottomPadding(double padding) {
	Q_D(AbstractPlot);
	if (padding != d->bottomPadding)
		exec(new AbstractPlotSetBottomPaddingCmd(d, padding, ki18n("%1: set bottom padding")));
}

STD_SETTER_CMD_IMPL_F_S(AbstractPlot, SetSymmetricPadding, bool, symmetricPadding, retransform)
void AbstractPlot::setSymmetricPadding(bool symmetric) {
	Q_D(AbstractPlot);
	if (symmetric != d->symmetricPadding)
		exec(new AbstractPlotSetSymmetricPaddingCmd(d, symmetric, ki18n("%1: set horizontal padding")));
}

//################################################################
//################### Private implementation #####################
//################################################################
AbstractPlotPrivate::AbstractPlotPrivate(AbstractPlot *owner)
	: WorksheetElementContainerPrivate(owner) {
}

QString AbstractPlotPrivate::name() const {
	return q->name();
}
