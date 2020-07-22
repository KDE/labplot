/***************************************************************************
    File                 : Double2IntegerFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2017-2020 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include <cmath>

//! conversion filter double -> int.
class Double2IntegerFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	Double2IntegerFilter() {}

	int integerAt(int row) const override {
		if (!m_inputs.value(0))
			return 0;

		double value = m_inputs.value(0)->valueAt(row);

		int result{0};
		if (!std::isnan(value))
			result = (int)round(value);
		//DEBUG("Double2Integer::integerAt() " << value << " -> " << result);

		return result;
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Integer; }

protected:
	//! Using typed ports: only double inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Numeric;
	}
};

#endif // ifndef DOUBLE2INTEGER_FILTER_H
