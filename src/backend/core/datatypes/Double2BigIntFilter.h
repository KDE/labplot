/*
    File                 : Double2BigIntFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : conversion filter double -> bigint

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef DOUBLE2BIGINT_FILTER_H
#define DOUBLE2BIGINT_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>
#include <cmath>

//! conversion filter double -> int.
class Double2BigIntFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	Double2BigIntFilter() {}

	qint64 bigIntAt(int row) const override {
		if (!m_inputs.value(0)) return 0;

		double value = m_inputs.value(0)->valueAt(row);

		int result = 0;
		if (!std::isnan(value))
			result = (qint64)round(value);
		//DEBUG("Double2BigInt::integerAt() " << value << " -> " << result);

		return result;
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::BigInt; }

protected:
	//! Using typed ports: only double inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Numeric;
	}
};

#endif // ifndef DOUBLE2BIGINT_FILTER_H
