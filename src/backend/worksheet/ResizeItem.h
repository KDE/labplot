/*
	File                 : ResizeItem.h
	Project              : LabPlot
	Description          : Item allowing to resize worksheet elements with the mouse
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RESIZEITEM_H
#define RESIZEITEM_H

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>  // Include the necessary header for QGraphicsSceneMouseEvent

class WorksheetElementContainer;
class QGraphicsSceneHoverEvent;

class ResizeItem : public QGraphicsItem {
public:
	explicit ResizeItem(WorksheetElementContainer*);
	virtual ~ResizeItem();
	void setRect(QRectF);

private:
	// Position flags using bitwise OR for combinations
	enum Position {
		Top = 0x1,
		Bottom = 0x2,
		Left = 0x4,
		TopLeft = Top | Left,
		BottomLeft = Bottom | Left,
		Right = 0x8,
		TopRight = Top | Right,
		BottomRight = Bottom | Right
	};

	class HandleItem : public QGraphicsRectItem {
	public:
		HandleItem(int positionFlags, ResizeItem*);
		int position() const;

	protected:
		QVariant itemChange(GraphicsItemChange, const QVariant&) override;
		void mousePressEvent(QGraphicsSceneMouseEvent*) override;
		void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
		void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
		void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
		void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;  // Added mouseMoveEvent

	private:
		QPointF restrictPosition(const QPointF&);
		int m_position;
		ResizeItem* m_parent;
		QRectF m_oldRect;
		QPointF m_lastMousePos;  // Store the previous mouse position for delta calculation
	};

	QRectF boundingRect() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	WorksheetElementContainer* container();

		   // Setters for the positions
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

#endif // RESIZEITEM_H
