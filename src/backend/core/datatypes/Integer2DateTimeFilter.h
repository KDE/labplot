/*
    File                 : Integer2DateTimeFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    Description          : Conversion filter int -> QDateTime, interpreting
    the input numbers as Julian days.
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTEGER2DATE_TIME_FILTER_H
#define INTEGER2DATE_TIME_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>

//! Conversion filter double -> QDateTime, interpreting the input numbers as (fractional) Julian days.
class Integer2DateTimeFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	QDate dateAt(int row) const override {
		if (!m_inputs.value(0)) return QDate();
		int inputValue = m_inputs.value(0)->integerAt(row);
		return QDate(1900, 1, 1 + inputValue);
	}
	QDateTime dateTimeAt(int row) const override {
		return QDateTime(dateAt(row), QTime());
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::DateTime; }

protected:
	//! Using typed ports: only double inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Integer;
	}
};

#endif // ifndef INTEGER2DATE_TIME_FILTER_H

