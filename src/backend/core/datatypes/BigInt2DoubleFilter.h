/***************************************************************************
    File                 : BigInt2DoubleFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : conversion filter int -> double.

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
#ifndef BIGINT2DOUBLE_FILTER_H
#define BIGINT2DOUBLE_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>

//! conversion filter double -> int.
class BigInt2DoubleFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	BigInt2DoubleFilter() {}

	double valueAt(int row) const override {
		if (!m_inputs.value(0)) return 0;

		qint64 value = m_inputs.value(0)->bigIntAt(row);
		double result = (double)value;
		//DEBUG("BigInt2Double::integerAt() " << value << " -> " << result);

		return result;
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Numeric; }

protected:
	//! Using typed ports: only bigint inputs are accepted
	bool inputAcceptable(int, const AbstractColumn *source) override {
		DEBUG("inputAcceptable(): source type = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, source->columnMode()));
		return source->columnMode() == AbstractColumn::ColumnMode::BigInt;
	}
};

#endif // ifndef BIGINT2DOUBLE_FILTER_H
