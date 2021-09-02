/*
    File                 : DateTime2DoubleFilter.h
    Project              : LabPlot
    Description          : Conversion filter QDateTime -> double (using Julian day).
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke@gmx.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATE_TIME2DOUBLE_FILTER_H
#define DATE_TIME2DOUBLE_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>
#include <cmath>

//! Conversion filter QDateTime -> double (using Julian day).
class DateTime2DoubleFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	double valueAt(int row) const override {
		if (!m_inputs.value(0)) return NAN;
		QDateTime inputDate = m_inputs.value(0)->dateTimeAt(row);
		if (!inputDate.isValid()) return NAN;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
		QDateTime start(QDate(1900, 1, 1).startOfDay());
#else
		QDateTime start(QDate(1900, 1, 1));
#endif
		return double(start.daysTo(inputDate)) +
			double( -inputDate.time().msecsTo(QTime(0,0,0,0)) ) / 86400000.0;
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Numeric; }

protected:
	//! Using typed ports: only DateTime inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn* source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::DateTime;
	}
};

#endif // ifndef DATE_TIME2DOUBLE_FILTER_H

