/***************************************************************************
    File                 : DateTime2IntegerFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Conversion filter QDateTime -> int (using Julian day).

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
#ifndef DATE_TIME2INTEGER_FILTER_H
#define DATE_TIME2INTEGER_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>

//! Conversion filter QDateTime -> int (using Julian day).
class DateTime2IntegerFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	int integerAt(int row) const override {
		//DEBUG("integerAt()");
		if (!m_inputs.value(0)) return 0;
		QDateTime inputDate = m_inputs.value(0)->dateTimeAt(row);
		if (!inputDate.isValid()) return 0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
		QDateTime start(QDate(1900, 1, 1).startOfDay());
#else
		QDateTime start(QDate(1900, 1, 1));
#endif
		return int(start.daysTo(inputDate));
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Integer; }

protected:
	//! Using typed ports: only DateTime inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::DateTime;
	}
};

#endif // ifndef DATE_TIME2INTEGER_FILTER_H

