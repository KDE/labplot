/***************************************************************************
    File                 : DayOfWeek2IntegerFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Conversion filter QDateTime -> int, translating
                           dates into days of the week (Monday -> 1).

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
#ifndef DAY_OF_WEEK2INTEGER_FILTER_H
#define DAY_OF_WEEK2INTEGER_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDate>

//! Conversion filter QDateTime -> int, translating dates into days of the week (Monday -> 1).
class DayOfWeek2IntegerFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	int integerAt(int row) const override {
		DEBUG("integerAt()");
		if (!m_inputs.value(0)) return 0;
		QDate date = m_inputs.value(0)->dateAt(row);
		if (!date.isValid()) return 0;
		return int(date.dayOfWeek());
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Integer; }

protected:
	//! Using typed ports: only date-time inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Day;
	}
};

#endif // ifndef DAY_OF_WEEK2INTEGER_FILTER_H

