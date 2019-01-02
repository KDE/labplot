/***************************************************************************
    File                 : TextLabelPrivate.h
    Project              : LabPlot
    Description          : Private members of TextLabel
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2014 by Alexander Semke (alexander.semke@web.de)

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

#ifndef TEXTLABELPRIVATE_H
#define TEXTLABELPRIVATE_H

#include <QStaticText>
#include <QFutureWatcher>
#include <QGraphicsItem>

class QGraphicsSceneHoverEvent;

class TextLabelPrivate: public QGraphicsItem {
public:
	explicit TextLabelPrivate(TextLabel*);

	qreal rotationAngle;
	float scaleFactor{1.0};
	int teXImageResolution{600};
	float teXImageScaleFactor{1.0};
	TextLabel::TextWrapper textWrapper;
	QFont teXFont;
	QColor fontColor;
	QColor backgroundColor;
	QImage teXImage;
	QFutureWatcher<QImage> teXImageFutureWatcher;
	bool teXRenderSuccessful{false};

	TextLabel::PositionWrapper position;	//position in parent's coordinate system, the label gets aligned around this point.
	bool positionInvalid{false};

	TextLabel::HorizontalAlignment horizontalAlignment{TextLabel::hAlignLeft};
	TextLabel::VerticalAlignment verticalAlignment{TextLabel::vAlignTop};

	TextLabel::BorderShape borderShape{TextLabel::NoBorder};
	QPen borderPen;
	qreal borderOpacity;

	QString name() const;
	void retransform();
	bool swapVisible(bool on);
	virtual void recalcShapeAndBoundingRect();
	void updatePosition();
	QPointF positionFromItemPosition(QPointF);
	void updateText();
	void updateTeXImage();
	void updateBorder();
	QStaticText staticText;

	bool suppressItemChangeEvent{false};
	bool suppressRetransform{false};
	bool m_printing{false};
	bool m_hovered{false};

	QRectF boundingRectangle; //bounding rectangle of the text
	QRectF transformedBoundingRectangle; //bounding rectangle of transformed (rotated etc.) text
	QPainterPath borderShapePath;
	QPainterPath labelShape;

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

	TextLabel* const q;

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};

#endif
