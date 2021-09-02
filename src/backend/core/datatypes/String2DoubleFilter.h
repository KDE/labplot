/*
    File                 : String2DoubleFilter.h
    Project              : AbstractColumn
    Description          : Locale-aware conversion filter QString -> double.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STRING2DOUBLE_FILTER_H
#define STRING2DOUBLE_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>
#include <cmath>

//! Locale-aware conversion filter QString -> double.
class String2DoubleFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	String2DoubleFilter() {}

	double valueAt(int row) const override {
		//DEBUG("String2Double::valueAt()");

		if (!m_inputs.value(0))
			return 0;

		double result;
		bool valid;
		if (m_useDefaultLocale) // we need a new QLocale instance here in case the default changed since the last call
			result = QLocale().toDouble(m_inputs.value(0)->textAt(row), &valid);
		else
			result = m_numberLocale.toDouble(m_inputs.value(0)->textAt(row), &valid);

		if (valid)
			return result;
		return NAN;
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Numeric; }

protected:
	//! Using typed ports: only string inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Text;
	}
};

#endif // ifndef STRING2DOUBLE_FILTER_H
