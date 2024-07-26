/*
	File                 : WorksheetElementContainerPrivate.h
	Project              : LabPlot
	Description          : Private members of WorksheetElementContainer.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2012-2024 Alexander Semke <alexander.semke@web.de>
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
	~WorksheetElementContainerPrivate() override = default;

	QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void prepareGeometryChangeRequested();
	void recalcShapeAndBoundingRect() override;
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	virtual void retransform() override;

	WorksheetElementContainer* q{nullptr};
	QRectF rect;
	bool m_printing{false};
	bool suppressChanged{false};

Q_SIGNALS:
	void selectedChange(QGraphicsItem*);
};

#endif
