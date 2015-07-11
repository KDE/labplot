#ifndef CUSTOMITEMPRIVATE_H
#define CUSTOMITEMPRIVATE_H

#include <QGraphicsItem>


class SegmentPrivate: public QGraphicsItem {
	public:
		explicit SegmentPrivate(Segment*);

		void retransform();
		virtual void recalcShapeAndBoundingRect();

        double scaleFactor;
        bool m_hovered;
        QPainterPath linePath;
		QRectF boundingRectangle; 
		QRectF transformedBoundingRectangle;
        QPainterPath itemShape;

		virtual QRectF boundingRect() const;
 		virtual QPainterPath shape() const;
		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0);
		Segment* const q;

    private:
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*);
		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

#endif
