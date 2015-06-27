#ifndef CUSTOMITEMPRIVATE_H
#define CUSTOMITEMPRIVATE_H

#include <QGraphicsItem>


class CustomItemPrivate: public QGraphicsItem {
	public:
		explicit CustomItemPrivate(CustomItem*);

		float rotationAngle;
		float scaleFactor;

		CustomItem::PositionWrapper position;
		bool positionInvalid;
		CustomItem::ErrorBar itemErrorBar;

		QString name() const;
		void retransform();
		bool swapVisible(bool on);
		virtual void recalcShapeAndBoundingRect();
		void updatePosition();

		bool suppressItemChangeEvent;
		bool suppressRetransform;
		bool m_printing;
		bool m_hovered;
		
		CustomItem::ItemsStyle itemsStyle;
		QBrush itemsBrush;
		QPen itemsPen;
		qreal itemsOpacity;
		qreal itemsRotationAngle;
		qreal itemsSize;
		//not used
		//qreal itemsAspectRatio;

		QRectF boundingRectangle; 
		QRectF transformedBoundingRectangle;
		QPainterPath itemShape;

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
