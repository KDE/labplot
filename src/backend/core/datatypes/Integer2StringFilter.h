/*
    File                 : Integer2StringFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017-2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Locale-aware conversion filter int -> QString.
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef INTEGER2STRING_FILTER_H
#define INTEGER2STRING_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>

//! Locale-aware conversion filter int -> QString.
class Integer2StringFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	explicit Integer2StringFilter() {}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Text; }

public:
	QString textAt(int row) const override {
		if (!m_inputs.value(0)) return QString();
		if (m_inputs.value(0)->rowCount() <= row) return QString();

		int inputValue = m_inputs.value(0)->integerAt(row);

		if (m_useDefaultLocale)
			return QLocale().toString(inputValue);
		else
			return m_numberLocale.toString(inputValue);
	}

protected:
	//! Using typed ports: only double inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Integer;
	}
};

#endif // INTEGER2STRING_FILTER_H

