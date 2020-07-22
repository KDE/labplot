/***************************************************************************
    File                 : String2DoubleFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2020 by Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Locale-aware conversion filter QString -> double.

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
	void setNumberLocale(const QLocale& locale) { m_numberLocale = locale; m_useDefaultLocale = false; }
	void setNumberLocaleToDefault() { m_useDefaultLocale = true; }

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

private:
	QLocale m_numberLocale;
	bool m_useDefaultLocale{true};
};

#endif // ifndef STRING2DOUBLE_FILTER_H
