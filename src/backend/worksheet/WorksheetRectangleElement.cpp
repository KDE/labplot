/***************************************************************************
    File                 : WorksheetRectangleElement.cpp
    Project              : LabPlot/SciDAVis
    Description          : Rectangle worksheet (decoration) element.
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

#include "worksheet/WorksheetRectangleElement.h"
#include "worksheet/AbstractCoordinateSystem.h"

/**
 * \class WorksheetRectangleElement
 * \brief Rectangle worksheet (decoration) element.
 *
 * 
 */

WorksheetRectangleElement::WorksheetRectangleElement(const QString &name) 
		: AbstractWorksheetElement(name) {
}
WorksheetRectangleElement::~WorksheetRectangleElement() {
}

QList<QGraphicsItem *> WorksheetRectangleElement::graphicsItems() const {
	return QList<QGraphicsItem *>() << &m_item;
}

void WorksheetRectangleElement::setZValue(qreal z) {
	// TODO: undo cmd
	m_item.setZValue(z);
}

qreal WorksheetRectangleElement::zValue() const { 
	return m_item.zValue();
}

void WorksheetRectangleElement::setXScale(qreal xScale, bool keepAspectRatio) {
	// TODO
}

void WorksheetRectangleElement::setYScale(qreal yScale, bool keepAspectRatio) {
	// TODO
}

qreal WorksheetRectangleElement::xScale() const {
	// TODO
	return 1.0;
}

qreal WorksheetRectangleElement::yScale() const {
	// TODO
	return 1.0;
}

void WorksheetRectangleElement::setRotationAngle(qreal angle) {
	// TODO
}

qreal WorksheetRectangleElement::rotationAngle() const {
	// TODO
	return 0.0;
}

void WorksheetRectangleElement::setPosition(const QPointF &position) {
	// TODO
}

QPointF WorksheetRectangleElement::position() const {
	// TODO
	return QPoint(0.0, 0.0);
}

QRectF WorksheetRectangleElement::boundingRect() const {
	return m_item.boundingRect();
}

bool WorksheetRectangleElement::contains(const QPointF &position) const {
	// TODO
	return false;
}

void WorksheetRectangleElement::setVisible(bool on) {
	// TODO: undo cmd
	m_item.setVisible(on);
}

bool WorksheetRectangleElement::isVisible() const {
	return m_item.isVisible();
}

void WorksheetRectangleElement::setRect(const QRectF &rect) {
	m_rect = rect;
	// TODO: transform rect
	QRectF transformedRect = rect;
	m_item.setRect(transformedRect);
}

QRectF WorksheetRectangleElement::rect() const {
	return m_rect;
}

void WorksheetRectangleElement::transform(const AbstractCoordinateSystem &system) {
	QPointF topLeft = system.mapLogicalToScene(m_rect.topLeft());
	QPointF bottomRight = system.mapLogicalToScene(m_rect.bottomRight());
	m_item.setRect(QRectF(topLeft, bottomRight));
}

