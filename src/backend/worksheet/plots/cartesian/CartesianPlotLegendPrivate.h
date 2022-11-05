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
#include <QPen>

class Background;
class CartesianPlotLegend;
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
	bool labelColumnMajor;
	float lineSymbolWidth; // the width of line+symbol
	QList<float> maxColumnTextWidths; // the maximal width of the text within each column
	int columnCount; // the actual number of columns, can be smaller then the specified layoutColumnCount
	int rowCount; // the number of rows in the legend, depends on the number of curves and on columnCount

	const CartesianPlot* plot{nullptr};

	TextLabel* title{nullptr};
	Background* background{nullptr};

	// Border
	QPen borderPen;
	qreal borderCornerRadius;
	qreal borderOpacity;

	// Layout
	float layoutTopMargin;
	float layoutBottomMargin;
	float layoutLeftMargin;
	float layoutRightMargin;
	float layoutVerticalSpacing;
	float layoutHorizontalSpacing;
	int layoutColumnCount;

private:
	QList<WorksheetElement*> m_curves; // list containing all visible curves
	QStringList m_names;

	bool translatePainter(QPainter*, int& row, int& col, int height);

	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};

#endif
