/*
    File                 : DayOfWeek2IntegerFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Stefan Gerlach <stefan.gerlach@uni.kn>
    Description          : Conversion filter QDateTime -> int, translating
    dates into days of the week (Monday -> 1).
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DAY_OF_WEEK2INTEGER_FILTER_H
#define DAY_OF_WEEK2INTEGER_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDate>

//! Conversion filter QDateTime -> int, translating dates into days of the week (Monday -> 1).
class DayOfWeek2IntegerFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	int integerAt(int row) const override {
		DEBUG("integerAt()");
		if (!m_inputs.value(0)) return 0;
		QDate date = m_inputs.value(0)->dateAt(row);
		if (!date.isValid()) return 0;
		return int(date.dayOfWeek());
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Integer; }

protected:
	//! Using typed ports: only date-time inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Day;
	}
};

#endif // ifndef DAY_OF_WEEK2INTEGER_FILTER_H

