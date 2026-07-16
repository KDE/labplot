/*
	File                 : PlotAreaBackground.h
	Project              : LabPlot
	Description          : Graphics item for drawing plot area background and border
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLOTAREABACKGROUND_H
#define PLOTAREABACKGROUND_H

#include <QGraphicsItem>

class Background;
class Line;
class CartesianPlot;

class PlotAreaBackgroundPrivate : public QGraphicsItem {
public:
	explicit PlotAreaBackgroundPrivate(CartesianPlot*);

	QRectF boundingRect() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void setRect(const QRectF&);
	void retransform();

	CartesianPlot* q;
	QRectF rect;
};

#endif // PLOTAREABACKGROUND_H
