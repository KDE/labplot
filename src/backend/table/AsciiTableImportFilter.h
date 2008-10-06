/***************************************************************************
    File                 : AsciiTableImportFilter.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Knut Franke
    Email (use @ for *)  : Knut.Franke*gmx.net
    Description          : Import an ASCII file as Table.

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

#ifndef ASCII_TABLE_IMPORT_FILTER_H
#define ASCII_TABLE_IMPORT_FILTER_H

#include "core/AbstractImportFilter.h"

//! Import an ASCII file as Table.
/**
 * This is a complete rewrite of equivalent functionality previously found in Table.
 *
 * Transition note:
 * Conversion to numeric format intentionally not supported, since this can easily be done via
 * Table's control tabs and automatic conversion would artificially limit the types of data that
 * can be imported. The Type tab should be improved to allow custom decimal separators to be specified.
 *
 * TODO: port options GUI from ImportTableDialog
 */
class AsciiTableImportFilter : public AbstractImportFilter
{
	Q_OBJECT

	public:
		AsciiTableImportFilter() :
			m_ignored_lines(0),
			m_separator("\t"),
			m_first_row_names_columns(true),
			m_trim_whitespace(false),
			m_simplify_whitespace(false) {}
		virtual AbstractAspect * importAspect(QIODevice * input);
		virtual QStringList fileExtensions() const;
		virtual QString name() const { return QObject::tr("ASCII table"); }

		ACCESSOR(int, ignored_lines);
		Q_PROPERTY(int ignored_lines READ ignored_lines WRITE set_ignored_lines);

		QString separator() const { QString result = m_separator; return result.replace("\t", "\\t"); }
		void set_separator(const QString &value) { m_separator = value; m_separator.replace("\\t","\t"); }
		Q_PROPERTY(QString separator READ separator WRITE set_separator);

		ACCESSOR(bool, first_row_names_columns);
		Q_PROPERTY(bool first_row_names_columns READ first_row_names_columns WRITE set_first_row_names_columns);

		ACCESSOR(bool, trim_whitespace);
		Q_PROPERTY(bool trim_whitespace READ trim_whitespace WRITE set_trim_whitespace);

		ACCESSOR(bool, simplify_whitespace);
		Q_PROPERTY(bool simplify_whitespace READ simplify_whitespace WRITE set_simplify_whitespace);

	private:
		int m_ignored_lines;
		QString m_separator;
		bool m_first_row_names_columns;
		bool m_trim_whitespace;
		bool m_simplify_whitespace;
};

#endif // ifndef ASCII_TABLE_IMPORT_FILTER_H
