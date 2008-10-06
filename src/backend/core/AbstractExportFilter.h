/***************************************************************************
    File                 : AbstractExportFilter.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Knut Franke
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

#ifndef ABSTRACT_EXPORT_FILTER_H
#define ABSTRACT_EXPORT_FILTER_H

class AbstractAspect;
class QIODevice;
class QStringList;

//! Interface for export operations.
/**
 * This is analogous to AbstractImportFilter.
 */
class AbstractExportFilter
{
	public:
		virtual ~AbstractExportFilter() {}
		//! Export object to output.
		/**
		 * \return true if export was sucessfull, false otherwise
		 */
		virtual bool exportAspect(AbstractAspect * object, QIODevice * output) = 0;
		//! The file extension(s) typically associated with the handled format.
		virtual QStringList fileExtensions() const = 0;
		//! A (localized) name for the filter.
		virtual QString name() const = 0;
		//! Uses name() and fileExtensions() to produce a filter specification as used by QFileDialog.
		QString nameAndPatterns() const {
			return name() + " (*." + fileExtensions().join(" *.") + ")";
		}
};

#endif // ifndef ABSTRACT_EXPORT_FILTER_H
