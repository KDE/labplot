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
		float scaleFactor;
		int teXImageResolution;
		float teXImageScaleFactor;
		TextLabel::TextWrapper textWrapper;
		QFont teXFont;
		QColor teXFontColor;
		QColor teXBackgroundColor;
		QImage teXImage;
		QFutureWatcher<QImage> teXImageFutureWatcher;
		bool teXRenderSuccessful;

		TextLabel::PositionWrapper position; //position in parent's coordinate system, the label gets aligned around this point.
		bool positionInvalid;

		TextLabel::HorizontalAlignment horizontalAlignment;
		TextLabel::VerticalAlignment verticalAlignment;

		QString name() const;
		void retransform();
		bool swapVisible(bool on);
		virtual void recalcShapeAndBoundingRect();
		void updatePosition();
		QPointF positionFromItemPosition(QPointF);
		void updateText();
		void updateTeXImage();
		QStaticText staticText;

		bool suppressItemChangeEvent;
		bool suppressRetransform;
		bool m_printing;
		bool m_hovered;

		QRectF boundingRectangle; //bounding rectangle of the text
		QRectF transformedBoundingRectangle; //bounding rectangle of transformed (rotated etc.) text
		QPainterPath labelShape;

		//reimplemented from QGraphicsItem
		QRectF boundingRect() const override;
 		QPainterPath shape() const override;
		void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0) override;
		QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

		TextLabel* const q;

	private:
		void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
		void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
		void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
		void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};

#endif
