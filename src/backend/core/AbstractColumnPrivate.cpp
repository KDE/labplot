/***************************************************************************
    File                 : AbstractColumnPrivate.cpp
    Project              : SciDAVis/LabPlot
    Description          : Private data class of AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs*gmx.net),
	                                      Knut Franke (knut.franke*gmx.de)
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

#include "AbstractColumnPrivate.h"

/**
 * \class AbstractColumn::Private
 * \brief Private data class of AbstractColumn
 */

/**
 * \brief Ctor
 */
AbstractColumn::Private::Private(AbstractColumn *owner) : m_owner(owner) {
	Q_CHECK_PTR(m_owner);
}

/**
 * \brief Return the constant masking interval attribute
 */
const IntervalAttribute<bool> &AbstractColumn::Private::masking() const {
	return m_masking;
}

/**
 * \brief Return the masking interval attribute
 */
IntervalAttribute<bool> &AbstractColumn::Private::masking() {
	return m_masking;
}

/**
 * \brief Replace the list of intervals of masked rows
 */
void AbstractColumn::Private::replaceMasking(IntervalAttribute<bool> masking) {
	m_masking = masking;
}

