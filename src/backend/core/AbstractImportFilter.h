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

class AbstractImportFilter : public QObject
{
	Q_OBJECT

	public:
		virtual ~AbstractImportFilter() {}
		virtual AbstractAspect *importAspect(QIODevice *input) = 0;
		virtual QStringList fileExtensions() const = 0;
		virtual QString name() const = 0;
		QString nameAndPatterns() const;
};

#endif // ifndef ABSTRACT_IMPORT_FILTER_H
