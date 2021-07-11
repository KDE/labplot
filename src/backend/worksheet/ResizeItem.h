/***************************************************************************
    File                 : ResizeItem.h
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

#ifndef RESIZEITEM_H
#define RESIZEITEM_H

#include <QGraphicsItem>
#include <QGraphicsRectItem>

class WorksheetElementContainer;
class QGraphicsSceneHoverEvent;

class ResizeItem : public QGraphicsItem {
public:
	ResizeItem(WorksheetElementContainer*);
	virtual ~ResizeItem();
	void setRect(QRectF);

private:
	enum Position {
		Top         = 0x1,
		Bottom      = 0x2,
		Left        = 0x4,
		TopLeft     = Top | Left,
		BottomLeft  = Bottom | Left,
		Right       = 0x8,
		TopRight    = Top | Right,
		BottomRight = Bottom | Right
	};

	class HandleItem : public QGraphicsRectItem {
	public:
		HandleItem(int positionFlags, ResizeItem*);
		int position() const;

	protected:
		QVariant itemChange(GraphicsItemChange, const QVariant&) override;
		void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
		void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;

	private:
		QPointF restrictPosition(const QPointF&);
		int m_position;
		ResizeItem* m_parent;
	};

	QRectF boundingRect() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0) override;
	WorksheetElementContainer* container();

	void setTopLeft(const QPointF&);
	void setTop(qreal);
	void setTopRight(const QPointF&);
	void setRight(qreal);
	void setBottomRight(const QPointF&);
	void setBottom(qreal);
	void setBottomLeft(const QPointF&);
	void setLeft(qreal);

private:
	void updateHandleItemPositions();

	QVector<HandleItem*> m_handleItems;
	QRectF m_rect;
	WorksheetElementContainer* m_container;
};

#endif // SIZEGRIPITEM_H
