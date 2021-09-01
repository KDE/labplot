/*
    File                 : CustomPointPrivate.h
    Project              : LabPlot
    Description          : Custom user-defined point on the plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
    SPDX-FileCopyrightText: 2015 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
