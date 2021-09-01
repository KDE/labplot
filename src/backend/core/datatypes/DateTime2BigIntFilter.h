/*
    File                 : DateTime2BigIntFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Conversion filter QDateTime -> bigint (using Julian day).

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef DATE_TIME2BIGINT_FILTER_H
#define DATE_TIME2BIGINT_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>

//! Conversion filter QDateTime -> bigint (using Julian day).
class DateTime2BigIntFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	qint64 bigIntAt(int row) const override {
		//DEBUG("bigIntAt()");
		if (!m_inputs.value(0)) return 0;
		QDateTime inputDate = m_inputs.value(0)->dateTimeAt(row);
		if (!inputDate.isValid()) return 0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
		QDateTime start(QDate(1900, 1, 1).startOfDay());
#else
		QDateTime start(QDate(1900, 1, 1));
#endif
		return qint64(start.daysTo(inputDate));
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::BigInt; }

protected:
	//! Using typed ports: only DateTime inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::DateTime;
	}
};

#endif // ifndef DATE_TIME2BIGINT_FILTER_H

