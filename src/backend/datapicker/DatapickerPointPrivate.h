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

#include "backend/worksheet/Worksheet.h"

#include <QBrush>
#include <QGraphicsItem>
#include <QPen>

class DatapickerPoint;
class Symbol;

class DatapickerPointPrivate : public QGraphicsItem {
public:
	explicit DatapickerPointPrivate(DatapickerPoint*);

	QString name() const;
	void retransform();
	virtual void recalcShapeAndBoundingRect();
	void updateProperties();
	void retransformErrorBar();

	bool m_printing{false};
	bool isReferencePoint{false};

	Symbol* symbol{nullptr};
	QPointF position;
	QRectF m_boundingRectangle;
	QPainterPath m_shape;

	QPointF plusDeltaXPos;
	QPointF minusDeltaXPos;
	QPointF plusDeltaYPos;
	QPointF minusDeltaYPos;
	QBrush errorBarBrush;
	QPen errorBarPen;
	qreal errorBarSize{Worksheet::convertToSceneUnits(8, Worksheet::Unit::Point)};

	// reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	DatapickerPoint* const q;

private:
	void update() const;
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

	friend class DatapickerTest;
	friend class DatapickerPointSetMinusDeltaXPosCmd;
	friend class DatapickerPointSetPlusDeltaXPosCmd;
	friend class DatapickerPointSetMinusDeltaYPosCmd;
	friend class DatapickerPointSetPlusDeltaYPosCmd;
};

#endif
