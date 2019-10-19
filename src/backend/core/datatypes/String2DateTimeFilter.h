/***************************************************************************
    File                 : String2DateTimeFilter.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007 by Knut Franke (knut.franke@gmx.de)
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

#include "backend/core/AbstractSimpleFilter.h"

/**
 * \brief Conversion filter QString -> QDateTime.
 *
 * The standard use of this filter is explicitly specifying the date/time format of the strings
 * on the input, either in the constructor or via setFormat().
 * However, if the input fails to comply to this format, String2DateTimeFilter
 * tries to guess the format, using internal lists of common date and time formats (#date_formats
 * and #time_formats).
 */
class String2DateTimeFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	//! Standard constructor.
	explicit String2DateTimeFilter(const QString& format="yyyy-MM-dd hh:mm:ss.zzz") : m_format(format) {}
	//! Set the format string to be used for conversion.
	void setFormat(const QString& format);
	//! Return the format string
	/**
	 * The default format string is "yyyy-MM-dd hh:mm:ss.zzz".
	 * \sa QDate::toString()
	 */
	QString format() const { return m_format; }

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override;

	//! \name XML related functions
	//@{
	void writeExtraAttributes(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	//@}

private:
	friend class String2DateTimeFilterSetFormatCmd;
	//! The format string.
	QString m_format;

	static const char * date_formats[];
	static const char * time_formats[];

public:
	QDateTime dateTimeAt(int row) const override;
	QDate dateAt(int row) const override;
	QTime timeAt(int row) const override;

protected:
	//! Using typed ports: only string inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override;
};

#endif // ifndef STRING2DATE_TIME_FILTER_H

