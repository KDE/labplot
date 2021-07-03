/***************************************************************************
    File                 : PlotAreaPrivate.h
    Project              : LabPlot
    Description          : Private members of PlotArea.
    --------------------------------------------------------------------
    Copyright            : (C) 2011 by Alexander Semke (alexander.semke@web.de)

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

#ifndef PLOTAREAPRIVATE_H
#define PLOTAREAPRIVATE_H

#include <QGraphicsItem>
#include <QPen>

class QBrush;
class PlotArea;

class PlotAreaPrivate: public QGraphicsItem {
public:
	explicit PlotAreaPrivate(PlotArea *owner);

	QString name() const;
	bool swapVisible(bool on);
	bool toggleClipping(bool on);
	bool clippingEnabled() const;
	void setRect(const QRectF&);

	//QGraphicsItem's virtual functions
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	QRectF rect;
	PlotArea::BackgroundType backgroundType;
	PlotArea::BackgroundColorStyle backgroundColorStyle;
	PlotArea::BackgroundImageStyle backgroundImageStyle;
	Qt::BrushStyle backgroundBrushStyle;
	QColor backgroundFirstColor;
	QColor backgroundSecondColor;
	QString backgroundFileName;
	qreal backgroundOpacity;

	PlotArea::BorderType borderType;
	QPen borderPen;
	qreal borderOpacity;
	qreal borderCornerRadius;

	PlotArea* const q;
};

#endif
