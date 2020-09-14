/***************************************************************************
    File                 : String2MonthFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Conversion filter String -> QDateTime, interpreting
                           the input as months of the year (either numeric or "Jan" etc).

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
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

