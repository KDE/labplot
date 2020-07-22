/***************************************************************************
    File                 : Integer2StringFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2017-2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Locale-aware conversion filter int -> QString.
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
#ifndef INTEGER2STRING_FILTER_H
#define INTEGER2STRING_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>

//! Locale-aware conversion filter int -> QString.
class Integer2StringFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	explicit Integer2StringFilter() {}
	void setNumberLocale(const QLocale& locale) { m_numberLocale = locale; m_useDefaultLocale = false; }
	void setNumberLocaleToDefault() { m_useDefaultLocale = true; }

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

private:
	QLocale m_numberLocale;
	bool m_useDefaultLocale{true};
};

#endif // INTEGER2STRING_FILTER_H

