/***************************************************************************
    File                 : AbstractStandardCurveSymbol.cpp
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

#include "worksheet/AbstractStandardCurveSymbol.h"
#include "worksheet/AbstractStandardCurveSymbolPrivate.h"

/**
 * \class AbstractStandardCurveSymbol
 * \brief Implements several methods used by many curve symbols.
 *
 * This class does not implement undo commands. These should be
 * implemented by the owner of the symbol prototype.
 */

AbstractStandardCurveSymbol::AbstractStandardCurveSymbol()
    : QObject(0), d_ptr(new AbstractStandardCurveSymbolPrivate()) {
}
		
AbstractStandardCurveSymbol::AbstractStandardCurveSymbol(AbstractStandardCurveSymbolPrivate *dd)
    : QObject(0), d_ptr(dd) {
}

AbstractStandardCurveSymbolPrivate::AbstractStandardCurveSymbolPrivate() {
	size = 1;
	aspectRatio = 1;
	rotationAngle = 0;
}

AbstractStandardCurveSymbolPrivate::~AbstractStandardCurveSymbolPrivate() {
}

void AbstractStandardCurveSymbol::setSize(qreal size) { 
	Q_D(AbstractStandardCurveSymbol);

	if (size != d->size) {
		prepareGeometryChange();
		d->size = size;
	}
}

qreal AbstractStandardCurveSymbol::size() const {
	Q_D(const AbstractStandardCurveSymbol);

	return d->size;
}

void AbstractStandardCurveSymbol::setAspectRatio(qreal aspectRatio) {
	Q_D(AbstractStandardCurveSymbol);

	if (aspectRatio != d->aspectRatio) {
		prepareGeometryChange();
		d->aspectRatio = aspectRatio;
	}
}

qreal AbstractStandardCurveSymbol::aspectRatio() const {
	Q_D(const AbstractStandardCurveSymbol);

	return d->aspectRatio;
}

void AbstractStandardCurveSymbol::setRotationAngle(qreal angle) {
	Q_D(AbstractStandardCurveSymbol);

	if (angle != d->rotationAngle) {
		prepareGeometryChange();
		d->rotationAngle = angle;
	}
}

qreal AbstractStandardCurveSymbol::rotationAngle() const {
	Q_D(const AbstractStandardCurveSymbol);

	return d->rotationAngle;
}

void AbstractStandardCurveSymbol::setBrush (const QBrush &brush) {
	Q_D(AbstractStandardCurveSymbol);

	d->brush = brush;
}

QBrush AbstractStandardCurveSymbol::brush() const {
	Q_D(const AbstractStandardCurveSymbol);

	return d->brush;
}

void AbstractStandardCurveSymbol::setPen(const QPen &pen) {
	Q_D(AbstractStandardCurveSymbol);

	d->pen = pen;
}

QPen AbstractStandardCurveSymbol::pen() const {
	Q_D(const AbstractStandardCurveSymbol);

	return d->pen;
}
		
void AbstractStandardCurveSymbolPrivate::cloneHelper(const AbstractStandardCurveSymbolPrivate *other) {
	size = other->size;
	aspectRatio = other->aspectRatio;
	rotationAngle = other->rotationAngle;
	brush = other->brush;
	pen = other->pen;
}

