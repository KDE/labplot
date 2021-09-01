/*
    File                 : AbstractColumnPrivate.cpp
    Project              : LabPlot
    Description          : Private data class of AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007-2009 Tilman Benkert (thzs@gmx.net),
    SPDX-FileCopyrightText: 2007-2009 Knut Franke (knut.franke@gmx.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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

