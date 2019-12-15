/***************************************************************************
    File                 : ImagePrivate.h
    Project              : LabPlot
    Description          : Worksheet element to draw images
    --------------------------------------------------------------------
    Copyright            : (C) 2019 by Alexander Semke (alexander.semke@web.de)

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
	int width = (int)Worksheet::convertToSceneUnits(2.0, Worksheet::Centimeter);
	int height = (int)Worksheet::convertToSceneUnits(3.0, Worksheet::Centimeter);
	bool keepRatio{true}; //keep aspect ratio when scaling the image
	qreal rotationAngle{0.0};

	// position in parent's coordinate system, the image will be aligned around this point
	TextLabel::PositionWrapper position{
		QPoint(Worksheet::convertToSceneUnits(1, Worksheet::Centimeter),
			   Worksheet::convertToSceneUnits(1, Worksheet::Centimeter)),
		TextLabel::hPositionCenter,
		TextLabel::vPositionCenter};

	//alignment
	TextLabel::HorizontalAlignment horizontalAlignment{TextLabel::hAlignCenter};
	TextLabel::VerticalAlignment verticalAlignment{TextLabel::vAlignCenter};

	//border
	QPen borderPen{Qt::black, Worksheet::convertToSceneUnits(1.0, Worksheet::Point), Qt::SolidLine};
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
	bool m_printing{false};
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
};

#endif
