/***************************************************************************
    File                 : PathCurveSymbol.h
    Project              : LabPlot/SciDAVis
    Description          : A standard curve symbol defined by a painter path and a string id.
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

#ifndef PATHCURVESYMBOL_H
#define PATHCURVESYMBOL_H

#include <QtGlobal>
#include "worksheet/AbstractCurveSymbol.h"

class PathCurveSymbolPrivate;
class PathCurveSymbol: public QObject, public AbstractCurveSymbol  {
	Q_OBJECT

	public:
		PathCurveSymbol(const QString& symbolId);
		virtual ~PathCurveSymbol();

		virtual QString id() const;
		virtual void setSize(qreal size);
		virtual qreal size() const;
		virtual void setAspectRatio(qreal aspectRatio);
		virtual qreal aspectRatio() const;
		virtual void setRotationAngle(qreal angle);
		virtual qreal rotationAngle() const;
		virtual void setBrush (const QBrush &brush);
		virtual QBrush brush() const;
		virtual void setPen(const QPen &pen);
		virtual QPen pen() const;
		virtual void setPath(const QPainterPath &path);
		virtual QPainterPath path() const;
		virtual bool fillingEnabled() const;
		virtual void setFillingEnabled(bool);
		
		virtual AbstractCurveSymbol *clone() const;

		virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=0);
		virtual void paint(QPainter *painter);
		virtual QRectF boundingRect() const;
		virtual QPainterPath shape() const;

	protected:
		PathCurveSymbolPrivate * const d_ptr;
		PathCurveSymbol(PathCurveSymbolPrivate *dd);
		
	private:
    	Q_DECLARE_PRIVATE(PathCurveSymbol)
};

#endif

