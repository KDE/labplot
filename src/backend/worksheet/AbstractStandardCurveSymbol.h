/***************************************************************************
    File                 : AbstractStandardCurveSymbol.h
    Project              : LabPlot/SciDAVis
    Description          : Implements several methods used by many curve symbols.
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

#ifndef ABSTRACTSTANDARDCURVESYMBOL_H
#define ABSTRACTSTANDARDCURVESYMBOL_H

#include <QtGlobal>
#include "worksheet/AbstractCurveSymbol.h"

class AbstractStandardCurveSymbolPrivate;
class AbstractStandardCurveSymbol: public QObject, public AbstractCurveSymbol  {
	Q_OBJECT

	public:
		AbstractStandardCurveSymbol();
		virtual ~AbstractStandardCurveSymbol() {}

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
	protected:
		AbstractStandardCurveSymbolPrivate * const d_ptr;
		AbstractStandardCurveSymbol(AbstractStandardCurveSymbolPrivate *dd);
		
	private:
    	Q_DECLARE_PRIVATE(AbstractStandardCurveSymbol)
};

#endif

