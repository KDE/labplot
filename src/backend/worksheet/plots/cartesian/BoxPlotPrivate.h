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

typedef QVector<QPointF> Points;

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
	bool notchesEnabled{false};
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
	Symbol* symbolMean{nullptr};
	Symbol* symbolOutlier{nullptr};
	Symbol* symbolFarOut{nullptr};
	Symbol* symbolJitter{nullptr};

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
	QPointF setOutlierPoint(double pos, double value);
	void mapOutliersToScene(int index);

	void draw(QPainter*);
	void drawFilling(QPainter*, int);
	void drawSymbols(QPainter*, int);

	bool m_hovered{false};
	bool m_suppressRetransform{false};

	QRectF m_boundingRectangle;
	QPainterPath m_boxPlotShape;

	QVector<QVector<QLineF>> m_boxRect; //QVector<QLineF> contains four lines that are clipped on the plot rectangle
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
	QVector<Points> m_outlierPointsLogical;	//positions of the outlier symbols in logical coordinates
	QVector<Points> m_outlierPoints;	//positions of the outlier symbols in scene coordinates
	Points m_meanSymbolPoint; //position of the mean symbol in scene coordinates
	QVector<bool> m_meanSymbolPointVisible; //true/false if the mean point is visible in the plot or not
	QVector<Points> m_jitterPointsLogical;	//positions of the jitters in logical coordinates
	QVector<Points> m_jitterPoints;	//positions of the jitters in scene coordinates
	QVector<Points> m_farOutPointsLogical;	//positions of the far out values in logical coordinates
	QVector<Points> m_farOutPoints;	//positions of the far out values in scene coordinates

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
