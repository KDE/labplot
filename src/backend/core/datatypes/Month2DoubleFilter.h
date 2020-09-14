/***************************************************************************
    File                 : MonthDoubleFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs@gmx.net
    Description          : Conversion filter QDateTime -> double, translating
                           dates into months (January -> 1).

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
#ifndef MONTH2DOUBLE_FILTER_H
#define MONTH2DOUBLE_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDate>
#include <cmath>

/**
 * \brief Conversion filter QDateTime -> double, translating dates into months (January -> 1).
 *
 * \sa QDate::month()
 */
class Month2DoubleFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	double valueAt(int row) const override {
		if (!m_inputs.value(0)) return NAN;
		QDate inputValue = m_inputs.value(0)->dateAt(row);
		if (!inputValue.isValid()) return NAN;
		return double(inputValue.month());
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Numeric; }

protected:
	//! Using typed ports: only date-time inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Month;
	}
};

#endif // ifndef MONTH2DOUBLE_FILTER_H

