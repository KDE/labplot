/***************************************************************************
    File                 : PlotArea.cpp
    Project              : LabPlot/SciDAVis
    Description          : Plot area (for background filling and clipping).
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
                           
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

#include "worksheet/PlotArea.h"
#include "worksheet/PlotAreaPrivate.h"
#include "worksheet/AbstractCoordinateSystem.h"
#include "lib/commandtemplates.h"
#include "lib/macros.h"
#include <QPainter>
#include <QPen>
#include <QtDebug>
#include "worksheet/Worksheet.h"
#include "worksheet/WorksheetGraphicsScene.h"

/**
 * \class PlotArea
 * \brief Plot area (for background filling and clipping).
 *
 * 
 */

PlotArea::PlotArea(const QString &name) 
	: WorksheetElementContainer(name, new PlotAreaPrivate(this)) {
}

PlotArea::PlotArea(const QString &name, PlotAreaPrivate *dd)
    : WorksheetElementContainer(name, dd) {
}

PlotArea::~PlotArea() {
}

PlotAreaPrivate::PlotAreaPrivate(PlotArea *owner)
	: WorksheetElementContainerPrivate(owner) {
	rect = QRectF(0, 0, 10, 10);
	setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
}

PlotAreaPrivate::~PlotAreaPrivate() {
}

QString PlotAreaPrivate::name() const {
	return q->name();
}


/* ============================ accessor documentation ================= */
/**
  \fn PlotArea::BASIC_D_ACCESSOR_DECL(bool, clippingEnabled, ClippingEnabled);
  \brief Set/get whether clipping is enabled.
*/
/**
  \fn PlotArea::CLASS_D_ACCESSOR_DECL(QRectF, rect, Rect);
  \brief Set/get the plot area rectangle.
*/

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(PlotArea, bool, clippingEnabled, clippingEnabled());
CLASS_SHARED_D_READER_IMPL(PlotArea, QRectF, rect, rect);

/* ============================ setter methods and undo commands ================= */

STD_SWAP_METHOD_SETTER_CMD_IMPL(PlotArea, SetClippingEnabled, bool, toggleClipping);
void PlotArea::setClippingEnabled(bool on) {
	Q_D(PlotArea);

	if (d->clippingEnabled() != on)
		exec(new PlotAreaSetClippingEnabledCmd(d, on, tr("%1: toggle clipping")));
}

bool PlotAreaPrivate::clippingEnabled() const {
	return (flags() & QGraphicsItem::ItemClipsChildrenToShape);
}

bool PlotAreaPrivate::toggleClipping(bool on) {
	bool oldValue = clippingEnabled();
	setFlag(QGraphicsItem::ItemClipsChildrenToShape, on);
	return oldValue;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL_F(PlotArea, SetRect, QRectF, swapRect, q->retransform);
void PlotArea::setRect(const QRectF &newRect) {
	Q_D(PlotArea);

	if (d->rect != newRect)
		exec(new PlotAreaSetRectCmd(d, newRect, tr("%1: set plot rectangle")));
}

QRectF PlotAreaPrivate::swapRect(const QRectF &newRect) {
	QRectF oldRect = rect;
	prepareGeometryChange();
	rect = newRect;
	return oldRect;
}

void PlotAreaPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		QWidget *widget) {
	Q_UNUSED(painter)
	Q_UNUSED(option)
	Q_UNUSED(widget)
}

QRectF PlotAreaPrivate::boundingRect () const {
	return transformedRect; 
}

QPainterPath PlotAreaPrivate::shape() const {
	QPainterPath path;
	path.addRect(transformedRect);
	return path;
}

void PlotArea::retransform() {
	Q_D(PlotArea);

	AbstractCoordinateSystem *system = coordinateSystem();
	if (system) {
		QPointF topLeft = system->mapLogicalToScene(d->rect.topLeft());
		QPointF bottomRight = system->mapLogicalToScene(d->rect.bottomRight());
		d->transformedRect = QRectF(topLeft, bottomRight);
	}
	else
		d->transformedRect = d->rect;

	WorksheetElementContainer::retransform();
}


