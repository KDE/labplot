/***************************************************************************
    File                 : AbstractExportFilter.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Knut Franke
    Email (use @ for *)  : Knut.Franke*gmx.net
    Description          : Interface for export operations.

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

#include "AbstractExportFilter.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

/**
 * \class AbstractExportFilter
 * \brief Interface for export operations.
 *
 * This is analogous to AbstractImportFilter.
 */

/**
 * \fn bool AbstractExportFilter::exportAspect(AbstractAspect *object, QIODevice *output)
 * \brief Export object to output.
 *
 * \return true if export was sucessfull, false otherwise
 */

/**
 * \fn QStringList AbstractExportFilter::fileExtensions() const
 * \brief The file extension(s) typically associated with the handled format.
 */

/**
 * \fn QString AbstractExportFilter::name() const
 * \brief A (localized) name for the filter.
 */

/**
 * \brief Uses name() and fileExtensions() to produce a filter specification as used by QFileDialog.
 */
QString AbstractExportFilter::nameAndPatterns() const {
	return name() + " (*." + fileExtensions().join(" *.") + ")";
}
