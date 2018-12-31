/***************************************************************************
    File                 : AbstractColumnPrivate.cpp
    Project              : LabPlot
    Description          : Private data class of AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs@gmx.net),
    Copyright            : (C) 2007-2009  Knut Franke (knut.franke@gmx.de)

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
 * \class AbstractColumnPrivate
 * \brief Private data class of AbstractColumn
 */

/**
 * \brief Ctor
 */
AbstractColumnPrivate::AbstractColumnPrivate(AbstractColumn* owner) : m_owner(owner) {
	Q_CHECK_PTR(m_owner);
}

