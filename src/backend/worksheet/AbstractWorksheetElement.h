/***************************************************************************
    File                 : AbstractWorksheetElement.h
    Project              : LabPlot/SciDAVis
    Description          : Base class for basically all children of a Worksheet object
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

class AbstractWorksheetElement: public AbstractAspect {
	Q_OBJECT

	public:
		AbstractWorksheetElement(const QString &name): AbstractAspect(name) {}
		virtual ~AbstractWorksheetElement() {};

		virtual const QGraphicsItem *graphicsItem() const = 0; // TODO: maybe this should return QList<const QGraphicsItem *> instead

		virtual void setZValue(qreal z) = 0;
		virtual qreal zValue () const = 0;

		virtual void setXScale(qreal xScale, bool keepAspectRatio=false) = 0;
		virtual void setYScale(qreal yScale, bool keepAspectRatio=false) = 0;
		virtual qreal xScale() const = 0;
		virtual qreal yScale() const = 0;

    	virtual void setRotationAngle(qreal angle) = 0;
		virtual qreal rotationAngle() const = 0;

	    virtual void setPosition(const QPointF &position) = 0;
    	virtual QPointF position() const = 0;

    	virtual QRectF boundingRect() const = 0;
		virtual bool contains(const QPointF &position) const = 0;

		virtual void setVisible(bool on) = 0;
		virtual bool isVisible() const = 0;
};

#endif

