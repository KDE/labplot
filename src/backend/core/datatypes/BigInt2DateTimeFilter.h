/*
	File                 : BigInt2DateTimeFilter.h
	Project              : AbstractColumn
	Description          : Conversion filter int -> QDateTime, interpreting
	the input numbers as Julian days.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007 Knut Franke Tilman Benkert <knut.franke*gmx.de, thzs@gmx.net (use @ for *)>
	SPDX-FileCopyrightText: 2020-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BIGINT2DATE_TIME_FILTER_H
#define BIGINT2DATE_TIME_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>
#include <QTimeZone>

//! Conversion filter bigint -> QDateTime, interpreting the input numbers as Julian days.
class BigInt2DateTimeFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	QDateTime dateTimeAt(int row) const override {
		QDateTime dt = QDateTime::fromSecsSinceEpoch(0, QTimeZone::UTC);
		qint64 inputValue = m_inputs.value(0)->bigIntAt(row);
		return dt.addMSecs(inputValue); // TODO: select unit (ms, s, min, hour, days)
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override {
		return AbstractColumn::ColumnMode::DateTime;
	}

protected:
	//! Using typed ports: only double inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn* source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::BigInt;
	}
};

#endif // ifndef BIGINT2DATE_TIME_FILTER_H
