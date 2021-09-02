/*
    File                 : Integer2MonthFilter.h
    Project              : AbstractColumn
    Description          : Conversion filter int -> QDateTime, interpreting
    the input numbers as months of the year.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTEGER2MONTH_FILTER_H
#define INTEGER2MONTH_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>
#include <cmath>

//! Conversion filter double -> QDateTime, interpreting the input numbers as months of the year.
class Integer2MonthFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	QDate dateAt(int row) const override {
		return dateTimeAt(row).date();
	}
	QTime timeAt(int row) const override {
		return dateTimeAt(row).time();
	}
	QDateTime dateTimeAt(int row) const override {
		if (!m_inputs.value(0)) return QDateTime();
		int inputValue = m_inputs.value(0)->integerAt(row);
		// Don't use Julian days here since support for years < 1 is bad
		// Use 1900-01-01 instead
		QDate result_date = QDate(1900,1,1).addMonths(inputValue);
		QTime result_time = QTime(0,0,0,0);
		return QDateTime(result_date, result_time);
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Month; }

protected:
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Integer;
	}
};

#endif // ifndef INTEGER2MONTH_FILTER_H

