/***************************************************************************
    File                 : Integer2DateTimeFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs@gmx.net
    Description          : Conversion filter int -> QDateTime, interpreting
                           the input numbers as Julian days.

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
#ifndef INTEGER2DATE_TIME_FILTER_H
#define INTEGER2DATE_TIME_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>

//! Conversion filter double -> QDateTime, interpreting the input numbers as (fractional) Julian days.
class Integer2DateTimeFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	virtual QDate dateAt(int row) const {
		if (!m_inputs.value(0)) return QDate();
		int inputValue = m_inputs.value(0)->integerAt(row);
		return QDate(1900, 1, 1 + inputValue);
	}
	virtual QDateTime dateTimeAt(int row) const {
		return QDateTime(dateAt(row), QTime());
	}

	//! Return the data type of the column
	virtual AbstractColumn::ColumnMode columnMode() const { return AbstractColumn::DateTime; }

protected:
	//! Using typed ports: only double inputs are accepted.
	virtual bool inputAcceptable(int, const AbstractColumn *source) {
		return source->columnMode() == AbstractColumn::Integer;
	}
};

#endif // ifndef INTEGER2DATE_TIME_FILTER_H

