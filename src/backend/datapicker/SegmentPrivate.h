/*
    File                 : SegmentPrivate.h
    Project              : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef SEGMENTPRIVATE_H
#define SEGMENTPRIVATE_H

#include <QGraphicsItem>
#include <QPen>

class SegmentPrivate: public QGraphicsItem {
public:
	explicit SegmentPrivate(Segment*);

	void retransform();
	virtual void recalcShapeAndBoundingRect();

	double scaleFactor;
	bool m_hovered{false};
	QPen pen;
	QPainterPath linePath;
	QRectF boundingRectangle;
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
