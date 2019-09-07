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

class QGraphicsItem;

class DatapickerPointPrivate: public QGraphicsItem {
public:
	explicit DatapickerPointPrivate(DatapickerPoint*);

	QString name() const;
	void retransform();
	virtual void recalcShapeAndBoundingRect();
	void updateData();
	void updatePropeties();
	void retransformErrorBar();

	bool m_printing{false};
	qreal rotationAngle;
	QPointF position;
	QRectF boundingRectangle;
	QRectF transformedBoundingRectangle;
	Symbol::Style pointStyle;
	QBrush brush;
	QPen pen;
	qreal opacity;
	qreal size;
	QPainterPath itemShape;

	QPointF plusDeltaXPos;
	QPointF minusDeltaXPos;
	QPointF plusDeltaYPos;
	QPointF minusDeltaYPos;
	QBrush errorBarBrush;
	QPen errorBarPen;
	qreal errorBarSize;

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	DatapickerPoint* const q;

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};

#endif
