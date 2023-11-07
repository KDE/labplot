/*
	File                 : ScaledTextItem.h
	Project              : LabPlot
	Description          : Supporter widget for texts
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCALEDTEXTITEM_H
#define SCALEDTEXTITEM_H

#include "backend/lib/macros.h"

#include <QPainter>

class ScaledTextItem : public QGraphicsTextItem {
public:
	explicit ScaledTextItem(QGraphicsItem* parent = nullptr)
		: QGraphicsTextItem(parent) {
	}

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
#if DEBUG_TEXTLABEL_BOUNDING_RECT
		painter->setPen(QColor(Qt::GlobalColor::green));
		painter->drawRect(boundingRect());
#endif

#if DEBUG_TEXTLABEL_BOUNDING_RECT
		painter->setPen(QColor(Qt::GlobalColor::black));
		painter->drawRect(QRectF(-5, -5, 10, 10));
#endif
		QGraphicsTextItem::paint(painter, option, widget);
	}

private:
};

#endif // SCALEDTEXTITEM_H
