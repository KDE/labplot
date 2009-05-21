/***************************************************************************
    File                 : AbstractCurveSymbol.h
    Project              : LabPlot/SciDAVis
    Description          : Abstract base class for curve symbols
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

#ifndef ABSTRACTCURVESYMBOL_H
#define ABSTRACTCURVESYMBOL_H

#include <QGraphicsItem>
#include <QPen>
#include <QBrush>

class AbstractCurveSymbol  {
	public:
		AbstractCurveSymbol();
		virtual ~AbstractCurveSymbol();

		virtual QString id() const = 0;
		virtual void setSize(qreal size) = 0;
		virtual qreal size() const = 0;
		virtual void setAspectRatio(qreal aspectRatio) = 0;
		virtual qreal aspectRatio() const = 0;
		virtual void setRotationAngle(qreal angle) = 0;
		virtual qreal rotationAngle() const = 0;
		virtual void setBrush (const QBrush &brush) = 0;
		virtual QBrush brush() const = 0;
		virtual void setPen(const QPen &pen) = 0;
		virtual QPen pen() const = 0;
		virtual AbstractCurveSymbol *clone() const = 0;

		virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=0) = 0;
		virtual QRectF boundingRect() const = 0;
		virtual QPainterPath shape() const = 0;
};

#endif


