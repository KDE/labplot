/*
    File                 : String2MonthFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    Description          : Conversion filter String -> QDateTime, interpreting
    the input as months of the year (either numeric or "Jan" etc).
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STRING2MONTH_FILTER_H
#define STRING2MONTH_FILTER_H

#include "../AbstractSimpleFilter.h"

class QDateTime;

//! Conversion filter String -> QDateTime, interpreting the input as months of the year (either numeric or "Jan" etc).
class String2MonthFilter : public AbstractSimpleFilter {
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

		QString input_value = m_inputs.value(0)->textAt(row);
		bool ok;
		int month_value = input_value.toInt(&ok);
		if(!ok) {
			QDate temp = QDate::fromString(input_value, "MMM");
			if(!temp.isValid())
				temp = QDate::fromString(input_value, "MMMM");
			if(!temp.isValid())
				return QDateTime();
			else
				month_value = temp.month();
		}

		// Don't use Julian days here since support for years < 1 is bad
		// Use 1900-01-01 instead
		QDate result_date = QDate(1900,1,1).addMonths(month_value - 1);
		QTime result_time = QTime(0,0,0,0);
		return QDateTime(result_date, result_time);
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Month; }

protected:
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Text;
	}
};

#endif // ifndef STRING2MONTH_FILTER_H

