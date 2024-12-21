/*
	File                 : XYCurvePrivate.h
	Project              : LabPlot
	Description          : Private members of XYCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2013-2020 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYCURVEPRIVATE_H
#define XYCURVEPRIVATE_H

#include "backend/worksheet/plots/cartesian/PlotPrivate.h"
#include <vector>

class Background;
class CartesianPlot;
class CartesianCoordinateSystem;
class Symbol;
class XYCurve;

class XYCurvePrivate : public PlotPrivate {
public:
	explicit XYCurvePrivate(XYCurve*);

	void retransform() override;
	void recalc();
	void updateLines(bool performanceOptimization = true);
	void addLine(QPointF p,
				 double& x,
				 double& minY,
				 double& maxY,
				 QPointF& lastPoint,
				 int& pixelDiff,
				 int numberOfPixelX,
				 double minDiffX,
				 RangeT::Scale scale,
				 bool& prevPixelDiffZero,
				 bool performanceOptimization); // for any x scale
	static void addUniqueLine(QPointF p,
							  double& minY,
							  double& maxY,
							  QPointF& lastPoint,
							  int& pixelDiff,
							  QVector<QLineF>& lines,
							  bool& prevPixelDiffZero); // finally add line if unique (no overlay)
	void updateDropLines();
	void updateSymbols();
	void updateRug();
	void updateValues();
	void updateFilling();
	void updateErrorBars();
	void recalcShapeAndBoundingRect() override;
	void updatePixmap();

	virtual bool activatePlot(QPointF mouseScenePos, double maxDist = -1) override;
	bool pointLiesNearLine(const QPointF p1, const QPointF p2, const QPointF pos, const double maxDist) const;
	bool
	pointLiesNearCurve(const QPointF mouseScenePos, const QPointF curvePosPrevScene, const QPointF curvePosScene, const int index, const double maxDist) const;

	// data source
	const AbstractColumn* xColumn{nullptr};
	const AbstractColumn* yColumn{nullptr};
	QString dataSourceCurvePath;
	QString xColumnPath;
	QString yColumnPath;
	bool sourceDataChangedSinceLastRecalc{false};

	Plot::PlotType plotType{Plot::PlotType::Line};

	// line
	XYCurve::LineType lineType{XYCurve::LineType::Line};
	bool lineSkipGaps{false};
	bool lineIncreasingXOnly{false};
	int lineInterpolationPointsCount{1};
	Line* line{nullptr};
	Line* dropLine{nullptr};

	// symbols
	Symbol* symbol{nullptr};

	// rug
	bool rugEnabled{false};
	WorksheetElement::Orientation rugOrientation{WorksheetElement::Orientation::Vertical};
	double rugOffset{0.0};
	double rugLength{Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point)};
	double rugWidth{0.0};
	QPainterPath rugPath;

	// values
	XYCurve::ValuesType valuesType{XYCurve::ValuesType::NoValues};
	const AbstractColumn* valuesColumn{nullptr};
	QString valuesColumnPath;
	XYCurve::ValuesPosition valuesPosition{XYCurve::ValuesPosition::Above};
	qreal valuesDistance{Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point)};
	qreal valuesRotationAngle{0.0};
	qreal valuesOpacity{1.0};
	char valuesNumericFormat{'f'}; //'g', 'e', 'E', etc. for numeric values
	int valuesPrecision{2}; // number of digits for numeric values
	QString valuesDateTimeFormat;
	QString valuesPrefix;
	QString valuesSuffix;
	QFont valuesFont;
	QColor valuesColor;

	// filling
	Background* background{nullptr};

	// error bars
	ErrorBar* errorBar{nullptr};

	XYCurve* const q;
	friend class XYCurve;

	//	CartesianPlot* plot{nullptr};
	//	const CartesianCoordinateSystem* cSystem{nullptr};	//current cSystem

private:
	CartesianPlot* plot() const {
		return m_plot;
	} // convenience method
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void drawValues(QPainter*);
	void draw(QPainter*);
	void calculateScenePoints();

	// TODO: add m_
	QPainterPath linePath;
	QPainterPath dropLinePath;
	QPainterPath valuesPath;
	QPainterPath errorBarsPath;
	QPainterPath symbolsPath;
	QVector<QLineF> m_lines;
	QVector<QPointF> m_logicalPoints; // points in logical coordinates
	QVector<QPointF> m_scenePoints; // points in scene coordinates
	bool m_scenePointsDirty{true}; // true whenever the scenepoints have to be recalculated before using
	std::vector<bool> m_pointVisible; // if point is currently visible in plot (size of m_logicalPoints)
	QVector<QPointF> m_valuePoints; // points for showing value
	QVector<QString> m_valueStrings; // strings for showing value
	QVector<QPolygonF> m_fillPolygons; // polygons for filling
	// TODO: QVector, rename, usage
	std::vector<int> validPointsIndicesLogical; // original indices in the source columns for valid and non-masked values (size of m_logicalPoints)
	std::vector<bool> connectedPointsLogical; // true for points connected with the consecutive point (size of m_logicalPoints)

	QPointF mousePos;

	friend class RetransformTest;
	friend class XYCurveTest;
	friend class XYFunctionCurveTest;
};

#endif
