/***************************************************************************
    File                 : BoxPlotPrivate.h
    Project              : LabPlot
    Description          : Box Plot - private implementation
    --------------------------------------------------------------------
	Copyright            : (C) 2021 Alexander Semke (alexander.semke@web.de)
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

	bool m_suppressRecalc{false};

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	BoxPlot* const q;

	//General
	QVector<const AbstractColumn*> dataColumns;
	QVector<QString> dataColumnPaths;
	BoxPlot::WhiskersType whiskersType{BoxPlot::WhiskersType::IQR};
	BoxPlot::Orientation orientation{BoxPlot::Orientation::Vertical};
	bool variableWidth{false};
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

	void recalc(int);
	void verticalBoxPlot(int);
	void horizontalBoxPlot(int);
	void setOutlierPoint(QPointF&, double pos, double value);
	void mapOutliersToScene(int index);

	void draw(QPainter*);
	void drawFilling(QPainter*, int);
	void drawSymbols(QPainter*, int);

	bool m_hovered{false};
	bool m_suppressRetransform{false};

	QRectF m_boundingRectangle;
	QPainterPath m_boxPlotShape;

	QVector<QRectF> m_boxRect;
	double m_widthScaleFactor{1.0};
	QVector<double> m_xMinBox;
	QVector<double> m_xMaxBox;
	QVector<double> m_yMinBox;
	QVector<double> m_yMaxBox;
	QVector<double> m_median;
	QVector<QLineF> m_medianLine;
	QVector<double> m_mean;
	QVector<QPainterPath> m_whiskersPath;
	QVector<double> m_whiskerMin;
	QVector<double> m_whiskerMax;
	QVector<QVector<QPointF>> m_outliersSymbolPointsLogical;	//positions of the outlier symbols in logical coordinates
	QVector<QVector<QPointF>> m_outliersSymbolPoints;	//positions of the outlier symbols in scene coordinates
	QVector<int> m_outliersCount; //total number of outliers. this number is different to the size of the vector m_outliersSymbolPoints containing unique points only
	QVector<QPointF> m_meanSymbolPoint; //position of the mean symbol in scene coordinates

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
