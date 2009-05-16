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
#include "worksheet/AbstractStandardCurveSymbolPrivate.h"
#include <QPainter>

/**
 * \class EllipseCurveSymbol
 * \brief Elliptic curve symbol.
 *
 * 
 */

class EllipseCurveSymbolPrivate: public AbstractStandardCurveSymbolPrivate {
};
 
EllipseCurveSymbol::EllipseCurveSymbol()
    : AbstractStandardCurveSymbol(new EllipseCurveSymbolPrivate) {
}
 
EllipseCurveSymbol::EllipseCurveSymbol(EllipseCurveSymbolPrivate *dd)
    : AbstractStandardCurveSymbol(dd) {
}
 
EllipseCurveSymbol::~EllipseCurveSymbol() {
}
 
EllipseCurveSymbol *EllipseCurveSymbol::m_staticPrototype = new EllipseCurveSymbol();

const EllipseCurveSymbol *EllipseCurveSymbol::staticPrototype() {
	return m_staticPrototype;
}

QRectF EllipseCurveSymbol::boundingRect () const
{
    Q_D(const EllipseCurveSymbol);

	// TODO: support for rotation

    qreal penWidth = d->pen.widthF();
	qreal xSize = d->size ;
	qreal ySize = d->size / d->aspectRatio;

	return QRectF((- xSize - penWidth) / 2, (-ySize - penWidth) / 2,
			xSize + penWidth, ySize + penWidth);
}

void EllipseCurveSymbol::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                 QWidget *widget) {
    Q_D(EllipseCurveSymbol);
    Q_UNUSED(widget);
    Q_UNUSED(option);

    painter->setPen(d->pen);
    painter->setBrush(d->brush);

	qreal xSize = d->size ;
	qreal ySize = d->size / d->aspectRatio;
	QRectF rect(-xSize / 2, -ySize / 2, xSize, ySize);

    painter->drawEllipse(rect);
}
		
AbstractCurveSymbol *EllipseCurveSymbol::clone() const
{
    Q_D(const EllipseCurveSymbol);

	EllipseCurveSymbol *twin = new EllipseCurveSymbol();
	twin->d_ptr->cloneHelper(d);
	return twin;
}
