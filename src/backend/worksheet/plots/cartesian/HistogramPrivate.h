/*
	File                 : HistogramPrivate.h
	Project              : LabPlot
	Description          : Private members of Histogram
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
	SPDX-FileCopyrightText: 2018-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HISTOGRAMPRIVATE_H
#define HISTOGRAMPRIVATE_H

#include "backend/worksheet/plots/cartesian/PlotPrivate.h"
#include <gsl/gsl_histogram.h>
#include <vector>

class Column;
class Background;
class Line;
class Value;

class HistogramPrivate : public PlotPrivate {
public:
	explicit HistogramPrivate(Histogram* owner);
	~HistogramPrivate() override;

	void retransform() override;
	void recalc();
	void updateType();
	void updateOrientation();
	void updateLines();
	void verticalHistogram();
	void horizontalHistogram();
	void updateSymbols();
	void updateValues();
	void updateFilling();
	void updateErrorBars();
	void updateRug();
	void updatePixmap();
	void recalcShapeAndBoundingRect() override;

	double xMinimum() const;
	double xMaximum() const;
	double yMinimum() const;
	double yMaximum() const;

	const AbstractColumn* bins();
	const AbstractColumn* binValues();
	const AbstractColumn* binPDValues();

	double getMaximumOccuranceofHistogram() const;

	// General
	const AbstractColumn* dataColumn{nullptr};
	QString dataColumnPath;
	Histogram::Type type{Histogram::Ordinary};
	Histogram::Orientation orientation{Histogram::Orientation::Vertical};
	Histogram::Normalization normalization{Histogram::Count};
	Histogram::BinningMethod binningMethod{Histogram::SquareRoot};
	int totalCount{0};
	int binCount{10};
	double binWidth{1.0};
	bool autoBinRanges{true};
	double binRangesMin{0.0};
	double binRangesMax{1.0};

	Line* line{nullptr};
	Symbol* symbol{nullptr};
	Background* background{nullptr};
	Value* value{nullptr};
	ErrorBar* errorBar{nullptr};

	// rug
	bool rugEnabled{false};
	double rugOffset{0.0};
	double rugLength{Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point)};
	double rugWidth{0.0};
	QPainterPath rugPath;

	QPainterPath linePath;
	QPainterPath symbolsPath;
	QPainterPath valuesPath;
	QPainterPath errorBarsPath;
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
	Column* m_binsColumn{nullptr}; // bin positions/edges
	Column* m_binValuesColumn{nullptr}; // bin values
	Column* m_binPDValuesColumn{nullptr}; // bin values in the probability density normalization
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void histogramValue(double& value, int bin) const;
	void draw(QPainter*);
};

#endif
