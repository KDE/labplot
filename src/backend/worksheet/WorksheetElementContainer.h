/***************************************************************************
    File                 : WorksheetElementContainer.h
    Project              : LabPlot/SciDAVis
    Description          : Generic WorksheetElement container.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
	Copyright            : (C) 2012 by Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses)
                           
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

#include "worksheet/AbstractWorksheetElement.h"

class WorksheetElementContainerPrivate;

class WorksheetElementContainer: public AbstractWorksheetElement {
	Q_OBJECT

	public:
		WorksheetElementContainer(const QString &name);
		virtual ~WorksheetElementContainer();

		virtual QGraphicsItem *graphicsItem() const;

		virtual void setVisible(bool on);
		virtual bool isVisible() const;
		virtual bool isFullyVisible() const;

		QRectF rect() const;
		virtual void setRect(const QRectF&) = 0;

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);
	
	protected:
		WorksheetElementContainerPrivate * const d_ptr;
		WorksheetElementContainer(const QString &name, WorksheetElementContainerPrivate *dd);

	protected slots:
		virtual void handleAspectAdded(const AbstractAspect *handleAspect);
		virtual void handleAspectAboutToBeRemoved(const AbstractAspect *handleAspect);
		void childSelected();
		void childDeselected();

	private:
    	Q_DECLARE_PRIVATE(WorksheetElementContainer)
};

#endif
