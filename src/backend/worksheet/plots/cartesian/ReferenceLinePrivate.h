/***************************************************************************
    File                 : ReferenceLinePrivate.h
    Project              : LabPlot
    Description          : Reference line on the plot
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
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


#ifndef REFERENCELINERIVATE_H
#define REFERENCELINEPRIVATE_H

#include <QGraphicsItem>

class ReferenceLinePrivate: public QGraphicsItem {
public:
	explicit ReferenceLinePrivate(ReferenceLine*, const CartesianPlot*);

	const CartesianPlot* plot;

	QString name() const;
	void retransform();
	bool swapVisible(bool);
	virtual void recalcShapeAndBoundingRect();
	void updatePosition();
	void updateData();

	bool suppressItemChangeEvent{false};
	bool suppressRetransform{false};
	bool m_printing{false};
	bool m_hovered{false};
	bool m_visible{true}; //point inside the plot (visible) or not

	QRectF boundingRectangle;
	QPainterPath lineShape;

	ReferenceLine::Orientation orientation;
	double position; //position in plot coordinates
	double length;
	QPen pen;
	qreal opacity;

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

	ReferenceLine* const q;

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};

#endif
