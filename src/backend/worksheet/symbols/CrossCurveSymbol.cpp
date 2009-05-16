/***************************************************************************
    File                 : CrossCurveSymbol.cpp
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

#include "worksheet/symbols/CrossCurveSymbol.h"
#include "worksheet/AbstractStandardCurveSymbolPrivate.h"
#include <QPainter>

/**
 * \class CrossCurveSymbol
 * \brief Cross-shaped curve symbol.
 *
 * 
 */

class CrossCurveSymbolPrivate: public AbstractStandardCurveSymbolPrivate {
};
 
CrossCurveSymbol::CrossCurveSymbol()
    : AbstractStandardCurveSymbol(new CrossCurveSymbolPrivate) {
}
 
CrossCurveSymbol::CrossCurveSymbol(CrossCurveSymbolPrivate *dd)
    : AbstractStandardCurveSymbol(dd) {
}
 
CrossCurveSymbol::~CrossCurveSymbol() {
}
 
QRectF CrossCurveSymbol::boundingRect () const
{
    Q_D(const CrossCurveSymbol);

	// TODO: support for rotation

    qreal penWidth = d->pen.widthF();
	qreal xSize = d->size ;
	qreal ySize = d->size / d->aspectRatio;

	return QRectF((- xSize - penWidth) / 2, (-ySize - penWidth) / 2,
			xSize + penWidth, ySize + penWidth);
}

void CrossCurveSymbol::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                 QWidget *widget) {
    Q_D(CrossCurveSymbol);
    Q_UNUSED(widget);
    Q_UNUSED(option);

    painter->setPen(d->pen);
    painter->setBrush(d->brush);

	qreal xSize = d->size ;
	qreal ySize = d->size / d->aspectRatio;
	QRectF rect(-xSize / 2, -ySize / 2, xSize, ySize);

	QLineF line1(0, -ySize / 2, 0, ySize / 2);
 	painter->drawLine(line1);
	QLineF line2(-xSize / 2, 0, xSize / 2, 0);
 	painter->drawLine(line2);
}
		
AbstractCurveSymbol *CrossCurveSymbol::clone() const
{
    Q_D(const CrossCurveSymbol);

	CrossCurveSymbol *twin = new CrossCurveSymbol();
	twin->d_ptr->cloneHelper(d);
	return twin;
}

