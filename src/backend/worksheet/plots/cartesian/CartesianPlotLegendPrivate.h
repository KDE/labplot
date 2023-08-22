/*
	File                 : CartesianPlotLegendPrivate.h
	Project              : LabPlot
	Description          : Private members of CartesianPlotLegend
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2013-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANPLOTLEGENDPRIVATE_H
#define CARTESIANPLOTLEGENDPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"
#include <QFont>

class Background;
class CartesianPlotLegend;
class Line;
class XYCurve;

class QBrush;
class QGraphicsSceneContextMenuEvent;
class QKeyEvent;

class CartesianPlotLegendPrivate : public WorksheetElementPrivate {
public:
	explicit CartesianPlotLegendPrivate(CartesianPlotLegend* owner);

	CartesianPlotLegend* const q;

	void retransform() override;
	void updatePosition();

	// QGraphicsItem's virtual functions
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	virtual void recalcShapeAndBoundingRect() override;

	bool m_hovered{false};

	QRectF rect;
	QFont labelFont;
	QColor labelColor;
	bool labelColumnMajor{true};
	qreal lineSymbolWidth{Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter)}; // the width of line+symbol
	QList<double> maxColumnTextWidths; // the maximal width of the text within each column
	int columnCount{0}; // the actual number of columns, can be smaller than the specified layoutColumnCount
	int rowCount{0}; // the number of rows in the legend, depends on the number of curves and on columnCount

	const CartesianPlot* plot{nullptr};

	TextLabel* title{nullptr};
	Background* background{nullptr};

	// Border
	Line* borderLine{nullptr};
	qreal borderCornerRadius{0.0};

	// Layout
	qreal layoutTopMargin{Worksheet::convertToSceneUnits(0.2, Worksheet::Unit::Centimeter)};
	qreal layoutBottomMargin{Worksheet::convertToSceneUnits(0.2, Worksheet::Unit::Centimeter)};
	qreal layoutLeftMargin{Worksheet::convertToSceneUnits(0.2, Worksheet::Unit::Centimeter)};
	qreal layoutRightMargin{Worksheet::convertToSceneUnits(0.2, Worksheet::Unit::Centimeter)};
	qreal layoutVerticalSpacing{Worksheet::convertToSceneUnits(0.1, Worksheet::Unit::Centimeter)};
	qreal layoutHorizontalSpacing{Worksheet::convertToSceneUnits(0.1, Worksheet::Unit::Centimeter)};
	int layoutColumnCount{1};

private:
	QList<WorksheetElement*> m_curves; // list containing all visible curves
	QStringList m_names;

	bool translatePainter(QPainter*, int& row, int& col, int height);

	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};

#endif
