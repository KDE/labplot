#ifndef CUSTOMITEMPRIVATE_H
#define CUSTOMITEMPRIVATE_H

#include <QGraphicsItem>

class CustomItemPrivate: public QGraphicsItem {
	public:
		explicit CustomItemPrivate(CustomItem*);

		float rotationAngle;
		float scaleFactor;

        QPointF plusDeltaXPos;
        QPointF minusDeltaXPos;
        QPointF plusDeltaYPos;
        QPointF minusDeltaYPos;

		CustomItem::PositionWrapper position;

		QString name() const;
		void retransform();
		bool swapVisible(bool on);
		virtual void recalcShapeAndBoundingRect();
		void updatePosition();
        void updateData();
        void retransformErrorBar();

		bool suppressItemChangeEvent;
		bool suppressRetransform;
		bool m_printing;
		bool m_hovered;
		bool m_suppressHoverEvents;
		
		CustomItem::ItemsStyle itemsStyle;
		QBrush itemsBrush;
		QPen itemsPen;
		qreal itemsOpacity;
		qreal itemsRotationAngle;
		qreal itemsSize;
		QRectF boundingRectangle; 
		QRectF transformedBoundingRectangle;
		QPainterPath itemShape;

        QBrush errorBarBrush;
        QPen errorBarPen;
        qreal errorBarSize;
        bool xSymmetricError;
        bool ySymmetricError;

		//reimplemented from QGraphicsItem
		virtual QRectF boundingRect() const;
 		virtual QPainterPath shape() const;
		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0);
		virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

		CustomItem* const q;

	private:
		virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*);
		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
};

#endif
