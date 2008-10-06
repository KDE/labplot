/***************************************************************************
    File                 : AbstractImportFilter.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Knut Franke
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

#ifndef ABSTRACT_IMPORT_FILTER_H
#define ABSTRACT_IMPORT_FILTER_H

#include <QObject>
#include <QStringList>

class AbstractAspect;
class QIODevice;

// This works just like attr_reader in Ruby (except that you also have to declare the member
// variable), i.e. it declares a get method for a (private) member variable.
#define READER(type, name) \
	type name() const { return m_ ## name; }

// This works just like attr_accessor in Ruby (except that you also have to declare the member
// variable), i.e. it declares get and set methods for a (private) member variable.
// TODO: find a better home for this macro as well as READER
// TODO: Due to technical limitations, this violates the method naming conventions in
// doc/coding.dox. Maybe we should add a special rule for accessor methods. Unless someone knows how
// to let the preprocessor do case conversion.
#define ACCESSOR(type, name) \
	type name() const { return m_ ## name; }; \
	void set_ ## name(const type value) { m_ ## name = value; }

//! Interface for import operations.
/**
 * The least common denominator of all import operations is that they read data from a device
 * (typically, but not necessarily, a file), interpret it in a filter-specific way (as project files
 * in SciDAVis/QtiPlot legacy format, Origin project, CSV table, image, ...) and convert it to the
 * corresponding internal representation (Project, Table, Graph, ...). The application kernel takes
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
class AbstractImportFilter : public QObject
{
	Q_OBJECT

	public:
		virtual ~AbstractImportFilter() {}
		//! Import an object from the specified device and convert it to an Aspect.
		/**
		 * May return 0 if import failed.
		 */
		virtual AbstractAspect * importAspect(QIODevice * input) = 0;
		//! The file extension(s) typically associated with the handled format.
		virtual QStringList fileExtensions() const = 0;
		//! A (localized) name for the filter.
		virtual QString name() const = 0;
		//! Uses name() and fileExtensions() to produce a filter specification as used by QFileDialog.
		QString nameAndPatterns() const {
			return name() + " (*." + fileExtensions().join(" *.") + ")";
		}
};

#endif // ifndef ABSTRACT_IMPORT_FILTER_H
