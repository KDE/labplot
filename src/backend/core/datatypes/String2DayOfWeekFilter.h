/*
    File                 : String2DayOfWeekFilter.h
    Project              : AbstractColumn
    Description          : Conversion filter String -> QDateTime, interpreting
    the input as days of the week (either numeric or "Mon" etc).
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STRING2DAYOFWEEK_FILTER_H
#define STRING2DAYOFWEEK_FILTER_H

#include "../AbstractSimpleFilter.h"

class QDateTime;

//! Conversion filter String -> QDateTime, interpreting the input as days of the week (either numeric or "Mon" etc).
class String2DayOfWeekFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	QDate dateAt(int row) const override {
			return dateTimeAt(row).date();
	}

	QTime timeAt(int row) const override{
			return dateTimeAt(row).time();
	}

	QDateTime dateTimeAt(int row) const override {
		if (!m_inputs.value(0)) return QDateTime();

		QString input_value = m_inputs.value(0)->textAt(row);
		if (input_value.isEmpty()) return QDateTime();
		bool ok;
		int day_value = input_value.toInt(&ok);
		if(!ok) {
			QDate temp = QDate::fromString(input_value, QLatin1String("ddd"));
			if(!temp.isValid())
				temp = QDate::fromString(input_value, QLatin1String("dddd"));
			if(!temp.isValid())
				return QDateTime();
			else
				day_value = temp.dayOfWeek();
		}

		// Don't use Julian days here since support for years < 1 is bad
		// Use 1900-01-01 instead (a Monday)
		QDate result_date = QDate(1900,1,1).addDays(day_value - 1);
		QTime result_time = QTime(0,0,0,0);
		return QDateTime(result_date, result_time);
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Day; }

protected:
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Text;
	}
};

#endif // ifndef STRING2DAYOFWEEK_FILTER_H

