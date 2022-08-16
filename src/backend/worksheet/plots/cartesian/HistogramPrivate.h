/*
	File                 : HistogramPrivate.h
	Project              : LabPlot
	Description          : Private members of Histogram
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
	SPDX-FileCopyrightText: 2018-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HISTOGRAMPRIVATE_H
#define HISTOGRAMPRIVATE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include "backend/worksheet/WorksheetElementPrivate.h"
#include <vector>

class Column;
class Background;

extern "C" {
#include <gsl/gsl_histogram.h>
}

class HistogramPrivate : public WorksheetElementPrivate {
public:
	explicit HistogramPrivate(Histogram* owner);
	~HistogramPrivate() override;

	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	void retransform() override;
	void recalcHistogram();
	void updateType();
	void updateOrientation();
	void updateLines();
	void verticalHistogram();
	void horizontalHistogram();
	void updateSymbols();
	void updateValues();
	void updateFilling();
	void updateErrorBars();
	void updatePixmap();
	void recalcShapeAndBoundingRect() override;

	void setHover(bool on);
	bool activateCurve(QPointF mouseScenePos, double maxDist);

	double xMinimum() const;
	double xMaximum() const;
	double yMinimum() const;
	double yMaximum() const;

	const AbstractColumn* bins();
	const AbstractColumn* binValues();
	const AbstractColumn* binPDValues();

	double getMaximumOccuranceofHistogram() const;

	bool m_suppressRecalc{false};

	// General
	const AbstractColumn* dataColumn{nullptr};
	QString dataColumnPath;
	Histogram::HistogramType type{Histogram::Ordinary};
	Histogram::HistogramOrientation orientation{Histogram::Vertical};
	Histogram::HistogramNormalization normalization{Histogram::Count};
	Histogram::BinningMethod binningMethod{Histogram::SquareRoot};
	int totalCount{0};
	int binCount{10};
	float binWidth{1.0f};
	bool autoBinRanges{true};
	double binRangesMin{0.0};
	double binRangesMax{1.0};

	// line
	Histogram::LineType lineType{Histogram::Bars};
	QPen linePen;
	qreal lineOpacity;

	// symbols
	Symbol* symbol{nullptr};

	// values
	int value{0};
	Histogram::ValuesType valuesType{Histogram::NoValues};
	const AbstractColumn* valuesColumn{nullptr};
	QString valuesColumnPath;
	Histogram::ValuesPosition valuesPosition{Histogram::ValuesAbove};
	qreal valuesDistance;
	qreal valuesRotationAngle;
	qreal valuesOpacity;
	char valuesNumericFormat{'f'}; // 'f', 'g', 'e', 'E', etc. for numeric values
	int valuesPrecision{2}; // number of digits for numeric values
	QString valuesDateTimeFormat;
	QString valuesPrefix;
	QString valuesSuffix;
	QFont valuesFont;
	QColor valuesColor;

	// filling
	Background* background{nullptr};

	// error bars
	Histogram::ErrorType errorType{Histogram::NoError};
	XYCurve::ErrorBarsType errorBarsType;
	double errorBarsCapSize{1};
	QPen errorBarsPen;
	qreal errorBarsOpacity;

	QPainterPath linePath;
	QPainterPath symbolsPath;
	QPainterPath valuesPath;
	QRectF boundingRectangle;
	QPainterPath curveShape;
	// TODO: use Qt container
	// TODO: add m_
	QVector<QLineF> lines;
	QVector<QLineF> linesUnclipped;
	QVector<QPointF> pointsLogical; // points in logical coordinates
	QVector<QPointF> pointsScene; // points in scene coordinates
	std::vector<bool> visiblePoints; // vector of the size of symbolPointsLogical with true of false for the points currently visible or not in the plot
	QVector<QPointF> valuesPoints;
	QVector<QString> valuesStrings;
	QPolygonF fillPolygon;

	Histogram* const q;

private:
	gsl_histogram* m_histogram{nullptr};
	size_t m_bins{0};

	//	bool m_printing{false};
	bool m_hovered{false};
	bool m_suppressRetransform{false};
	QPixmap m_pixmap;
	QImage m_hoverEffectImage;
	QImage m_selectionEffectImage;
	bool m_hoverEffectImageIsDirty{false};
	bool m_selectionEffectImageIsDirty{false};
	Column* m_binsColumn{nullptr}; // bin positions/edges
	Column* m_binValuesColumn{nullptr}; // bin values
	Column* m_binPDValuesColumn{nullptr}; // bin values in the probability density normalization

	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void histogramValue(double& value, int bin) const;
	void drawSymbols(QPainter*);
	void drawValues(QPainter*);
	void drawFilling(QPainter*);
	void draw(QPainter*);
};

#endif
