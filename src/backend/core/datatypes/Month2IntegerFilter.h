/*
    File                 : Month2IntegerFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Stefan Gerlach <stefan.gerlach@uni.kn>
    Description          : Conversion filter QDateTime -> double, translating
    dates into months (January -> 1).

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MONTH2INTEGER_FILTER_H
#define MONTH2INTEGER_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDate>

/**
 * \brief Conversion filter QDateTime -> int, translating dates into months (January -> 1).
 *
 * \sa QDate::month()
 */
class Month2IntegerFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	int integerAt(int row) const override {
		DEBUG("integerAt()");
		if (!m_inputs.value(0)) return 0;
		QDate inputValue = m_inputs.value(0)->dateAt(row);
		if (!inputValue.isValid()) return 0;
		return int(inputValue.month());
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Integer; }

protected:
	//! Using typed ports: only date-time inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Month;
	}
};

#endif // ifndef MONTH2INTEGER_FILTER_H

