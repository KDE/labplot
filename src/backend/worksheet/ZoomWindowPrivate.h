#ifndef ZOOMWINDOWPRIVATE_H
#define ZOOMWINDOWPRIVATE_H

#include <QGraphicsPixmapItem>


class ZoomWindowPrivate: public QGraphicsPixmapItem {
	public:
        explicit ZoomWindowPrivate(ZoomWindow*);

		QString name() const;
		void retransform();

		bool m_printing;
		bool m_hovered;

		QRectF boundingRectangle;
		QPainterPath windowShape;

		virtual QRectF boundingRect() const;
		virtual QPainterPath shape() const;
		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0);

		ZoomWindow* const q;

	private:
		virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*);
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*);
		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
};

#endif
