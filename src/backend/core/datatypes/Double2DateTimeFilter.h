/*
    File                 : Double2DateTimeFilter.h
    Project              : AbstractColumn
    Description          : Conversion filter double -> QDateTime, interpreting
    the input numbers as (fractional) Julian days.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DOUBLE2DATE_TIME_FILTER_H
#define DOUBLE2DATE_TIME_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>
#include <cmath>

//! Conversion filter double -> QDateTime, interpreting the input numbers as (fractional) Julian days.
class Double2DateTimeFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	QDate dateAt(int row) const override {
		if (!m_inputs.value(0)) return QDate();
		double inputValue = m_inputs.value(0)->valueAt(row);
		if (std::isnan(inputValue)) return QDate();
		//QDEBUG("	convert " << inputValue << " to " << QDate(1900, 1, int(inputValue)));
		return QDate(1900, 1, 1 + int(inputValue));
	}
	QTime timeAt(int row) const override {
		if (!m_inputs.value(0)) return QTime();
		double inputValue = m_inputs.value(0)->valueAt(row);
		if (std::isnan(inputValue)) return QTime();
		// we only want the digits behind the dot and convert them from fraction of day to milliseconds
		//QDEBUG("	convert " << inputValue << " to " << QTime(0,0,0,0).addMSecs(int( (inputValue - int(inputValue)) * 86400000.0 )));
		return QTime(0,0,0,0).addMSecs(int( (inputValue - int(inputValue)) * 86400000.0 ));
	}
	QDateTime dateTimeAt(int row) const override {
		return QDateTime(dateAt(row), timeAt(row));
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::DateTime; }

protected:
	//! Using typed ports: only double inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Double;
	}
};

#endif // ifndef DOUBLE2DATE_TIME_FILTER_H

