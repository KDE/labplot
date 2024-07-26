/*
	File                 : ReferenceRangePrivate.h
	Project              : LabPlot
	Description          : Reference range on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REFERENCERANGEPRIVATE_H
#define REFERENCERANGEPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"

class CartesianCoordinateSystem;

class ReferenceRangePrivate : public WorksheetElementPrivate {
public:
	explicit ReferenceRangePrivate(ReferenceRange*);

	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void updateOrientation();
	void updatePositionLimit();

	bool m_visible{true}; // point inside the plot (visible) or not

	ReferenceRange::Orientation orientation{ReferenceRange::Orientation::Horizontal};
	QPointF positionLogicalStart;
	QPointF positionLogicalEnd;
	QRectF rect;

	Line* line{nullptr};
	Background* background{nullptr};

	// reimplemented from QGraphicsItem
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	QPointF recalculateRect();

	ReferenceRange* const q;

private:
	bool m_bottomClipped{false};
	bool m_topClipped{false};
	bool m_leftClipped{false};
	bool m_rightClipped{false};
};

#endif
