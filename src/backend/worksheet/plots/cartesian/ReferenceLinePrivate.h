/*
    File                 : ReferenceLinePrivate.h
    Project              : LabPlot
    Description          : Reference line on the plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef REFERENCELINEPRIVATE_H
#define REFERENCELINEPRIVATE_H

#include <QGraphicsItem>

class CartesianCoordinateSystem;

class ReferenceLinePrivate: public QGraphicsItem {
public:
	explicit ReferenceLinePrivate(ReferenceLine*);

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
	QPainterPath lineShape;

	ReferenceLine::Orientation orientation{ReferenceLine::Orientation::Horizontal};
	double position{0.0}; //position in plot coordinates
	double length{0.0}; //length of the line in graphic item's coordinates
	QPen pen;
	qreal opacity{1.0};
	QPointF positionScene; //position of the graphics item in scene coordinates

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
