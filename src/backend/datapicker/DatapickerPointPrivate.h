/*
    File                 : DatapickerPointPrivate.h
    Project              : LabPlot
    Description          : Graphic Item for coordinate points of Datapicker
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef DATAPICKERPOINTPRIVATE_H
#define DATAPICKERPOINTPRIVATE_H

class QGraphicsItem;

class DatapickerPointPrivate: public QGraphicsItem {
public:
	explicit DatapickerPointPrivate(DatapickerPoint*);

	QString name() const;
	void retransform();
	virtual void recalcShapeAndBoundingRect();
	void updatePoint();
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
