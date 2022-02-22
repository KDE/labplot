/*
    File                 : BoxPlotPrivate.h
    Project              : LabPlot
    Description          : Box Plot - private implementation
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef REFERENCELINEPRIVATE_H
#define REFERENCELINEPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"
#include <QPen>

class CartesianCoordinateSystem;

typedef QVector<QPointF> Points;

class BoxPlotPrivate: public WorksheetElementPrivate {
public:
	explicit BoxPlotPrivate(BoxPlot*);

	void retransform() override;
	void recalc();
	virtual void recalcShapeAndBoundingRect() override;
	void updatePixmap();

	bool m_suppressRecalc{false};

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	bool activateCurve(QPointF mouseScenePos, double maxDist);
	void setHover(bool on);

	BoxPlot* const q;

	//General
	QVector<const AbstractColumn*> dataColumns;
	QVector<const AbstractColumn*> dataColumnsOrdered;
	QVector<QString> dataColumnPaths;
	BoxPlot::Orientation orientation{BoxPlot::Orientation::Vertical};
	BoxPlot::Ordering ordering{BoxPlot::Ordering::None};
	bool variableWidth{false};
	double widthFactor{1.0};
	bool notchesEnabled{false};
	qreal opacity{1.0};

	double xMin;
	double xMax;
	double yMin;
	double yMax;

	//box filling
	bool fillingEnabled{true};
	WorksheetElement::BackgroundType fillingType;
	WorksheetElement::BackgroundColorStyle fillingColorStyle;
	WorksheetElement::BackgroundImageStyle fillingImageStyle;
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
	Symbol* symbolMedian{nullptr};
	Symbol* symbolOutlier{nullptr};
	Symbol* symbolFarOut{nullptr};
	Symbol* symbolData{nullptr};
	bool jitteringEnabled{true};

	//whiskers
	BoxPlot::WhiskersType whiskersType{BoxPlot::WhiskersType::IQR};
	double whiskersRangeParameter; //Tukey's parameter k controlling the range of the whiskers, usually k=1.5
	QPen whiskersPen;
	qreal whiskersOpacity;
	double whiskersCapSize;
	QPen whiskersCapPen;
	qreal whiskersCapOpacity;

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
	void updateFillingRect(int index, const QVector<QLineF>&);

	void draw(QPainter*);
	void drawFilling(QPainter*, int);
	void drawSymbols(QPainter*, int);

	bool m_hovered{false};
	bool m_suppressRetransform{false};

	QRectF m_boundingRectangle;
	QPainterPath m_boxPlotShape;

	QVector<QVector<QLineF>> m_boxRect; //QVector<QLineF> contains four lines that are clipped on the plot rectangle
	QVector<QRectF> m_fillRect; //rectangles used for the filling (clipped versions of the boxes)
	double m_widthScaleFactor{1.0};
	QVector<double> m_xMinBox;
	QVector<double> m_xMaxBox;
	QVector<double> m_yMinBox;
	QVector<double> m_yMaxBox;
	QVector<double> m_median;
	QVector<QLineF> m_medianLine;
	QVector<double> m_mean;
	QVector<QPainterPath> m_whiskersPath;
	QVector<QPainterPath> m_whiskersCapPath;
	QVector<double> m_whiskerMin;
	QVector<double> m_whiskerMax;
	QVector<Points> m_outlierPointsLogical;	//positions of the outlier symbols in logical coordinates
	QVector<Points> m_outlierPoints;	//positions of the outlier symbols in scene coordinates
	Points m_meanSymbolPoint; //position of the mean symbol in scene coordinates
	QVector<bool> m_meanSymbolPointVisible; //true/false if the mean point is visible in the plot or not
	Points m_medianSymbolPoint; //position of the median symbol in scene coordinates
	QVector<bool> m_medianSymbolPointVisible; //true/false if the median point is visible in the plot or not
	QVector<Points> m_dataPointsLogical;	//positions of the data points in logical coordinates
	QVector<Points> m_dataPoints;	//positions of the data points in scene coordinates
	QVector<Points> m_farOutPointsLogical;	//positions of the far out values in logical coordinates
	QVector<Points> m_farOutPoints;	//positions of the far out values in scene coordinates

	QPixmap m_pixmap;
	QImage m_hoverEffectImage;
	QImage m_selectionEffectImage;

	bool m_hoverEffectImageIsDirty{false};
	bool m_selectionEffectImageIsDirty{false};
};

#endif
