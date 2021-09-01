/*
    File                 : Integer2BigIntFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
    Description          : conversion filter int -> bigint
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTEGER2BIGINT_FILTER_H
#define INTEGER2BIGINT_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>

//! conversion filter integer -> bigint
class Integer2BigIntFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	Integer2BigIntFilter() {}

	qint64 bigIntAt(int row) const override {
		if (!m_inputs.value(0)) return 0;

		int value = m_inputs.value(0)->integerAt(row);
		qint64 result = (qint64)value;
		//DEBUG("Integer2BigInt::integerAt() " << value << " -> " << result);

		return result;
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::BigInt; }

protected:
	//! Using typed ports: only integer inputs are accepted
	bool inputAcceptable(int, const AbstractColumn *source) override {
		DEBUG("inputAcceptable(): source type = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, source->columnMode()));
		return source->columnMode() == AbstractColumn::ColumnMode::Integer;
	}
};

#endif // ifndef INTEGER2BIGINT_FILTER_H
