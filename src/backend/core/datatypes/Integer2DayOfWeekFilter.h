/*
    File                 : Integer2DayOfWeekFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Stefan Gerlach <stefan.gerlach@uni.kn>
    Description          : Conversion filter int -> QDateTime, interpreting
    the input numbers as days of the week (1 -> Monday).
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTEGER2DAY_OF_WEEK_FILTER_H
#define INTEGER2DAY_OF_WEEK_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>

//! Conversion filter int -> QDateTime, interpreting the input numbers as days of the week (1 = Monday).
class Integer2DayOfWeekFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	QDate dateAt(int row) const override {
		if (!m_inputs.value(0)) return QDate();
		int inputValue = m_inputs.value(0)->integerAt(row);
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
	//! Using typed ports: only integer inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Integer;
	}
};

#endif // ifndef INTEGER2DAY_OF_WEEK_FILTER_H
