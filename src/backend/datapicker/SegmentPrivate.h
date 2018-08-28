/***************************************************************************
    File                 : SegmentPrivate.h
    Project              : LabPlot
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


#ifndef SEGMENTPRIVATE_H
#define SEGMENTPRIVATE_H

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

	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	Segment* const q;

private:
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

#endif
