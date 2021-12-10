/*
    File                 : WorksheetElementContainer.h
    Project              : LabPlot
    Description          : Worksheet element container - parent of multiple elements.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2012-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef WORKSHEETELEMENTCONTAINER_H
#define WORKSHEETELEMENTCONTAINER_H

#include "backend/worksheet/WorksheetElement.h"

class WorksheetElementContainerPrivate;
class ResizeItem;

class WorksheetElementContainer : public WorksheetElement {
	Q_OBJECT

public:
	WorksheetElementContainer(const QString&, AspectType);
	~WorksheetElementContainer() override;

	QGraphicsItem* graphicsItem() const override;

	void setVisible(bool) override;
	bool isVisible() const override;
	bool isFullyVisible() const override;
	void setPrinting(bool) override;
	void setResizeEnabled(bool);

	QRectF rect() const;
	virtual void setRect(const QRectF&, bool forceUpdate = false) = 0;
	virtual void prepareGeometryChange();
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;

	typedef WorksheetElementContainerPrivate Private;

public slots:
	void retransform() override;
	void childHovered();
	void childUnhovered();

protected:
	WorksheetElementContainerPrivate* const d_ptr;
	WorksheetElementContainer(const QString&, WorksheetElementContainerPrivate*, AspectType);
	ResizeItem* m_resizeItem{nullptr};

protected slots:
	virtual void handleAspectAdded(const AbstractAspect*);

private:
	Q_DECLARE_PRIVATE(WorksheetElementContainer)

signals:
	friend class WorksheetElementContainerSetVisibleCmd;
	void visibleChanged(bool);
};

#endif
