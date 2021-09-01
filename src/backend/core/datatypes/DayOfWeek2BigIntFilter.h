/*
    File                 : DayOfWeek2BigIntFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Conversion filter QDateTime -> bigint, translating
    dates into days of the week (Monday -> 1).

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef DAY_OF_WEEK2BIGINT_FILTER_H
#define DAY_OF_WEEK2BIGINT_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDate>

//! Conversion filter QDateTime -> bigint, translating dates into days of the week (Monday -> 1).
class DayOfWeek2BigIntFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	qint64 bigIntAt(int row) const override {
		DEBUG("bigIntAt()");
		if (!m_inputs.value(0)) return 0;
		QDate date = m_inputs.value(0)->dateAt(row);
		if (!date.isValid()) return 0;
		return qint64(date.dayOfWeek());
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::BigInt; }

protected:
	//! Using typed ports: only day inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Day;
	}
};

#endif // ifndef DAY_OF_WEEK2BIGINT_FILTER_H

