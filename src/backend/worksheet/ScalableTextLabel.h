/***************************************************************************
    File                 : ScalableTextLabel.h
    Project              : LabPlot/SciDAVis
    Description          : A one-line text label supporting floating point font sizes.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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

#ifndef TEXTLABEL_H
#define TEXTLABEL_H

#include <QObject>
#include <QFont>
#include <QBrush>
#include <QPen>
#include "lib/macros.h"

class TextLabelPrivate;
class TextLabel: public QObject {
	Q_OBJECT

	public:
		TextLabel();
		~TextLabel();

		enum HorizontalAlignmentFlags {
			hAlignLeft = 0x01,
			hAlignCenter = 0x02,
			hAlignRight = 0x04,
		};
		Q_DECLARE_FLAGS(HorizontalAlignment, HorizontalAlignmentFlags)

		enum VerticalAlignmentFlags {
			vAlignTop = 0x10,
			vAlignCenter = 0x20,
			vAlignBottom = 0x40,
		};
		Q_DECLARE_FLAGS(VerticalAlignment, VerticalAlignmentFlags)

		BASIC_D_ACCESSOR_DECL(qreal, fontSize, FontSize);
		BASIC_D_ACCESSOR_DECL(qreal, rotationAngle, RotationAngle);
		CLASS_D_ACCESSOR_DECL(QFont, font, Font);
		CLASS_D_ACCESSOR_DECL(QColor, textColor, TextColor);
		CLASS_D_ACCESSOR_DECL(QString, text, Text);
		CLASS_D_ACCESSOR_DECL(QPointF, position, Position);
		HorizontalAlignment horizontalAlignment() const;
		VerticalAlignment verticalAlignment() const;
		void setAlignment(HorizontalAlignment hAlign, VerticalAlignment vAlign);

		QRectF boundingRect();

		typedef TextLabelPrivate Private;

    	void paint(QPainter *painter);

	protected:
		TextLabelPrivate * const d_ptr;
		TextLabel(TextLabelPrivate *dd);

	private:
    	Q_DECLARE_PRIVATE(TextLabel)
		void init();
		void createTextLayout();
};

#endif
