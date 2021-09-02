/*
    File                 : String2BigIntFilter.h
    Project              : AbstractColumn
    Description          : Locale-aware conversion filter QString -> bigint.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STRING2BIGINT_FILTER_H
#define STRING2BIGINT_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>

//! Locale-aware conversion filter QString -> int.
class String2BigIntFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	String2BigIntFilter() {}

	qint64 bigIntAt(int row) const override {
		//DEBUG("String2BigInt::bigIntAt()");
		if (!m_inputs.value(0)) return 0;

		qint64 result;
		bool valid;
		QString textValue = m_inputs.value(0)->textAt(row);
		//DEBUG("	textValue = " << STDSTRING(textValue));
		if (m_useDefaultLocale) // we need a new QLocale instance here in case the default changed since the last call
			result = QLocale().toLongLong(textValue, &valid);
		else
			result = m_numberLocale.toLongLong(textValue, &valid);
		//DEBUG("	result = " << result << " valid = " << valid);

		if (valid)
			return result;
		return 0;
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::BigInt; }

protected:
	//! Using typed ports: only string inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Text;
	}
};

#endif // ifndef STRING2BIGINT_FILTER_H
