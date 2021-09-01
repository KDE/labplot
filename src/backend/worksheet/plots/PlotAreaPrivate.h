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

#include <QGraphicsItem>
#include <QPen>

class QBrush;
class PlotArea;

class PlotAreaPrivate: public QGraphicsItem {
public:
	explicit PlotAreaPrivate(PlotArea *owner);

	QString name() const;
	bool swapVisible(bool on);
	bool toggleClipping(bool on);
	bool clippingEnabled() const;
	void setRect(const QRectF&);

	//QGraphicsItem's virtual functions
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	QRectF rect;
	PlotArea::BackgroundType backgroundType;
	PlotArea::BackgroundColorStyle backgroundColorStyle;
	PlotArea::BackgroundImageStyle backgroundImageStyle;
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
