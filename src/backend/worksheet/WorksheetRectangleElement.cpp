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
 * This element will be a rectangle after transformation depends on the coordinate system.
 * 
 */


// TODO: Move as much stuff as possible into a common decoration element base class
// TODO: undo cmds


WorksheetRectangleElement::WorksheetRectangleElement(const QString &name) 
		: AbstractWorksheetElement(name) {
}
WorksheetRectangleElement::WorksheetRectangleElement(const QString &name, const QRectF &rect)
		: AbstractWorksheetElement(name), m_rect(rect.normalized()) {
}

WorksheetRectangleElement::~WorksheetRectangleElement() {
}

QGraphicsItem *WorksheetRectangleElement::graphicsItem() const {
	return &m_item;
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
	m_rect = rect.normalized();
	retransform();
}

QRectF WorksheetRectangleElement::rect() const {
	return m_rect;
}

void WorksheetRectangleElement::retransform() {
	AbstractCoordinateSystem *system = coordinateSystem();

	QPainterPath path;

	if (system) {
		QList<QLineF> lines;
		lines.append(QLineF(m_rect.topLeft(), m_rect.topRight()));
		lines.append(QLineF(m_rect.topRight(), m_rect.bottomRight()));
		lines.append(QLineF(m_rect.bottomRight(), m_rect.bottomLeft()));
		lines.append(QLineF(m_rect.bottomLeft(), m_rect.topLeft()));
		lines = system->mapLogicalToScene(lines);	
		
		foreach (QLineF line, lines) {
			path.moveTo(line.p1());
			path.lineTo(line.p2());
		}
	}
	else
		path.addRect(m_rect);
	
	m_item.setPath(path);
}
		
void WorksheetRectangleElement::handlePageResize(double horizontalRatio, double verticalRatio) {
	// TODO: rescale line width
	
	BaseClass::handlePageResize(horizontalRatio, verticalRatio);
}

