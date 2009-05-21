/***************************************************************************
    File                 : AbstractWorksheetElement.h
    Project              : LabPlot/SciDAVis
    Description          : Base class for all Worksheet children.
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

#ifndef ABSTRACTWORKSHEETELEMENT_H
#define ABSTRACTWORKSHEETELEMENT_H

#include "core/AbstractAspect.h"
#include <QGraphicsItem>
class AbstractCoordinateSystem;

class AbstractWorksheetElement: public AbstractAspect {
	Q_OBJECT

	public:
		AbstractWorksheetElement(const QString &name);
		virtual ~AbstractWorksheetElement();

		virtual QGraphicsItem *graphicsItem() const = 0;

		virtual void setZValue(qreal z) = 0;
		virtual qreal zValue() const = 0;

		virtual void setVisible(bool on) = 0;
		virtual bool isVisible() const = 0;
		virtual bool isFullyVisible() const;

		virtual AbstractCoordinateSystem *coordinateSystem() const;

		static QPainterPath shapeFromPath(const QPainterPath &path, const QPen &pen);

	public slots:
		virtual void retransform() = 0;
};

#endif

