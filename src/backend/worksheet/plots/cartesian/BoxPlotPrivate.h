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
	QPen medianLinePen;
	qreal medianLineOpacity;

	//markers
	Symbol::Style symbolOutliersStyle;
	Symbol::Style symbolMeanStyle;
	QBrush symbolsBrush;
	QPen symbolsPen;
	qreal symbolsOpacity;
	qreal symbolsRotationAngle;
	qreal symbolsSize;

	//whiskers
	QPen whiskersPen;
	double whiskersCapSize;
	qreal whiskersOpacity;

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void recalcVertical();
	void verticalBoxPlot();
	void recalcHorizontal();
	void horizontalBoxPlot();

	void draw(QPainter*);
	void drawFilling(QPainter*);
	void drawSymbols(QPainter*);

	bool n_suppressItemChangeEvent{false};
	bool m_suppressRetransform{false};
	bool m_suppressRecalc{false};

	QRectF m_boundingRectangle;
	QPainterPath m_boxPlotShape;

	QRectF m_boxRect;
	double m_xMinBox;
	double m_xMaxBox;
	double m_yMinBox;
	double m_yMaxBox;
	double m_median;
	QLineF m_medianLine;
	QPainterPath m_whiskersPath;
	double m_whiskerMin;
	double m_whiskerMax;
	QVector<QPointF> m_outliersSymbolPointsLogical;	//positions of the outlier symbols in logical coordinates
	QVector<QPointF> m_outliersSymbolPoints;	//positions of the outlier symbols in scene coordinates
	int m_outliersCount; //total number of outliers. this number is different to the size of the vector m_outliersSymbolPoints containing unique points only
	QPointF m_meanSymbolPoint; //position of the mean symbol in scene coordinates

	double m_xMin;
	double m_xMax;
	double m_yMin;
	double m_yMax;

	QPixmap m_pixmap;
	QImage m_hoverEffectImage;
	QImage m_selectionEffectImage;

	bool m_hoverEffectImageIsDirty{false};
	bool m_selectionEffectImageIsDirty{false};
};

#endif
