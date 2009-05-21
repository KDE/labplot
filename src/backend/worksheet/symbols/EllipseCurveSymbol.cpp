/***************************************************************************
    File                 : EllipseCurveSymbol.cpp
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

#include "worksheet/symbols/EllipseCurveSymbol.h"
#include "worksheet/symbols/PathCurveSymbolPrivate.h"
#include <QPainter>

/**
 * \class EllipseCurveSymbol
 * \brief Elliptic curve symbol.
 *
 * 
 */

EllipseCurveSymbol::EllipseCurveSymbol()
    : PathCurveSymbol(new PathCurveSymbolPrivate("ellipse")) {
	QPainterPath path;
	path.addEllipse(QRectF(-0.5, -0.5, 1, 1));
	setPath(path);
}
 
EllipseCurveSymbol::~EllipseCurveSymbol() {
}
 
EllipseCurveSymbol *EllipseCurveSymbol::m_staticPrototype = new EllipseCurveSymbol();

const EllipseCurveSymbol *EllipseCurveSymbol::staticPrototype() {
	return m_staticPrototype;
}

