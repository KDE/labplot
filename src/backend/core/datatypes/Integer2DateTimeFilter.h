/*
	File                 : Integer2DateTimeFilter.h
	Project              : AbstractColumn
	Description          : Conversion filter int -> QDateTime, interpreting
	the input numbers as Julian days.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
	SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTEGER2DATE_TIME_FILTER_H
#define INTEGER2DATE_TIME_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QDateTime>
#include <QTimeZone>

//! Conversion filter int -> QDateTime, interpreting the input as seconds since epoch
class Integer2DateTimeFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	QDateTime dateTimeAt(int row) const override {
		DEBUG(Q_FUNC_INFO)
		QDateTime dt = QDateTime::fromSecsSinceEpoch(0, QTimeZone::UTC);
		int inputValue = m_inputs.value(0)->integerAt(row);
		return dt.addMSecs(inputValue); // TODO: select unit (ms, s, min, hour, days)
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override {
		return AbstractColumn::ColumnMode::DateTime;
	}

protected:
	//! Using typed ports: only double inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn* source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Integer;
	}
};

#endif // ifndef INTEGER2DATE_TIME_FILTER_H
