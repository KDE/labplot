/***************************************************************************
    File                 : Integer2DayOfWeekFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Conversion filter int -> QDateTime, interpreting
                           the input numbers as days of the week (1 -> Monday).

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
#ifndef INTEGER2DAY_OF_WEEK_FILTER_H
#define INTEGER2DAY_OF_WEEK_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>

//! Conversion filter int -> QDateTime, interpreting the input numbers as days of the week (1 = Monday).
class Integer2DayOfWeekFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	QDate dateAt(int row) const override {
		if (!m_inputs.value(0)) return QDate();
		int inputValue = m_inputs.value(0)->integerAt(row);
		// Don't use Julian days here since support for years < 1 is bad
		// Use 1900-01-01 instead (a Monday)
		return QDate(1900,1,1).addDays(inputValue);
	}
	QTime timeAt(int row) const override {
		Q_UNUSED(row)
		return QTime(0,0,0,0);
	}
	QDateTime dateTimeAt(int row) const override {
		return QDateTime(dateAt(row), timeAt(row));
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Day; }

protected:
	//! Using typed ports: only integer inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Integer;
	}
};

#endif // ifndef INTEGER2DAY_OF_WEEK_FILTER_H
