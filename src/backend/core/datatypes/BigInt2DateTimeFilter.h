/*
    File                 : BigInt2DateTimeFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs@gmx.net
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
    Description          : Conversion filter int -> QDateTime, interpreting
    the input numbers as Julian days.
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BIGINT2DATE_TIME_FILTER_H
#define BIGINT2DATE_TIME_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>

//! Conversion filter bigint -> QDateTime, interpreting the input numbers as Julian days.
class BigInt2DateTimeFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	QDate dateAt(int row) const override {
		if (!m_inputs.value(0)) return QDate();
		qint64 inputValue = m_inputs.value(0)->bigIntAt(row);
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
		return source->columnMode() == AbstractColumn::ColumnMode::BigInt;
	}
};

#endif // ifndef BIGINT2DATE_TIME_FILTER_H

