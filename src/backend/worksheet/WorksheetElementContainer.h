/*
	File                 : WorksheetElementContainer.h
	Project              : LabPlot
	Description          : Worksheet element container - parent of multiple elements.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2012-2024 Alexander Semke <alexander.semke@web.de>
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

	void setVisible(bool) override;
	bool isFullyVisible() const override;
	void setPrinting(bool) override;
	void setResizeEnabled(bool);

	QRectF rect() const;
	virtual void setRect(const QRectF&) = 0;
	virtual void setPrevRect(const QRectF&) = 0;
	virtual void prepareGeometryChange();
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;

	typedef WorksheetElementContainerPrivate Private;

public Q_SLOTS:
	virtual void retransform() override;

private Q_SLOTS:
	void childHovered();
	void childUnhovered();
	void childChanged();

protected:
	WorksheetElementContainer(const QString&, WorksheetElementContainerPrivate*, AspectType);
	ResizeItem* m_resizeItem{nullptr};

protected Q_SLOTS:
	virtual void handleAspectAdded(const AbstractAspect*);
	virtual void handleAspectMoved();

private:
	Q_DECLARE_PRIVATE(WorksheetElementContainer)

Q_SIGNALS:
	friend class WorksheetElementContainerSetVisibleCmd;
};

#endif
