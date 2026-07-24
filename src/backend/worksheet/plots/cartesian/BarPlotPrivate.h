/*
	File                 : BarPlotPrivate.h
	Project              : LabPlot
	Description          : Bar Plot - private implementation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BARPLOTPRIVATE_H
#define BARPLOTPRIVATE_H

#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/PlotPrivate.h"
#include <QPen>

class Background;
class ErrorBar;
class Line;
class CartesianCoordinateSystem;
class Value;
class KConfigGroup;

typedef QVector<QPointF> Points;

class BarPlotPrivate : public PlotPrivate {
public:
	explicit BarPlotPrivate(BarPlot*);

	void retransform() override;
	void recalc();
	virtual void recalcShapeAndBoundingRect() override;
	void updateValues();
	void updatePixmap();

	Background* addBackground(const KConfigGroup&);
	Line* addBorderLine(const KConfigGroup&);
	void addValue(const KConfigGroup&);
	ErrorBar* addErrorBar(const KConfigGroup&);

	BarPlot* const q;

	// General
	const AbstractColumn* xColumn{nullptr};
	QString xColumnPath;
	QVector<const AbstractColumn*> dataColumns;
	QVector<QString> dataColumnPaths;
	BarPlot::Type type{BarPlot::Type::Grouped};
	BarPlot::Orientation orientation{BarPlot::Orientation::Vertical};
	double widthFactor{1.0};
	qreal opacity{1.0};

	double xMin{0.};
	double xMax{1.};
	double yMin{0.};
	double yMax{1.};

	QVector<Background*> backgrounds;
	QVector<Line*> borderLines;
	QVector<ErrorBar*> errorBars;
	Value* value{nullptr};

private:
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void recalc(int);
	void verticalBarPlot(int);
	void horizontalBarPlot(int);
	void updateErrorBars(int);
	void updateFillingRect(int columnIndex, int valueIndex, const QVector<QLineF>&);

	void draw(QPainter*);

	QVector<QPointF> m_valuesPoints; // positions of values in scene coordinates for all columns
	QVector<QVector<QPointF>> m_valuesPointsLogical; // QVector<QPointF> contains the points in logical coordinates for the value positions for one data column
	QVector<QString> m_valuesStrings;
	QPainterPath m_valuesPath;
	QVector<QPainterPath> m_errorBarsPaths;

	QVector<QVector<QVector<QLineF>>> m_barLines; // QVector<QLineF> contains four lines that are clipped on the plot rectangle
	QVector<QVector<QPolygonF>> m_fillPolygons; // polygons used for the filling (clipped versions of the boxes)
	QVector<double> m_stackedBarPositiveOffsets; // offsets for the y-positions for stacked bar plots, positive direction
	QVector<double> m_stackedBarNegativeOffsets; // offsets for the y-positions for stacked bar plots, negative direction
	QVector<double> m_stackedBar100PercentValues; // total sum of values in a stacked bar group defining the 100% value
	double m_widthScaleFactor{1.0};
	double m_groupWidth{1.0}; // width of a bar group
	double m_groupGap{0.0}; // gap around a group of bars
	double m_zero{0.0}; // zero baseline used to draw the bars, has non-zero values for log-scales
};

#endif
