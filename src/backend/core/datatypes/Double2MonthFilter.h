/***************************************************************************
    File                 : Double2MonthFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs@gmx.net
    Description          : Conversion filter double -> QDateTime, interpreting
                           the input numbers as months of the year.

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
		return source->columnMode() == AbstractColumn::ColumnMode::Numeric;
	}
};

#endif // ifndef DOUBLE2MONTH_FILTER_H

