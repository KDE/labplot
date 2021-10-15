/*
    File                 : DayOfWeek2DoubleFilter.h
    Project              : AbstractColumn
    Description          : Conversion filter QDateTime -> double, translating
    dates into days of the week (Monday -> 1).
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DAY_OF_WEEK2DOUBLE_FILTER_H
#define DAY_OF_WEEK2DOUBLE_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDate>
#include <cmath>

//! Conversion filter QDateTime -> double, translating dates into days of the week (Monday -> 1).
class DayOfWeek2DoubleFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	double valueAt(int row) const override {
		if (!m_inputs.value(0)) return NAN;
		QDate date = m_inputs.value(0)->dateAt(row);
		if (!date.isValid()) return NAN;
		return double(date.dayOfWeek());
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Double; }

protected:
	//! Using typed ports: only date-time inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Day;
	}
};

#endif // ifndef DAY_OF_WEEK2DOUBLE_FILTER_H

