/*
    File                 : PlotAreaPrivate.h
    Project              : LabPlot
    Description          : Private members of PlotArea.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2011 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef PLOTAREAPRIVATE_H
#define PLOTAREAPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"
#include <QPen>

class QBrush;
class PlotArea;

class PlotAreaPrivate: public WorksheetElementPrivate {
public:
	explicit PlotAreaPrivate(PlotArea *owner);

	bool toggleClipping(bool on);
	bool clippingEnabled() const;
	void setRect(const QRectF&);
	void retransform() override {};

	//QGraphicsItem's virtual functions
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	virtual void recalcShapeAndBoundingRect() override {};

	QRectF rect;
	WorksheetElement::BackgroundType backgroundType;
	WorksheetElement::BackgroundColorStyle backgroundColorStyle;
	WorksheetElement::BackgroundImageStyle backgroundImageStyle;
	Qt::BrushStyle backgroundBrushStyle;
	QColor backgroundFirstColor;
	QColor backgroundSecondColor;
	QString backgroundFileName;
	qreal backgroundOpacity;

	PlotArea::BorderType borderType;
	QPen borderPen;
	qreal borderOpacity;
	qreal borderCornerRadius;

	PlotArea* const q;
};

#endif
