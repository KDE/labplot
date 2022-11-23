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
class Line;
class Value;

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
	void updateRug();
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
	double binWidth{1.0};
	bool autoBinRanges{true};
	double binRangesMin{0.0};
	double binRangesMax{1.0};

	Line* line{nullptr};
	Symbol* symbol{nullptr};
	Background* background{nullptr};
	Value* value{nullptr};

	// error bars
	Histogram::ErrorType errorType{Histogram::NoError};
	const AbstractColumn* errorPlusColumn{nullptr};
	QString errorPlusColumnPath;
	const AbstractColumn* errorMinusColumn{nullptr};
	QString errorMinusColumnPath;
	Line* errorBarsLine{nullptr};

	// rug
	bool rugEnabled{false};
	double rugOffset;
	double rugLength;
	double rugWidth;
	QPainterPath rugPath;

	QPainterPath linePath;
	QPainterPath symbolsPath;
	QPainterPath valuesPath;
	QPainterPath errorBarsPath;
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
	void drawFilling(QPainter*);
	void draw(QPainter*);
};

#endif
