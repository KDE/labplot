/***************************************************************************
    File                 : Double2IntegerFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : conversion filter double -> int.

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
#ifndef DOUBLE2INTEGER_FILTER_H
#define DOUBLE2INTEGER_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>
#include <cmath>

//! conversion filter double -> int.
class Double2IntegerFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	Double2IntegerFilter() {}

	virtual int integerAt(int row) const {
		DEBUG("Double2Integer::integerAt()");
		if (!m_inputs.value(0)) return 0;

		double value = m_inputs.value(0)->valueAt(row);
		DEBUG("	value = " << value);

		int result = 0;
		if (!std::isnan(value))
			result = (int)round(value);
		DEBUG("	result = " << result);

		return result;
	}

	//! Return the data type of the column
	virtual AbstractColumn::ColumnMode columnMode() const { return AbstractColumn::Integer; }

protected:
	//! Using typed ports: only double inputs are accepted.
	virtual bool inputAcceptable(int, const AbstractColumn *source) {
		return source->columnMode() == AbstractColumn::Numeric;
	}
};

#endif // ifndef DOUBLE2INTEGER_FILTER_H
