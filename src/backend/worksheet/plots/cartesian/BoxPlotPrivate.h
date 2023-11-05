/*
	File                 : BoxPlotPrivate.h
	Project              : LabPlot
	Description          : Box Plot - private implementation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BOXPLOTPRIVATE_H
#define BOXPLOTPRIVATE_H

#include "backend/worksheet/plots/cartesian/PlotPrivate.h"
#include <QPen>

class Background;
class CartesianCoordinateSystem;
class Spreadsheet;
class KConfigGroup;

typedef QVector<QPointF> Points;

class BoxPlotPrivate : public PlotPrivate {
public:
	explicit BoxPlotPrivate(BoxPlot*);

	void retransform() override;
	void recalc();
	virtual void recalcShapeAndBoundingRect() override;
	void updateRug();
	void updatePixmap();
	void fillDataSpreadsheet(Spreadsheet*) const;

	Background* addBackground(const KConfigGroup&);
	Line* addBorderLine(const KConfigGroup&);
	Line* addMedianLine(const KConfigGroup&);
	void adjustPropertiesContainers();
	void setHover(bool on);

	BoxPlot* const q;

	// General
	QVector<const AbstractColumn*> dataColumns;
	QVector<const AbstractColumn*> dataColumnsOrdered;
	QVector<QString> dataColumnPaths;
	BoxPlot::Orientation orientation{BoxPlot::Orientation::Vertical};
	BoxPlot::Ordering ordering{BoxPlot::Ordering::None};
	bool variableWidth{false};
	double widthFactor{1.0};
	bool notchesEnabled{false};
	qreal opacity{1.0};

	double xMin{0.5};
	double xMax{1.5};
	double yMin{0.5};
	double yMax{1.5};

	// box
	QVector<Background*> backgrounds;
	QVector<Line*> borderLines;
	QVector<Line*> medianLines;

	// markers
	Symbol* symbolMean{nullptr};
	Symbol* symbolMedian{nullptr};
	Symbol* symbolOutlier{nullptr};
	Symbol* symbolFarOut{nullptr};
	Symbol* symbolData{nullptr};
	Symbol* symbolWhiskerEnd{nullptr};
	bool jitteringEnabled{true};

	// whiskers
	BoxPlot::WhiskersType whiskersType{BoxPlot::WhiskersType::IQR};
	double whiskersRangeParameter{1.5}; // Tukey's parameter k controlling the range of the whiskers, usually k=1.5
	Line* whiskersLine{nullptr};
	double whiskersCapSize{Worksheet::convertToSceneUnits(5.0, Worksheet::Unit::Point)};
	Line* whiskersCapLine{nullptr};

	// rug
	bool rugEnabled{false};
	double rugOffset{0.0};
	double rugLength{Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point)};
	double rugWidth{0.0};
	QPainterPath rugPath;

private:
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void recalc(int);
	void verticalBoxPlot(int);
	void horizontalBoxPlot(int);
	QPointF setOutlierPoint(double pos, double value);
	void mapSymbolsToScene(int index);
	void updateFillingRect(int index, const QVector<QLineF>&);

	void draw(QPainter*);
	void drawSymbols(QPainter*, int);

	QVector<QVector<QLineF>> m_boxRect; // QVector<QLineF> contains four lines that are clipped on the plot rectangle
	QVector<QPolygonF> m_fillPolygon; // polygons used for the filling (clipped versions of the boxes)
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
	QVector<QPainterPath> m_rugPath;
	QVector<double> m_whiskerMin;
	QVector<double> m_whiskerMax;

	// vectors to store the information required to draw the different symbols
	QVector<Points> m_whiskerEndPointsLogical; // positions of the whisker end values in logical coordinates
	QVector<Points> m_whiskerEndPoints; // positions of the whisker end values in scene coordinates
	QVector<Points> m_outlierPointsLogical; // positions of the outlier symbols in logical coordinates
	QVector<Points> m_outlierPoints; // positions of the outlier symbols in scene coordinates
	Points m_meanPointLogical; // position of the mean symbol in logical coordinates
	Points m_meanPoint; // position of the mean symbol in scene coordinates
	QVector<bool> m_meanPointVisible; // true/false if the mean point is visible in the plot or not
	Points m_medianPointLogical; // position of the median symbol in logical coordinates
	Points m_medianPoint; // position of the median symbol in scene coordinates
	QVector<bool> m_medianPointVisible; // true/false if the median point is visible in the plot or not
	QVector<Points> m_dataPointsLogical; // positions of the data points in logical coordinates
	QVector<Points> m_dataPoints; // positions of the data points in scene coordinates
	QVector<Points> m_farOutPointsLogical; // positions of the far out values in logical coordinates
	QVector<Points> m_farOutPoints; // positions of the far out values in scene coordinates
};

#endif
