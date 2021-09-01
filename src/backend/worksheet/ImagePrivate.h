/*
    File                 : ImagePrivate.h
    Project              : LabPlot
    Description          : Worksheet element to draw images
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef IMAGEPRIVATE_H
#define IMAGEPRIVATE_H

#include <QGraphicsItem>

#include "backend/worksheet/TextLabel.h"

class QGraphicsSceneHoverEvent;

class ImagePrivate: public QGraphicsItem {
public:
	explicit ImagePrivate(Image*);

	QImage image;
	QString fileName;
	qreal opacity{1.0};
	int width = (int)Worksheet::convertToSceneUnits(2.0, Worksheet::Unit::Centimeter);
	int height = (int)Worksheet::convertToSceneUnits(3.0, Worksheet::Unit::Centimeter);
	bool keepRatio{true}; //keep aspect ratio when scaling the image
	qreal rotationAngle{0.0};

	// position in parent's coordinate system, the image will be aligned around this point
	TextLabel::PositionWrapper position{
		QPoint(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter),
			   Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter)),
		TextLabel::HorizontalPosition::Center,
		TextLabel::VerticalPosition::Center};

	//alignment
	TextLabel::HorizontalAlignment horizontalAlignment{TextLabel::HorizontalAlignment::Center};
	TextLabel::VerticalAlignment verticalAlignment{TextLabel::VerticalAlignment::Center};

	//border
	QPen borderPen{Qt::black, Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point), Qt::SolidLine};
	qreal borderOpacity{1.0};

	QString name() const;
	void retransform();
	bool swapVisible(bool on);
	virtual void recalcShapeAndBoundingRect();
	void updateImage();
	void scaleImage();
	void updatePosition();
	QPointF positionFromItemPosition(QPointF);
	void updateBorder();

	bool suppressItemChangeEvent{false};
	bool suppressRetransform{false};
	bool m_hovered{false};

	QRectF boundingRectangle; //bounding rectangle of the text
	QRectF transformedBoundingRectangle; //bounding rectangle of transformed (rotated etc.) text
	QPainterPath borderShapePath;
	QPainterPath imageShape;

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

	Image* const q;

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;

	bool parentRect(QRectF&);
};

#endif
