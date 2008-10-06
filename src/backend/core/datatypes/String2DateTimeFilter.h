/***************************************************************************
    File                 : String2DateTimeFilter.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
                           Knut Franke
    Email (use @ for *)  : thzs*gmx.net, knut.franke*gmx.de
    Description          : Conversion filter QString -> QDateTime.
                           
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
#ifndef STRING2DATE_TIME_FILTER_H
#define STRING2DATE_TIME_FILTER_H

#include "core/AbstractSimpleFilter.h"
#include <QDateTime>
#include <QDate>
#include <QTime>

/**
 * \brief Conversion filter QString -> QDateTime.
 *
 * The standard use of this filter is explicitly specifiying the date/time format of the strings
 * on the input, either in the constructor or via setFormat().
 * However, if the input fails to comply to this format, String2DateTimeFilter
 * tries to guess the format, using internal lists of common date and time formats (#date_formats
 * and #time_formats).
 */
class String2DateTimeFilter : public AbstractSimpleFilter
{
	Q_OBJECT

	public:
		//! Standard constructor.
		explicit String2DateTimeFilter(QString format="yyyy-MM-dd hh:mm:ss.zzz") : m_format(format) {}
		//! Set the format string to be used for conversion.
		void setFormat(const QString& format);
		//! Return the format string
		/**
		 * The default format string is "yyyy-MM-dd hh:mm:ss.zzz".
		 * \sa QDate::toString()
		 */
		QString format() const { return m_format; }

		//! Return the data type of the column
		virtual SciDAVis::ColumnDataType dataType() const { return SciDAVis::TypeQDateTime; }

		//! \name XML related functions
		//@{
		virtual void writeExtraAttributes(QXmlStreamWriter * writer) const;
		virtual bool load(XmlStreamReader * reader);
		//@}

	signals:
		void formatChanged();

	private:
		friend class String2DateTimeFilterSetFormatCmd;
		//! The format string.
		QString m_format;

		static const char * date_formats[];
		static const char * time_formats[];

	public:
		virtual QDateTime dateTimeAt(int row) const;
		virtual QDate dateAt(int row) const { return dateTimeAt(row).date(); }
		virtual QTime timeAt(int row) const { return dateTimeAt(row).time(); }

	protected:
		//! Using typed ports: only string inputs are accepted.
		virtual bool inputAcceptable(int, const AbstractColumn *source) {
			return source->dataType() == SciDAVis::TypeQString;
		}
};

class String2DateTimeFilterSetFormatCmd : public QUndoCommand
{
	public:
		String2DateTimeFilterSetFormatCmd(String2DateTimeFilter* target, const QString &new_format);

		virtual void redo();
		virtual void undo();

	private:
		String2DateTimeFilter* m_target;
		QString m_other_format;
};

#endif // ifndef STRING2DATE_TIME_FILTER_H

