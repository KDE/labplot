/*
	File                 : BarPlotPrivate.h
	Project              : LabPlot
	Description          : Bar Plot - private implementation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BARPLOTPRIVATE_H
#define BARPLOTPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"
#include <QPen>

class CartesianCoordinateSystem;

typedef QVector<QPointF> Points;

class BarPlotPrivate : public WorksheetElementPrivate {
public:
	explicit BarPlotPrivate(BarPlot*);

	void retransform() override;
	void recalc();
	virtual void recalcShapeAndBoundingRect() override;
	void updatePixmap();

	bool m_suppressRecalc{false};

	// reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	bool activateCurve(QPointF mouseScenePos, double maxDist);
	void setHover(bool on);

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

	double xMin;
	double xMax;
	double yMin;
	double yMax;

	// box filling
	bool fillingEnabled{true};
	WorksheetElement::BackgroundType fillingType;
	WorksheetElement::BackgroundColorStyle fillingColorStyle;
	WorksheetElement::BackgroundImageStyle fillingImageStyle;
	Qt::BrushStyle fillingBrushStyle;
	QColor fillingFirstColor;
	QColor fillingSecondColor;
	QString fillingFileName;
	qreal fillingOpacity;

	// box border
	QPen borderPen;
	qreal borderOpacity;

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void recalc(int);
	void verticalBarPlot(int);
	void horizontalBarPlot(int);
	void updateFillingRect(int columnIndex, int valueIndex, const QVector<QLineF>&);

	void draw(QPainter*);
	void drawFilling(QPainter*, int columnIndex, int valueIndex);

	bool m_hovered{false};
	bool m_suppressRetransform{false};

	QRectF m_boundingRectangle;
	QPainterPath m_barPlotShape;

	QVector<QVector<QVector<QLineF>>> m_barLines; // QVector<QLineF> contains four lines that are clipped on the plot rectangle
	QVector<QVector<QPolygonF>> m_fillPolygons; // polygons used for the filling (clipped versions of the boxes)
	QVector<double> m_stackedBarOffsets; // offsets for the y-positions for stacked bar plots
	double m_widthScaleFactor{1.0};

	QPixmap m_pixmap;
	QImage m_hoverEffectImage;
	QImage m_selectionEffectImage;

	bool m_hoverEffectImageIsDirty{false};
	bool m_selectionEffectImageIsDirty{false};
};

#endif
