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

#include "backend/worksheet/WorksheetElementPrivate.h"
#include "backend/worksheet/TextLabel.h"

class QGraphicsSceneHoverEvent;

class ImagePrivate: public WorksheetElementPrivate {
public:
	explicit ImagePrivate(Image*);

	QImage image;
	QString fileName;
	bool embedded{true};
	qreal opacity{1.0};
	int width = (int)Worksheet::convertToSceneUnits(2.0, Worksheet::Unit::Centimeter);
	int height = (int)Worksheet::convertToSceneUnits(3.0, Worksheet::Unit::Centimeter);
	bool keepRatio{true}; //keep aspect ratio when scaling the image

	//border
	QPen borderPen{Qt::black, Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point), Qt::SolidLine};
	qreal borderOpacity{1.0};

	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void updateImage();
	void scaleImage();
	void updatePosition();
	void updateBorder();

	bool m_hovered{false};

	QRectF boundingRectangle; //bounding rectangle of the text
	QRectF transformedBoundingRectangle; //bounding rectangle of transformed (rotated etc.) text
	QPainterPath borderShapePath;
	QPainterPath imageShape;

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	Image* const q;

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;

	QImage m_image; //scaled and the actual version of the original image that is used for drawing
};

#endif
