/***************************************************************************
    File                 : WorksheetElementContainer.h
    Project              : LabPlot
    Description          : Worksheet element container - parent of multiple elements.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2017 Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef WORKSHEETELEMENTCONTAINER_H
#define WORKSHEETELEMENTCONTAINER_H

#include "backend/worksheet/WorksheetElement.h"

class WorksheetElementContainerPrivate;

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

	QRectF rect() const;
	virtual void setRect(const QRectF&) = 0;
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

protected slots:
	virtual void handleAspectAdded(const AbstractAspect*);

private:
	Q_DECLARE_PRIVATE(WorksheetElementContainer)

signals:
	friend class WorksheetElementContainerSetVisibleCmd;
	void visibleChanged(bool);
};

#endif
