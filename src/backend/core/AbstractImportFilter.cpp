/***************************************************************************
    File                 : AbstractImportFilter.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Knut Franke
    Email (use @ for *)  : Knut.Franke*gmx.net
    Description          : Interface for import operations.

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

#include "AbstractImportFilter.h"

/**
 * \class AbstractImportFilter
 * \brief Interface for import operations.
 *
 * The least common denominator of all import operations is that they read data from a device
 * (typically, but not necessarily, a file), interpret it in a filter-specific way (as project files
 * in SciDAVis/QtiPlot legacy format, Origin project, CSV table, image, ...) and convert it to the
 * corresponding internal representation (Project, Spreadsheet, Graph, ...). The application kernel takes
 * care of all the pesky details, such as letting the user choose one or more files to import or
 * adding the resulting Aspect either to the current project or a newly created one.
 *
 * The main design goal was to make implementing import filters as easy as possible. Therefore,
 * filter options are simply declared as Qt properties; the implementation does not have to bother
 * with providing a GUI for its options (this task is taken over by ImportDialog). This approach
 * allows fast prototyping of import filters, but leaves some things to be desired usability-wise
 * (localized option labels, tool tips, intelligent layout, etc.). Some of these deficiencies could
 * be overcome by adding extra methods to filters
 * (e.g. Qstring labelText(const char * property_name)), but presumably one will want to replace the
 * auto-generated GUI with a custom one once the filter is completely implemented and tested. This
 * is done by providing a method "QWidget * makeOptionsGui();". The result of calling
 * this method is then used by ImportDialog instead of the auto-generated GUI. The filter still has
 * to inherit from QObject and use the Q_OBJECT macro, since otherwise there's no way of testing for
 * the presence of this method.
 */

/**
 * \fn AbstractAspect *AbstractImportFilter::importAspect(QIODevice *input)
 * \brief Import an object from the specified device and convert it to an Aspect.
 *
 * May return 0 if import failed.
 */

/**
 * \fn QStringList AbstractImportFilter::fileExtensions() const
 * \brief The file extension(s) typically associated with the handled format.
 */

/**
 * \fn QString AbstractImportFilter::name() const
 * \brief A (localized) name for the filter.
 */

/**
 * \brief Uses name() and fileExtensions() to produce a filter specification as used by QFileDialog.
 */
 QString AbstractImportFilter::nameAndPatterns() const {
	 return name() + " (*." + fileExtensions().join(" *.") + ')';
 }
