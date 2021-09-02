/*
    File                 : BigInt2IntegerFilter.h
    Project              : AbstractColumn
    Description          : conversion filter bigint -> integer
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BIGINT2INTEGER_FILTER_H
#define BIGINT2INTEGER_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>
#include <cmath>

//! conversion filter bigint -> integer
class BigInt2IntegerFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	BigInt2IntegerFilter() {}

	int integerAt(int row) const override {
		if (!m_inputs.value(0)) return 0;

		qint64 value = m_inputs.value(0)->bigIntAt(row);

		int result = static_cast<int>(value);
		//DEBUG("BigInt2Integer::integerAt() " << value << " -> " << result);

		return result;
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Integer; }

protected:
	//! Using typed ports: only bigint inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::BigInt;
	}
};

#endif // ifndef BIGINT2INTEGER_FILTER_H
