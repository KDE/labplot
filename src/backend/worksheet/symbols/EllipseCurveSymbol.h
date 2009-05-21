/***************************************************************************
    File                 : EllipseCurveSymbol.h
    Project              : LabPlot/SciDAVis
    Description          : Elliptic curve symbol.
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

#ifndef ELLIPSECURVESYMBOL_H
#define ELLIPSECURVESYMBOL_H

#include "worksheet/symbols/PathCurveSymbol.h"

class EllipseCurveSymbol: public PathCurveSymbol  {
	Q_OBJECT

	public:
		EllipseCurveSymbol();
		virtual ~EllipseCurveSymbol();

		static const EllipseCurveSymbol *staticPrototype();

	private:
		static EllipseCurveSymbol *m_staticPrototype;
};

#endif


