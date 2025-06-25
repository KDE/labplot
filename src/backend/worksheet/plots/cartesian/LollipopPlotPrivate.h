/*
	File                 : LollipopPrivate.h
	Project              : LabPlot
	Description          : Lollipop Plot - private implementation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BARPLOTPRIVATE_H
#define BARPLOTPRIVATE_H

#include "backend/worksheet/plots/cartesian/PlotPrivate.h"
#include <QPen>

class CartesianCoordinateSystem;
class Line;
class Symbol;
class Value;
class KConfigGroup;

typedef QVector<QPointF> Points;

class LollipopPlotPrivate : public PlotPrivate {
public:
	explicit LollipopPlotPrivate(LollipopPlot*);

	void retransform() override;
	void recalc();
	virtual void recalcShapeAndBoundingRect() override;
	void updateValues();
	void updatePixmap();

	Line* addLine(const KConfigGroup&);
	Symbol* addSymbol(const KConfigGroup&);
	void addValue(const KConfigGroup&);

	LollipopPlot* const q;

	// General
	const AbstractColumn* xColumn{nullptr};
	QString xColumnPath;
	QVector<const AbstractColumn*> dataColumns;
	QVector<QString> dataColumnPaths;
	LollipopPlot::Orientation orientation{LollipopPlot::Orientation::Vertical};
	qreal opacity{1.0};

	double xMin{0.};
	double xMax{1.};
	double yMin{0.};
	double yMax{1.};

	QVector<Line*> lines;
	QVector<Symbol*> symbols;
	Value* value{nullptr};

private:
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void recalc(int);
	void verticalPlot(int);
	void horizontalPlot(int);

	void draw(QPainter*);

	QVector<QVector<QLineF>> m_barLines; // QVector<QLineF> contains the lines for each data column
	QVector<QVector<QPointF>> m_symbolPoints; // QVector<QPointF> contains the positions of symbols for each data column

	QVector<QPointF> m_valuesPoints;
	QVector<QPointF> m_valuesPointsLogical;
	QVector<QString> m_valuesStrings;
	QPainterPath m_valuesPath;

	double m_groupWidth{1.0}; // width of a bar group
	double m_groupGap{0.0}; // gap around a group of bars
};

#endif
