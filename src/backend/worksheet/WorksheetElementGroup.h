/***************************************************************************
    File                 : WorksheetElementGroup.h
    Project              : LabPlot/SciDAVis
    Description          : Generic WorksheetElement container
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

#ifndef WORKSHEETELEMENTGROUP_H
#define WORKSHEETELEMENTGROUP_H

#include "worksheet/AbstractWorksheetElement.h"

class WorksheetElementGroup: public AbstractWorksheetElement {
	Q_OBJECT

	public:
		WorksheetElementGroup(const QString &name);
		~WorksheetElementGroup();

		virtual QList<QGraphicsItem *> graphicsItems() const;

		virtual void setZValue(qreal z);
		virtual qreal zValue() const;
		virtual void setZValueRange(qreal minZ, qreal maxZ);
		virtual qreal zValueMin() const ;
		virtual qreal zValueMax() const ;

		virtual void setXScale(qreal xScale, bool keepAspectRatio=false);
		virtual void setYScale(qreal yScale, bool keepAspectRatio=false);
		virtual qreal xScale() const;
		virtual qreal yScale() const;

    	virtual void setRotationAngle(qreal angle);
		virtual qreal rotationAngle() const;

	    virtual void setPosition(const QPointF &position);
    	virtual QPointF position() const;

    	virtual QRectF boundingRect() const;
		virtual bool contains(const QPointF &position) const;

		virtual void setVisible(bool on);
		virtual bool isVisible() const;
		virtual bool isFullVisible() const;
};

#endif

