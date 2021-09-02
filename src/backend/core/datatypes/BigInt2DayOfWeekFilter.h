/*
    File                 : BigInt2DayOfWeekFilter.h
    Project              : AbstractColumn
    Description          : Conversion filter bigint -> QDateTime, interpreting
    the input numbers as days of the week (1 -> Monday).
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BIGINT2DAY_OF_WEEK_FILTER_H
#define BIGINT2DAY_OF_WEEK_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>

//! Conversion filter int -> QDateTime, interpreting the input numbers as days of the week (1 = Monday).
class BigInt2DayOfWeekFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	QDate dateAt(int row) const override {
		if (!m_inputs.value(0)) return QDate();
		qint64 inputValue = m_inputs.value(0)->bigIntAt(row);
		// Don't use Julian days here since support for years < 1 is bad
		// Use 1900-01-01 instead (a Monday)
		return QDate(1900,1,1).addDays(inputValue);
	}
	QTime timeAt(int row) const override {
		Q_UNUSED(row)
		return QTime(0,0,0,0);
	}
	QDateTime dateTimeAt(int row) const override {
		return QDateTime(dateAt(row), timeAt(row));
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Day; }

protected:
	//! Using typed ports: only bigint inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::BigInt;
	}
};

#endif // ifndef BIGINT2DAY_OF_WEEK_FILTER_H
