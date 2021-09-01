/*
    File                 : Month2BigIntFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
    Description          : Conversion filter QDateTime -> bigint, translating
    dates into months (January -> 1).

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MONTH2BIGINT_FILTER_H
#define MONTH2BIGINT_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDate>

/**
 * \brief Conversion filter QDateTime -> bigint, translating dates into months (January -> 1).
 *
 * \sa QDate::month()
 */
class Month2BigIntFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	qint64 bigIntAt(int row) const override {
		DEBUG("bigIntAt()");
		if (!m_inputs.value(0)) return 0;
		QDate inputValue = m_inputs.value(0)->dateAt(row);
		if (!inputValue.isValid()) return 0;
		return qint64(inputValue.month());
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::BigInt; }

protected:
	//! Using typed ports: only date-time inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Month;
	}
};

#endif // ifndef MONTH2BIGINT_FILTER_H

