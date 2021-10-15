/*
    File                 : BigInt2DoubleFilter.h
    Project              : AbstractColumn
    Description          : conversion filter int -> double.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BIGINT2DOUBLE_FILTER_H
#define BIGINT2DOUBLE_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>

//! conversion filter double -> int.
class BigInt2DoubleFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	BigInt2DoubleFilter() {}

	double valueAt(int row) const override {
		if (!m_inputs.value(0)) return 0;

		qint64 value = m_inputs.value(0)->bigIntAt(row);
		double result = (double)value;
		//DEBUG("BigInt2Double::integerAt() " << value << " -> " << result);

		return result;
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Double; }

protected:
	//! Using typed ports: only bigint inputs are accepted
	bool inputAcceptable(int, const AbstractColumn *source) override {
		DEBUG("inputAcceptable(): source type = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, source->columnMode()));
		return source->columnMode() == AbstractColumn::ColumnMode::BigInt;
	}
};

#endif // ifndef BIGINT2DOUBLE_FILTER_H
