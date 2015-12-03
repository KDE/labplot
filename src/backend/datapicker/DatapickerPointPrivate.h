/***************************************************************************
    File                 : DatapickerPointPrivate.h
    Project              : LabPlot
    Description          : Graphic Item for coordinate points of Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
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


#ifndef DATAPICKERPOINTPRIVATE_H
#define DATAPICKERPOINTPRIVATE_H

#include <QGraphicsItem>

class DatapickerPointPrivate: public QGraphicsItem {
	public:
        explicit DatapickerPointPrivate(DatapickerPoint*);

		float scaleFactor;

        QPointF plusDeltaXPos;
        QPointF minusDeltaXPos;
        QPointF plusDeltaYPos;
        QPointF minusDeltaYPos;

        DatapickerPoint::PositionWrapper position;

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
		
        qreal rotationAngle;
        QRectF boundingRectangle;
        QRectF transformedBoundingRectangle;
        DatapickerPoint::PointsStyle style;
        QBrush brush;
        QPen pen;
        qreal opacity;
        qreal size;
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

        DatapickerPoint* const q;

	private:
		virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*);
		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
};

#endif
