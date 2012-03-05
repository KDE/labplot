/***************************************************************************
    File                 : TextLabel.cpp
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

#include "worksheet/TextLabel.h"
#include "Worksheet.h"

#include <QFontMetricsF>
#include <QTextLayout>
#include <QTextLine>
#include <QPainter>
#include <QtDebug>


/**
 * \class TextLabel
 * \brief A one-line text label supporting floating point font sizes.
 *
 * 
 */

class TextLabelPrivate {
	public:
		TextLabelPrivate(TextLabel *owner) : q(owner) {
		}

		qreal fontSize;
		qreal rotationAngle;
		QFont font;
		QColor textColor;
		QString text;
		QPointF position;
		TextLabel::HorizontalAlignment horizontalAlignment;
		TextLabel::VerticalAlignment verticalAlignment;

		QTextLayout *layout;
		QRectF boundingRectangle;

		TextLabel * const q;
};

TextLabel::TextLabel()
		: d_ptr(new TextLabelPrivate(this)) {
	init();
}

TextLabel::TextLabel(TextLabelPrivate *dd)
		: d_ptr(dd) {
	init();
}

void TextLabel::init() {
	Q_D(TextLabel);

	d->rotationAngle = 0;
	d->fontSize = Worksheet::convertToSceneUnits(10.0, Worksheet::Point);
	d->textColor = QColor(Qt::black);
	d->font.setPointSizeF(1000);

	d->layout = NULL;
}

TextLabel::~TextLabel() {
	delete d_ptr;
}

BASIC_SHARED_D_READER_IMPL(TextLabel, qreal, fontSize, fontSize);
BASIC_SHARED_D_READER_IMPL(TextLabel, qreal, rotationAngle, rotationAngle);
CLASS_SHARED_D_READER_IMPL(TextLabel, QFont, font, font);
CLASS_SHARED_D_READER_IMPL(TextLabel, QColor, textColor, textColor);
CLASS_SHARED_D_READER_IMPL(TextLabel, QString, text, text);
CLASS_SHARED_D_READER_IMPL(TextLabel, QPointF, position, position);

void TextLabel::setFontSize(qreal size) {
	Q_D(TextLabel);
	
	d->fontSize = size;
	d->boundingRectangle = QRectF();
	
}

void TextLabel::setRotationAngle(qreal angle) {
	Q_D(TextLabel);

	d->rotationAngle = angle;
	d->boundingRectangle = QRectF();
}

void TextLabel::setFont(const QFont &font) {
	Q_D(TextLabel);

	QFont fontCopy = font;
	fontCopy.setPointSizeF(1000);

	if (fontCopy != d->font) {
		d->font = fontCopy;

		delete d->layout;
		d->layout = NULL;
		d->boundingRectangle = QRectF();
	}
}

void TextLabel::setTextColor(const QColor &color) {
	Q_D(TextLabel);

	d->textColor = color;
}

void TextLabel::setText(const QString &text) {
	Q_D(TextLabel);

	if (text != d->text) {
		d->text = text;

		delete d->layout;
		d->layout = NULL;
		d->boundingRectangle = QRectF();
	}
}

void TextLabel::setPosition(const QPointF &pos) {
	Q_D(TextLabel);

	d->position = pos;
	d->boundingRectangle = QRectF();
}

TextLabel::HorizontalAlignment TextLabel::horizontalAlignment() const {
	Q_D(const TextLabel);

	return d->horizontalAlignment;
}

TextLabel::VerticalAlignment TextLabel::verticalAlignment() const {
	Q_D(const TextLabel);

	return d->verticalAlignment;
}

void TextLabel::setAlignment(HorizontalAlignment hAlign, VerticalAlignment vAlign) {
	Q_D(TextLabel);

	d->horizontalAlignment = hAlign;
	d->verticalAlignment = vAlign;
	d->boundingRectangle = QRectF();
}

void TextLabel::paint(QPainter *painter) {
	Q_D(TextLabel);

	if (d->layout == NULL)
		createTextLayout();

	painter->save();

	painter->translate(d->position);

	painter->setFont(d->font);
	painter->setPen(QPen(d->textColor));

	qreal scaleFactor = d->fontSize / 1000.0;
	painter->scale(scaleFactor, scaleFactor);

	painter->rotate(d->rotationAngle);

	QTextLine line = d->layout->lineAt(0);
    QRectF textRect = line.naturalTextRect();
	QPointF pos;

	switch (d->horizontalAlignment) {
		case hAlignLeft:
			pos.setX(textRect.left());
			break;
		case hAlignCenter:
			pos.setX(textRect.center().x());
			break;
		case hAlignRight:
			pos.setX(textRect.right());
			break;
	}

	switch (d->verticalAlignment) {
		case vAlignTop:
			pos.setY(textRect.top());
			break;
		case vAlignCenter:
			pos.setY(textRect.center().y());
			break;
		case vAlignBottom:
			pos.setY(textRect.bottom());
			break;
	}

	line.draw(painter, -pos);

	painter->restore();
}

void TextLabel::createTextLayout() {
	Q_D(TextLabel);

	delete d->layout;
	d->layout = new QTextLayout(d->text, d->font);
	d->layout->setCacheEnabled(true);

	QTextOption option;
	option.setUseDesignMetrics(true);

	d->layout->setTextOption(option);
	d->layout->beginLayout();
	d->layout->createLine();
	d->layout->endLayout();
}

QRectF TextLabel::boundingRect() {
	Q_D(TextLabel);

	if (d->text.isEmpty())
		return QRectF();

	if (d->boundingRectangle.isNull()) {
		if (d->layout == NULL)
			createTextLayout();

		QTextLine line = d->layout->lineAt(0);
		QRectF textRect = line.naturalTextRect();
		QPointF pos;

		switch (d->horizontalAlignment) {
			case hAlignLeft:
				pos.setX(textRect.left());
				break;
			case hAlignCenter:
				pos.setX(textRect.center().x());
				break;
			case hAlignRight:
				pos.setX(textRect.right());
				break;
		}

		switch (d->verticalAlignment) {
			case vAlignTop:
				pos.setY(textRect.top());
				break;
			case vAlignCenter:
				pos.setY(textRect.center().y());
				break;
			case vAlignBottom:
				pos.setY(textRect.bottom());
				break;
		}

		textRect.translate(-pos);

		qreal scaleFactor = d->fontSize / 1000.0;
		textRect.setTopLeft(textRect.topLeft() * scaleFactor);
		textRect.setBottomRight(textRect.bottomRight() * scaleFactor);

		QMatrix matrix;
		matrix.rotate(d->rotationAngle);
		textRect = matrix.mapRect(textRect);
		textRect.translate(d->position);

		d->boundingRectangle = textRect;
	}

	return d->boundingRectangle;
}

