/***************************************************************************
    File                 : CustomPointPrivate.h
    Project              : LabPlot
    Description          : Custom user-defined point on the plot
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
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


#ifndef CUSTOMPOINTPRIVATE_H
#define CUSTOMPOINTPRIVATE_H

#include <QGraphicsItem>

class CustomPointPrivate: public QGraphicsItem {
	public:
		explicit CustomPointPrivate(CustomPoint*, const CartesianPlot*);

		const CartesianPlot* plot;

		QString name() const;
		void retransform();
		bool swapVisible(bool);
		virtual void recalcShapeAndBoundingRect();
		void updatePosition();
		void updateData();

		bool suppressItemChangeEvent;
		bool suppressRetransform;
		bool m_printing;
		bool m_hovered;
		bool m_visible; //point inside the plot (visible) or not

		QRectF boundingRectangle;
		QRectF transformedBoundingRectangle;
		QPainterPath pointShape;

		QPointF position; //position in plot coordinates
		QPointF positionScene; //position in scene coordinatates

		//symbol
		Symbol::Style symbolStyle;
		QBrush symbolBrush;
		QPen symbolPen;
		qreal symbolOpacity;
		qreal symbolRotationAngle;
		qreal symbolSize;

		//reimplemented from QGraphicsItem
		QRectF boundingRect() const override;
		QPainterPath shape() const override;
		void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
		QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

		CustomPoint* const q;

	private:
		void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
		void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
		void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
		void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};

#endif
