/***************************************************************************
    File                 : String2IntegerFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Locale-aware conversion filter QString -> int.

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
#ifndef STRING2INTEGER_FILTER_H
#define STRING2INTEGER_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>

//! Locale-aware conversion filter QString -> int.
class String2IntegerFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	String2IntegerFilter() : m_use_default_locale(true) {}
	void setNumericLocale(const QLocale& locale) { m_numeric_locale = locale; m_use_default_locale = false; }
	void setNumericLocaleToDefault() { m_use_default_locale = true; }

	int integerAt(int row) const override {
		//DEBUG("String2Integer::integerAt()");
		if (!m_inputs.value(0)) return 0;

		int result;
		bool valid;
		QString textValue = m_inputs.value(0)->textAt(row);
		//DEBUG("	textValue = " << textValue.toStdString());
		if (m_use_default_locale) // we need a new QLocale instance here in case the default changed since the last call
			result = QLocale().toInt(textValue, &valid);
		else
			result = m_numeric_locale.toInt(textValue, &valid);
		//DEBUG("	result = " << result << " valid = " << valid);

		if (valid)
			return result;
		return 0;
	}

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::Integer; }

protected:
	//! Using typed ports: only string inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::Text;
	}

private:
	QLocale m_numeric_locale;
	bool m_use_default_locale;
};

#endif // ifndef STRING2INTEGER_FILTER_H
