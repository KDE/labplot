/***************************************************************************
    File                 : BoxPlotPrivate.h
    Project              : LabPlot
    Description          : Box Plot - private implementation
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
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


#ifndef REFERENCELINEPRIVATE_H
#define REFERENCELINEPRIVATE_H

#include <QGraphicsItem>
#include <QPen>

class CartesianCoordinateSystem;

class BoxPlotPrivate: public QGraphicsItem {
public:
	explicit BoxPlotPrivate(BoxPlot*);

	QString name() const;
	void retransform();
	void recalc();
	bool swapVisible(bool);
	virtual void recalcShapeAndBoundingRect();
	void updatePixmap();

	double xMinimum();
	double xMaximum();
	double yMinimum();
	double yMaximum();

	bool m_hovered{false};
	bool m_visible{true}; //point inside the plot (visible) or not

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	BoxPlot* const q;

	//General
	const AbstractColumn* dataColumn{nullptr};
	QString dataColumnPath;
	BoxPlot::WhiskersType whiskersType{BoxPlot::WhiskersType::MinMax};
	BoxPlot::Orientation orientation{BoxPlot::Orientation::Vertical};
	qreal opacity{1.0};

	//box filling
	QRectF boxRect;
	bool fillingEnabled{true};
	PlotArea::BackgroundType fillingType;
	PlotArea::BackgroundColorStyle fillingColorStyle;
	PlotArea::BackgroundImageStyle fillingImageStyle;
	Qt::BrushStyle fillingBrushStyle;
	QColor fillingFirstColor;
	QColor fillingSecondColor;
	QString fillingFileName;
	qreal fillingOpacity;

	//box border
	QPen borderPen;
	qreal borderOpacity;

	//median line
	QLineF medianLine;
	QPen medianLinePen;
	qreal medianLineOpacity;

	//whiskers
	QPainterPath whiskersPath;
	QPen whiskersPen;
	double whiskersCapSize;
	qreal whiskersOpacity;

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void verticalBoxPlot();
	void horizontalBoxPlot();

	void draw(QPainter*);
	void drawBox(QPainter*);

	bool n_suppressItemChangeEvent{false};
	bool m_suppressRetransform{false};
	bool m_suppressRecalc{false};

	QRectF m_boundingRectangle;
	QPainterPath m_boxPlotShape;
	QPixmap m_pixmap;
	QImage m_hoverEffectImage;
	QImage m_selectionEffectImage;

	bool m_hoverEffectImageIsDirty{false};
	bool m_selectionEffectImageIsDirty{false};

	double m_xMin;
	double m_xMax;
	double m_yMin;
	double m_yMax;
};

#endif
