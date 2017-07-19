/***************************************************************************
    File                 : Integer2DoubleFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Locale-aware conversion filter int -> double.

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
#ifndef INTEGER2DOUBLE_FILTER_H
#define INTEGER2DOUBLE_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>

//! Locale-aware conversion filter double -> int.
class Integer2DoubleFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	Integer2DoubleFilter() {}

	virtual double valueAt(int row) const {
		DEBUG("valueAt()");
		if (!m_inputs.value(0)) return 0;

		double result;
		result = (double)(m_inputs.value(0)->integerAt(row));

		return result;
	}

	//! Return the data type of the column
	virtual AbstractColumn::ColumnMode columnMode() const { return AbstractColumn::Numeric; }

protected:
	//! Using typed ports: only integer inputs are accepted.
	virtual bool inputAcceptable(int, const AbstractColumn *source) {
		return source->columnMode() == AbstractColumn::Integer;
	}
};

#endif // ifndef INTEGER2DOUBLE_FILTER_H
