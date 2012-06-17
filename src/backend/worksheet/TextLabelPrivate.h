/***************************************************************************
    File                 : TextLabelPrivate.h
    Project              : LabPlot/SciDAVis
    Description          : Private members of Worksheet.
    --------------------------------------------------------------------
    Copyright            : (C) 2012 by Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses) 
                           
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

#ifndef WORKSHEETPRIVATE_H
#define WORKSHEETPRIVATE_H

#include <QStaticText>

class TextLabelPrivate: public QGraphicsItem{
	public:
		TextLabelPrivate(TextLabel*);
	
		float rotationAngle;
		float scaleFactor;
		QString text;
		bool teXUsed;
		qreal teXFontSize;
		
		QPointF position;//point in parent's coordinate system, the label gets aligned around this point.
		TextLabel::HorizontalPosition horizontalPosition;
		TextLabel::VerticalPosition verticalPosition;
		
		TextLabel::HorizontalAlignment horizontalAlignment;
		TextLabel::VerticalAlignment verticalAlignment;

		TextLabel * const q;
		QString name() const;
		void retransform();
		bool swapVisible(bool on);
		virtual void recalcShapeAndBoundingRect();
		void updatePosition();
		void updateText();
		void updateTeXImage();
		QStaticText staticText;
		bool cancelItemChangeEvent;

		QRectF boundingRectangle; //bounding rectangle of the text
		QRectF transformedBoundingRectangle; //bounding rectangle of transformed (rotated etc.) text
		QPainterPath labelShape;

		//reimplemented from QGraphicsItem
		virtual QRectF boundingRect() const;
 		virtual QPainterPath shape() const;
		virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0);
		QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	private:
		QImage teXImage;
};

#endif
