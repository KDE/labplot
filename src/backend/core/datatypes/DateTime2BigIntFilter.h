/*
	File                 : DateTime2BigIntFilter.h
	Project              : AbstractColumn
	Description          : Conversion filter QDateTime -> bigint (using Julian day).
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATE_TIME2BIGINT_FILTER_H
#define DATE_TIME2BIGINT_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>

//! Conversion filter QDateTime -> BigInt (using Julian day).
class DateTime2BigIntFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	qint64 bigIntAt(int row) const override {
		if (!m_inputs.value(0))
			return 0;
		const auto dt = m_inputs.value(0)->dateTimeAt(row);
		if (!dt.isValid())
			return 0;

		return dt.toMSecsSinceEpoch(); // TODO: ask unit (see other DateTime filter)
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override {
		return AbstractColumn::ColumnMode::BigInt;
	}

protected:
	//! Using typed ports: only DateTime inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn* source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::DateTime;
	}
};

#endif // ifndef DATE_TIME2BIGINT_FILTER_H
