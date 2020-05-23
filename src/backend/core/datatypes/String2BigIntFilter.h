/***************************************************************************
    File                 : String2BigIntFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Locale-aware conversion filter QString -> bigint.

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef STRING2BIGINT_FILTER_H
#define STRING2BIGINT_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>

//! Locale-aware conversion filter QString -> int.
class String2BigIntFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	String2BigIntFilter() : m_use_default_locale(true) {}
	void setNumericLocale(const QLocale& locale) { m_numeric_locale = locale; m_use_default_locale = false; }
	void setNumericLocaleToDefault() { m_use_default_locale = true; }

	qint64 bigIntAt(int row) const override {
		//DEBUG("String2BigInt::bigIntAt()");
		if (!m_inputs.value(0)) return 0;

		qint64 result;
		bool valid;
		QString textValue = m_inputs.value(0)->textAt(row);
		//DEBUG("	textValue = " << STDSTRING(textValue));
		if (m_use_default_locale) // we need a new QLocale instance here in case the default changed since the last call
			result = QLocale().toLongLong(textValue, &valid);
		else
			result = m_numeric_locale.toLongLong(textValue, &valid);
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

private:
	QLocale m_numeric_locale;
	bool m_use_default_locale;
};

#endif // ifndef STRING2BIGINT_FILTER_H
