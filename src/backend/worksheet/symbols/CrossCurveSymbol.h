/***************************************************************************
    File                 : CrossCurveSymbol.h
    Project              : LabPlot/SciDAVis
    Description          : Cross-shaped curve symbol.
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

#ifndef CROSSCURVESYMBOL_H
#define CROSSCURVESYMBOL_H

#include "worksheet/AbstractStandardCurveSymbol.h"

class CrossCurveSymbolPrivate;
class CrossCurveSymbol: public AbstractStandardCurveSymbol  {
	Q_OBJECT

	public:
		CrossCurveSymbol();
		virtual ~CrossCurveSymbol();

		QString id() const { return "cross"; }
		virtual AbstractCurveSymbol *clone() const;
    	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0);
		QRectF boundingRect () const;

	protected:
		CrossCurveSymbol(CrossCurveSymbolPrivate *dd);
		
	private:
    	Q_DECLARE_PRIVATE(CrossCurveSymbol)
};

#endif



