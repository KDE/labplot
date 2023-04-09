/*
	File                 : MonthDoubleFilter.h
	Project              : AbstractColumn
	Description          : Conversion filter QDateTime -> double, translating
	dates into months (January -> 1).
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
	SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

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
		if (!m_inputs.value(0))
			return NAN;
		QDate inputValue = m_inputs.value(0)->dateAt(row);
		if (!inputValue.isValid())
			return NAN;
		return double(inputValue.month());
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override {
		return AbstractColumn::ColumnMode::Double;
	}

protected:
	//! Using typed ports: only date-time inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn* source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Month;
	}
};

#endif // ifndef MONTH2DOUBLE_FILTER_H
