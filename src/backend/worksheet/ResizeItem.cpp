/***************************************************************************
    File                 : ResizeItem.cpp
    Project              : LabPlot
    Description          : Item allowing to resize worksheet elements with the mouse
    --------------------------------------------------------------------
    Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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

#include "ResizeItem.h"
#include "backend/worksheet/WorksheetElementContainer.h"
#include <QBrush>
#include <QCursor>

ResizeItem::HandleItem::HandleItem(int position, ResizeItem* parent) : QGraphicsRectItem(-10, -10, 20, 20, parent),
	m_position(position), m_parent(parent) {

	setBrush(QBrush(Qt::red));
	setFlag(ItemIsMovable);
	setFlag(ItemSendsGeometryChanges);
}

int ResizeItem::HandleItem::position() const {
	return m_position;
}

QVariant ResizeItem::HandleItem::itemChange(GraphicsItemChange change, const QVariant& value) {
	QVariant newValue = value;

	if (change == ItemPositionChange)
		newValue = restrictPosition(value.toPointF());
	else if (change == ItemPositionHasChanged) {
		QPointF pos = value.toPointF();

		switch (m_position) {
		case TopLeft:
			m_parent->setTopLeft(pos);
			break;
		case Top:
			m_parent->setTop(pos.y());
			break;
		case TopRight:
			m_parent->setTopRight(pos);
			break;
		case Right:
			m_parent->setRight(pos.x());
			break;
		case BottomRight:
			m_parent->setBottomRight(pos);
			break;
		case Bottom:
			m_parent->setBottom(pos.y());
			break;
		case BottomLeft:
			m_parent->setBottomLeft(pos);
			break;
		case Left:
			m_parent->setLeft(pos.x());
			break;
		}
	}

	return newValue;
}

void ResizeItem::HandleItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
	switch (m_position) {
	case TopLeft:
		setCursor(Qt::SizeBDiagCursor);
		break;
	case Top:
		setCursor(Qt::SizeVerCursor);
		break;
	case TopRight:
		setCursor(Qt::SizeBDiagCursor);
		break;
	case Right:
		setCursor(Qt::SizeHorCursor);
		break;
	case BottomRight:
		setCursor(Qt::SizeBDiagCursor);
		break;
	case Bottom:
		setCursor(Qt::SizeVerCursor);
		break;
	case BottomLeft:
		setCursor(Qt::SizeBDiagCursor);
		break;
	case Left:
		setCursor(Qt::SizeHorCursor);
		break;
	}
	QGraphicsItem::hoverLeaveEvent(event);
}

void ResizeItem::HandleItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
	QGraphicsItem::hoverLeaveEvent(event);
}

QPointF ResizeItem::HandleItem::restrictPosition(const QPointF& pos) {
	QPointF newPos = this->pos();

	if (m_position & Top || m_position & Bottom)
		newPos.setY(pos.y());

	if (m_position & Left || m_position & Right)
		newPos.setX(pos.x());

	if (m_position & Top && newPos.y() > m_parent->m_rect.bottom())
		newPos.setY(m_parent->m_rect.bottom());
	else if (m_position & Bottom && newPos.y() < m_parent->m_rect.top())
		newPos.setY(m_parent->m_rect.top());

	if (m_position & Left && newPos.x() > m_parent->m_rect.right())
		newPos.setX(m_parent->m_rect.right());
	else if (m_position & Right && newPos.x() < m_parent->m_rect.left())
		newPos.setX(m_parent->m_rect.left());

	return newPos;
}

ResizeItem::ResizeItem(WorksheetElementContainer* container) : QGraphicsItem(container->graphicsItem()),
m_container(container) {

	m_handleItems.append(new HandleItem(TopLeft, this));
	m_handleItems.append(new HandleItem(Top, this));
	m_handleItems.append(new HandleItem(TopRight, this));
	m_handleItems.append(new HandleItem(Right, this));
	m_handleItems.append(new HandleItem(BottomRight, this));
	m_handleItems.append(new HandleItem(Bottom, this));
	m_handleItems.append(new HandleItem(BottomLeft, this));
	m_handleItems.append(new HandleItem(Left, this));
}

ResizeItem::~ResizeItem() {
}

void ResizeItem::setRect(QRectF rect) {
	prepareGeometryChange();
	m_rect = mapRectFromScene(rect);
	updateHandleItemPositions();
}

QRectF ResizeItem::boundingRect() const {
	return m_rect;
}

void ResizeItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {

}

#define IMPL_SET_FN(TYPE, POS)                    \
void ResizeItem::set ## POS (TYPE v) {            \
	m_rect.set ## POS (v);                        \
	m_container->setRect(mapRectToScene(m_rect)); \
}

IMPL_SET_FN(qreal, Top)
IMPL_SET_FN(qreal, Right)
IMPL_SET_FN(qreal, Bottom)
IMPL_SET_FN(qreal, Left)
IMPL_SET_FN(const QPointF&, TopLeft)
IMPL_SET_FN(const QPointF&, TopRight)
IMPL_SET_FN(const QPointF&, BottomRight)
IMPL_SET_FN(const QPointF&, BottomLeft)

void ResizeItem::updateHandleItemPositions() {
	for (auto* item : m_handleItems) {
		item->setFlag(ItemSendsGeometryChanges, false);

		switch (item->position()) {
		case TopLeft:
			item->setPos(m_rect.topLeft());
			break;
		case Top:
			item->setPos(m_rect.left() + m_rect.width() / 2 - 1, m_rect.top());
			break;
		case TopRight:
			item->setPos(m_rect.topRight());
			break;
		case Right:
			item->setPos(m_rect.right(), m_rect.top() + m_rect.height() / 2 - 1);
			break;
		case BottomRight:
			item->setPos(m_rect.bottomRight());
			break;
		case Bottom:
			item->setPos(m_rect.left() + m_rect.width() / 2 - 1, m_rect.bottom());
			break;
		case BottomLeft:
			item->setPos(m_rect.bottomLeft());
			break;
		case Left:
			item->setPos(m_rect.left(), m_rect.top() + m_rect.height() / 2 - 1);
			break;
		}

		item->setFlag(ItemSendsGeometryChanges, true);
	}
}
