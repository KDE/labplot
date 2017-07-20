/***************************************************************************
    File                 : Month2IntegerFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Conversion filter QDateTime -> double, translating
                           dates into months (January -> 1).
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
#ifndef MONTH2INTEGER_FILTER_H
#define MONTH2INTEGER_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDate>
#include <cmath>

/**
 * \brief Conversion filter QDateTime -> int, translating dates into months (January -> 1).
 *
 * \sa QDate::month()
 */
class Month2IntegerFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	virtual int integerAt(int row) const {
		DEBUG("integerAt()");
		if (!m_inputs.value(0)) return NAN;
		QDate inputValue = m_inputs.value(0)->dateAt(row);
		if (!inputValue.isValid()) return NAN;
		return int(inputValue.month());
	}

	//! Return the data type of the column
	virtual AbstractColumn::ColumnMode columnMode() const { return AbstractColumn::Integer; }

protected:
	//! Using typed ports: only date-time inputs are accepted.
	virtual bool inputAcceptable(int, const AbstractColumn *source) {
		return source->columnMode() == AbstractColumn::Month;
	}
};

#endif // ifndef MONTH2INTEGER_FILTER_H

