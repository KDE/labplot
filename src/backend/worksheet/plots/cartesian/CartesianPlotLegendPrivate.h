/***************************************************************************
    File                 : CartesianPlotLegendPrivate.h
    Project              : LabPlot
    Description          : Private members of CartesianPlotLegend
    --------------------------------------------------------------------
    Copyright            : (C) 2013-2018 by Alexander Semke (alexander.semke@web.de)
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
#include <QPen>
#include <QFont>

class QBrush;
class CartesianPlotLegend;
class XYCurve;
class QGraphicsSceneContextMenuEvent;
class QKeyEvent;

class CartesianPlotLegendPrivate : public QGraphicsItem {
public:
	explicit CartesianPlotLegendPrivate(CartesianPlotLegend* owner);

	CartesianPlotLegend* const q;

	QString name() const;
	bool swapVisible(bool on);
	void retransform();
	void updatePosition();

	//QGraphicsItem's virtual functions
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

	bool suppressItemChangeEvent{false};
	bool suppressRetransform{false};
	bool m_printing{false};
	bool m_hovered{false};

	QList<WorksheetElement*> curvesList; //list containing all visible curves
	QRectF rect;
	QFont labelFont;
	QColor labelColor;
	bool labelColumnMajor;
	WorksheetElement::PositionWrapper position; //position in parent's coordinate system
	qreal rotationAngle;
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
	qreal borderCornerRadius;
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
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
};

#endif
