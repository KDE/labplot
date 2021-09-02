/*
    File                 : BigInt2StringFilter.h
    Project              : AbstractColumn
    Description          : Locale-aware conversion filter bigint -> QString.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BIGINT2STRING_FILTER_H
#define BIGINT2STRING_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>

//! Locale-aware conversion filter bigint -> QString.
class BigInt2StringFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	explicit BigInt2StringFilter() {}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Text; }

public:
	QString textAt(int row) const override {
		if (!m_inputs.value(0)) return QString();
		if (m_inputs.value(0)->rowCount() <= row) return QString();

		qint64 inputValue = m_inputs.value(0)->bigIntAt(row);

		if (m_useDefaultLocale)
			return QLocale().toString(inputValue);
		else
			return m_numberLocale.toString(inputValue);
	}

protected:
	//! Using typed ports: only bigint inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::BigInt;
	}
};

#endif // BIGINT2STRING_FILTER_H

