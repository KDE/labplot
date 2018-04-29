/***************************************************************************
    File                 : DateTime2StringFilter.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007 by Knut Franke (knut.franke@gmx.de)
    Description          : Conversion filter QDateTime -> QString.

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
#ifndef DATE_TIME2STRING_FILTER_H
#define DATE_TIME2STRING_FILTER_H

#include "backend/core/AbstractSimpleFilter.h"

class DateTime2StringFilterSetFormatCmd;

//! Conversion filter QDateTime -> QString.
class DateTime2StringFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	//! Standard constructor.
	explicit DateTime2StringFilter(QString format="yyyy-MM-dd hh:mm:ss.zzz") : m_format(format) {}
	//! Set the format string to be used for conversion.
	void setFormat(const QString& format);

	//! Return the format string
	/**
		* The default format string is "yyyy-MM-dd hh:mm:ss.zzz".
	* \sa QDate::toString()
		*/
	QString format() const { return m_format; }

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::Text; }

private:
	friend class DateTime2StringFilterSetFormatCmd;
	//! The format string.
	QString m_format;

public:
	QString textAt(int row) const override;

	//! \name XML related functions
	//@{
	void writeExtraAttributes(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	//@}

protected:
	//! Using typed ports: only DateTime inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override;
};

#endif // ifndef DATE_TIME2STRING_FILTER_H

