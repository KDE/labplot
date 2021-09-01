/*
    File                 : BigInt2MonthFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Conversion filter bigint -> QDateTime, interpreting
    the input numbers as months of the year.

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef BIGINT2MONTH_FILTER_H
#define BIGINT2MONTH_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>
#include <cmath>

//! Conversion filter bigint -> QDateTime, interpreting the input numbers as months of the year.
class BigInt2MonthFilter : public AbstractSimpleFilter {
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
		qint64 inputValue = m_inputs.value(0)->bigIntAt(row);
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
		return source->columnMode() == AbstractColumn::ColumnMode::BigInt;
	}
};

#endif // ifndef BIGINT2MONTH_FILTER_H

