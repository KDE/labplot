/*
    File                 : Double2MonthFilter.h
    Project              : AbstractColumn
    Description          : Conversion filter double -> QDateTime, interpreting
    the input numbers as months of the year.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DOUBLE2MONTH_FILTER_H
#define DOUBLE2MONTH_FILTER_H

#include "../AbstractSimpleFilter.h"
#include "backend/lib/macros.h"
#include <QDateTime>
#include <cmath>

//! Conversion filter double -> QDateTime, interpreting the input numbers as months of the year.
class Double2MonthFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	QDate dateAt(int row) const override {
		return dateTimeAt(row).date();
	}
	QTime timeAt(int row) const override {
		return dateTimeAt(row).time();
	}
	QDateTime dateTimeAt(int row) const override {
		DEBUG("Double2MonthFilter::dateTimeAt() row = " << row);
		if (!m_inputs.value(0)) return QDateTime();
		double inputValue = m_inputs.value(0)->valueAt(row);
		if (std::isnan(inputValue)) return QDateTime();
		// Don't use Julian days here since support for years < 1 is bad
		// Use 1900-01-01 instead
		QDate result_date = QDate(1900,1,1).addMonths(qRound(inputValue - 1.0));
		QTime result_time = QTime(0,0,0,0);
		QDEBUG("value = " << inputValue << " result = " << QDateTime(result_date, result_time));
		return QDateTime(result_date, result_time);
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Month; }

protected:
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Double;
	}
};

#endif // ifndef DOUBLE2MONTH_FILTER_H

