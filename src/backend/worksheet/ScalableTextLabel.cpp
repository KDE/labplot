/***************************************************************************
    File                 : ScalableTextLabel.cpp
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

#include "worksheet/ScalableTextLabel.h"

#include <QFontMetricsF>
#include <QTextLayout>
#include <QTextLine>
#include <QPainter>
#include <QtDebug>


/**
 * \class ScalableTextLabel
 * \brief A one-line text label supporting floating point font sizes.
 *
 * 
 */

class ScalableTextLabelPrivate {
	public:
		ScalableTextLabelPrivate(ScalableTextLabel *owner) : q(owner) {
		}

		qreal fontSize;
		qreal rotationAngle;
		QFont font;
		QColor textColor;
		QString text;
		QPointF position;
		ScalableTextLabel::HorizontalAlignment horizontalAlignment;
		ScalableTextLabel::VerticalAlignment verticalAlignment;

		QTextLayout *layout;

		ScalableTextLabel * const q;
};

ScalableTextLabel::ScalableTextLabel()
		: d_ptr(new ScalableTextLabelPrivate(this)) {
	init();
}

ScalableTextLabel::ScalableTextLabel(ScalableTextLabelPrivate *dd)
		: d_ptr(dd) {
	init();
}

void ScalableTextLabel::init() {
	Q_D(ScalableTextLabel);

	d->rotationAngle = 0;
	d->fontSize = 10;
	d->textColor = QColor(Qt::black);
	d->font.setPointSizeF(1000);

	d->layout = NULL;
}

ScalableTextLabel::~ScalableTextLabel() {
	delete d_ptr;
}

BASIC_SHARED_D_READER_IMPL(ScalableTextLabel, qreal, fontSize, fontSize);
BASIC_SHARED_D_READER_IMPL(ScalableTextLabel, qreal, rotationAngle, rotationAngle);
CLASS_SHARED_D_READER_IMPL(ScalableTextLabel, QFont, font, font);
CLASS_SHARED_D_READER_IMPL(ScalableTextLabel, QColor, textColor, textColor);
CLASS_SHARED_D_READER_IMPL(ScalableTextLabel, QString, text, text);
CLASS_SHARED_D_READER_IMPL(ScalableTextLabel, QPointF, position, position);

void ScalableTextLabel::setFontSize(qreal size) {
	Q_D(ScalableTextLabel);
	
	d->fontSize = size;
}

void ScalableTextLabel::setRotationAngle(qreal angle) {
	Q_D(ScalableTextLabel);

	d->rotationAngle = angle;
}

void ScalableTextLabel::setFont(const QFont &font) {
	Q_D(ScalableTextLabel);

	QFont fontCopy = font;
	fontCopy.setPointSizeF(1000);

	if (fontCopy != d->font) {
		d->font = fontCopy;

		delete d->layout;
		d->layout = NULL;
	}
}

void ScalableTextLabel::setTextColor(const QColor &color) {
	Q_D(ScalableTextLabel);

	d->textColor = color;
}

void ScalableTextLabel::setText(const QString &text) {
	Q_D(ScalableTextLabel);

	if (text != d->text) {
		d->text = text;

		delete d->layout;
		d->layout = NULL;
	}
}

void ScalableTextLabel::setPosition(const QPointF &pos) {
	Q_D(ScalableTextLabel);

	d->position = pos;
}

ScalableTextLabel::HorizontalAlignment ScalableTextLabel::horizontalAlignment() const {
	Q_D(const ScalableTextLabel);

	return d->horizontalAlignment;
}

ScalableTextLabel::VerticalAlignment ScalableTextLabel::verticalAlignment() const {
	Q_D(const ScalableTextLabel);

	return d->verticalAlignment;
}

void ScalableTextLabel::setAlignment(HorizontalAlignment hAlign, VerticalAlignment vAlign) {
	Q_D(ScalableTextLabel);

	d->horizontalAlignment = hAlign;
	d->verticalAlignment = vAlign;
}

void ScalableTextLabel::paint(QPainter *painter) {
	Q_D(ScalableTextLabel);

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

void ScalableTextLabel::createTextLayout() {
	Q_D(ScalableTextLabel);

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




