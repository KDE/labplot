/*
    File                 : String2IntegerFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017-2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Locale-aware conversion filter QString -> int.
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STRING2INTEGER_FILTER_H
#define STRING2INTEGER_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>

//! Locale-aware conversion filter QString -> int.
class String2IntegerFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	explicit String2IntegerFilter() {}

	int integerAt(int row) const override {
		//DEBUG("String2Integer::integerAt()");
		if (!m_inputs.value(0)) return 0;

		int result;
		bool valid;
		QString textValue = m_inputs.value(0)->textAt(row);
		//DEBUG("	textValue = " << STDSTRING(textValue));
		if (m_useDefaultLocale)
			result = QLocale().toInt(textValue, &valid);
		else
			result = m_numberLocale.toInt(textValue, &valid);
		//DEBUG("	result = " << result << " valid = " << valid);

		if (valid)
			return result;
		return 0;
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Integer; }

protected:
	//! Using typed ports: only string inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Text;
	}
};

#endif // ifndef STRING2INTEGER_FILTER_H
