/*
	File                 : ImagePrivate.h
	Project              : LabPlot
	Description          : Worksheet element to draw images
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEPRIVATE_H
#define IMAGEPRIVATE_H

#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/WorksheetElementPrivate.h"

class QGraphicsSceneHoverEvent;

class ImagePrivate : public WorksheetElementPrivate {
public:
	explicit ImagePrivate(Image*);

	QImage image; // original image
	QImage imageScaled; // scaled and the actual version of the original image that is used for drawing
	QString fileName;
	bool embedded{true};
	qreal opacity{1.0};
	int width = (int)Worksheet::convertToSceneUnits(2.0, Worksheet::Unit::Centimeter);
	int height = (int)Worksheet::convertToSceneUnits(3.0, Worksheet::Unit::Centimeter);
	bool keepRatio{true}; // keep aspect ratio when scaling the image
	Line* borderLine{nullptr};

	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void updateImage();
	void scaleImage();
	void updateBorder();

	QRectF transformedBoundingRectangle; // bounding rectangle of transformed (rotated etc.) text
	QPainterPath borderShapePath;
	QPainterPath imageShape;

	// reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	Image* const q;
};

#endif
