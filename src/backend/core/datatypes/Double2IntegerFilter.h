/*
    File                 : Double2IntegerFilter.h
    Project              : AbstractColumn
    Description          : conversion filter double -> int.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017-2020 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
		return source->columnMode() == AbstractColumn::ColumnMode::Double;
	}
};

#endif // ifndef DOUBLE2INTEGER_FILTER_H
