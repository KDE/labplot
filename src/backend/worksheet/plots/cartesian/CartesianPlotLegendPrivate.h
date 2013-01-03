/***************************************************************************
    File                 : CartesianPlotLegendPrivate.h
    Project              : LabPlot/SciDAVis
    Description          : Private members of CartesianPlotLegend.
    --------------------------------------------------------------------
    Copyright            : (C) 2013 by Alexander Semke (alexander.semke*web.de)
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

#ifndef CARTESIANPLOTLEGENDPRIVATE_H
#define CARTESIANPLOTLEGENDPRIVATE_H

#include <QGraphicsItem>
#include <QBrush>
#include <QPen>
#include <QFont>

class CartesianPlotLegend;

class CartesianPlotLegendPrivate : public QGraphicsItem {
	public:
		CartesianPlotLegendPrivate(CartesianPlotLegend* owner);

		CartesianPlotLegend* const q;

		QString name() const;
		void update();
		bool swapVisible(bool on);

		void recalcShapeAndBoundingRect();
		void retransform();

		//QGraphicsItem's virtual functions
		virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
		virtual QRectF boundingRect() const;
		virtual QPainterPath shape() const;

		QRectF rect;
		QFont labelFont;
		QColor labelColor;
		float lineSymbolWidth;

		//Background
		PlotArea::BackgroundType backgroundType;
		PlotArea::BackgroundColorStyle backgroundColorStyle;
		PlotArea::BackgroundImageStyle backgroundImageStyle;
		Qt::BrushStyle backgroundBrushStyle;
		QColor backgroundFirstColor;
		QColor backgroundSecondColor;
		QString backgroundFileName;
		float backgroundOpacity;

		//Border
		QPen borderPen;
		qreal borderOpacity;

		//Layout
		float layoutTopMargin;
		float layoutBottomMargin;
		float layoutLeftMargin;
		float layoutRightMargin;
		float layoutVerticalSpacing;
		float layoutHorizontalSpacing;
		int layoutColumnCount;
};

#endif
