/***************************************************************************
    File                 : WorksheetElementContainer.h
    Project              : LabPlot/SciDAVis
    Description          : Generic WorksheetElement container.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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
class AbstractCoordinateSystem;

class WorksheetElementContainer: public AbstractWorksheetElement {
	Q_OBJECT

	public:
		WorksheetElementContainer(const QString &name);
		~WorksheetElementContainer();

		virtual QList<QGraphicsItem *> graphicsItems() const;

		virtual void setZValue(qreal z);
		virtual qreal zValue() const;
		virtual void setZValueRange(qreal minZ, qreal maxZ);
		virtual qreal zValueMin() const ;
		virtual qreal zValueMax() const ;

    	virtual QRectF boundingRect() const;
		virtual bool contains(const QPointF &position) const;

		virtual void setVisible(bool on);
		virtual bool isVisible() const;
		virtual bool isFullVisible() const;

		virtual void retransform() const;
};

#endif

