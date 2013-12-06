/***************************************************************************
    File                 : AbstractCurveSymbol.cpp
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

#include "backend/worksheet/AbstractCurveSymbol.h"

/**
 * \class AbstractCurveSymbol
 * \brief Abstract base class for curve symbols.
 *
 * 
 */

AbstractCurveSymbol::AbstractCurveSymbol() {
}

AbstractCurveSymbol::~AbstractCurveSymbol() {
}

/**
 * \fn QString AbstractCurveSymbol::getId() const;
 * \brief Get the ID string of the symbol.
 */
/**
 * \fn void AbstractCurveSymbol::setSize(qreal size);
 * \brief Set the symbol size.
 */
/**
 * \fn qreal AbstractCurveSymbol::size() const;
 * \brief Get the symbol size.
 */
/**
 * \fn void AbstractCurveSymbol::setAspectRatio(qreal aspectRatio);
 * \brief Set the symbol aspect ratio (horizontal/vertical size).
 */
/**
 * \fn qreal AbstractCurveSymbol::aspectRatio() const;
 * \brief Get the symbol aspect ratio (horizontal/vertical size).
 */
/**
 * \fn void AbstractCurveSymbol::setRotationAngle(qreal angle);
 * \brief Set the rotation angle in degrees.
 */
/**
 * \fn qreal AbstractCurveSymbol::rotationAngle() const;
 * \brief Get the rotation angle in degrees.
 */
/**
 * \fn void AbstractCurveSymbol::setBrush (const QBrush &brush);
 * \brief Set the background brush.
 *
 * Use QBrush(Qt::NoBrush) to deactivate symbol filling.
 */
/**
 * \fn QBrush AbstractCurveSymbol::brush() const;
 * \brief Get the background brush.
 */
/**
 * \fn void AbstractCurveSymbol::setPen(const QPen &pen);
 * \brief Set the border pen.
 */
/**
 * \fn QPen AbstractCurveSymbol::pen() const;
 * \brief Get the border pen.
 */
/**
 * \fn AbstractCurveSymbol *clone() const;
 * \brief Get a copy of the symbol.
 */

/**
  \fn void AbstractCurveSymbol::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=0) = 0;
  \brief same as QGraphicsItem::paint()
 */
/**
  \fn QRectF AbstractCurveSymbol::boundingRect() const = 0;
  \brief same as QGraphicsItem::boundingRect()
 */
/**
  \fn QPainterPath AbstractCurveSymbol::shape() const = 0;
  \brief same as QGraphicsItem::shape()
 */

