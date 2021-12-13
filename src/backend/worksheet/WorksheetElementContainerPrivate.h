/*
    File                 : WorksheetElementContainerPrivate.h
    Project              : LabPlot
    Description          : Private members of WorksheetElementContainer.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2012-2015 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef WORKSHEETELEMENTCONTAINERPRIVATE_H
#define WORKSHEETELEMENTCONTAINERPRIVATE_H

#include "WorksheetElementPrivate.h"

class QGraphicsSceneContextMenuEvent;
class WorksheetElementContainer;

class WorksheetElementContainerPrivate : public WorksheetElementPrivate {

public:
	explicit WorksheetElementContainerPrivate(WorksheetElementContainer*);
	~WorksheetElementContainerPrivate() override= default;

	QRectF boundingRect() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;

	void prepareGeometryChangeRequested();
	void recalcShapeAndBoundingRect();
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	virtual void retransform() override;

	WorksheetElementContainer* q{nullptr};
	QRectF boundingRectangle;
	QPainterPath containerShape;
	QRectF rect;
	bool m_hovered{false};
	bool m_printing{false};

signals:
	void selectedChange(QGraphicsItem*);
};

#endif
