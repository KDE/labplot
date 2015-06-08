#ifndef IMAGEPRIVATE_H
#define IMAGEPRIVATE_H

#include <QStaticText>
#include <QFutureWatcher>
#include <QGraphicsItem>


class ImagePrivate: public QGraphicsItem {
	public:
		explicit ImagePrivate(Image*);

		float rotationAngle;
		float scaleFactor;

		Image::PositionWrapper position;
		Image::ReferencePoints points;
		
		QImage graphImage;
		QPointF refPointCoordinate[3];
		QString name() const;
		void retransform();
		bool swapVisible(bool on);
		bool swapSelectPoint(bool on);
		virtual void recalcShapeAndBoundingRect();
		void updatePosition();

		bool suppressItemChangeEvent;
		bool suppressRetransform;
		bool m_printing;
		bool selectPoint;

		QRectF boundingRectangle; 
		QRectF transformedBoundingRectangle;
		QPainterPath imageShape;

		//reimplemented from QGraphicsItem
		virtual QRectF boundingRect() const;
 		virtual QPainterPath shape() const;
		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0);
		virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

		Image* const q;

	private:
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
};

#endif
