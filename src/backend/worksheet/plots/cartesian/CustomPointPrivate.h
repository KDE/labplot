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

class CustomPoint;
class CartesianPlot;
class CartesianCoordinateSystem;
class Symbol;

class CustomPointPrivate: public QGraphicsItem {
public:
	explicit CustomPointPrivate(CustomPoint*);

	QString name() const;
	void retransform();
	bool swapVisible(bool);
	virtual void recalcShapeAndBoundingRect();
	void updatePosition();
	void updateData();

	bool suppressItemChangeEvent{false};
	bool suppressRetransform{false};
	bool m_hovered{false};
	bool m_visible{true}; //point inside the plot (visible) or not

	QRectF boundingRectangle;
	QRectF transformedBoundingRectangle;
	QPainterPath pointShape;

	QPointF position; //position in plot coordinates
	QPointF positionScene; //position in scene coordinates
	Symbol* symbol{nullptr};

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

	CustomPoint* const q;

private:
	const CartesianPlot* plot();
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	QPointF mapPlotAreaToParent(QPointF point);
	QPointF mapParentToPlotArea(QPointF point);
};

#endif
