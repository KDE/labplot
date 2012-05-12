/***************************************************************************
    File                 : PlotAreaPrivate.h
    Project              : LabPlot/SciDAVis
    Description          : Private members of PlotArea.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011 by Alexander Semke (alexander.semke*web.de)
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

#ifndef PLOTAREAPRIVATE_H
#define PLOTAREAPRIVATE_H

#include <QGraphicsItem>
#include <QBrush>
#include <QPen>

class PlotArea;
class PlotAreaPrivate: public QGraphicsItem{
	public:
		PlotAreaPrivate(PlotArea *owner);
		virtual ~PlotAreaPrivate();

		QString name() const;
		bool swapVisible(bool on);
		bool toggleClipping(bool on);
		bool clippingEnabled() const;
		QRectF swapRect(const QRectF &newRect);
		void setTransformedRect(const QRectF&);
		
		//QGraphicsItem's virtual functions
		virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
		virtual QRectF boundingRect() const;
		virtual QPainterPath shape() const;
		
		QRectF rect;
		QRectF transformedRect;
		
		PlotArea::BackgroundType backgroundType;
		PlotArea::BackgroundColorStyle backgroundColorStyle;
		PlotArea::BackgroundImageStyle backgroundImageStyle;
		QBrush backgroundBrush;
		QColor backgroundFirstColor;
		QColor backgroundSecondColor;
		QString backgroundFileName;
		qreal backgroundOpacity;
		QPen borderPen;
		qreal borderOpacity;
		
		PlotArea * const q;
};

#endif


