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
class QGraphicsSceneContextMenuEvent;

class CartesianPlotLegendPrivate : public QGraphicsItem {
	public:
		explicit CartesianPlotLegendPrivate(CartesianPlotLegend* owner);

		CartesianPlotLegend* const q;

		QString name() const;
		bool swapVisible(bool on);
		void retransform();
		void updatePosition();

		//QGraphicsItem's virtual functions
		virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
		virtual QRectF boundingRect() const;
		virtual QPainterPath shape() const;
		virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);		

		bool suppressItemChangeEvent;
		bool suppressRetransform;
		bool m_printing;

		QRectF rect;
		QFont labelFont;
		QColor labelColor;
		bool labelColumnMajor;
		CartesianPlotLegend::PositionWrapper position; //position in parent's coordinate system
		float lineSymbolWidth; //the width of line+symbol
		QList<float> maxColumnTextWidths; //the maximal width of the text within each column
		int columnCount; //the actual number of columns, can be smaller then the specified layoutColumnCount
		int rowCount; //the number of rows in the legend, depends on the number of curves and on columnCount
		TextLabel* title;

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

	private:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent*);
};

#endif
