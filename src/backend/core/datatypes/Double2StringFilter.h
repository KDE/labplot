/***************************************************************************
    File                 : Double2StringFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2007 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2020 by Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Locale-aware conversion filter double -> QString.

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
#ifndef DOUBLE2STRING_FILTER_H
#define DOUBLE2STRING_FILTER_H

#include "../AbstractSimpleFilter.h"
#include <QLocale>
#include <cmath>

//! Locale-aware conversion filter double -> QString.
class Double2StringFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	explicit Double2StringFilter(char format='g', int digits = 6) : m_format(format), m_digits(digits) {}
	//! Set format character as in QString::number
	void setNumericFormat(char format);
	//! Set number of displayed digits
	void setNumDigits(int digits);
	//! Get format character as in QString::number
	char numericFormat() const { return m_format; }
	//! Get number of displayed digits
	int numDigits() const { return m_digits; }
	void setNumberLocale(const QLocale& locale) { m_numberLocale = locale; m_useDefaultLocale = false; }
	void setNumberLocaleToDefault() { m_useDefaultLocale = true; }

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Text; }

private:
	friend class Double2StringFilterSetFormatCmd;
	friend class Double2StringFilterSetDigitsCmd;
	//! Format character as in QString::number
	char m_format;
	//! Display digits or precision as in QString::number
	int m_digits;
	QLocale m_numberLocale;
	bool m_useDefaultLocale{true};

	//! \name XML related functions
	//@{
	void writeExtraAttributes(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	//@}

public:
	QString textAt(int row) const override {
		//DEBUG("Double2String::textAt()");
		if (!m_inputs.value(0)) return QString();
		if (m_inputs.value(0)->rowCount() <= row) return QString();
		double inputValue = m_inputs.value(0)->valueAt(row);
		if (std::isnan(inputValue)) return QString();
		if (m_useDefaultLocale)
			return QLocale().toString(inputValue, m_format, m_digits);
		else
			return m_numberLocale.toString(inputValue, m_format, m_digits);
	}

protected:
	//! Using typed ports: only double inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override {
		return source->columnMode() == AbstractColumn::ColumnMode::Numeric;
	}
};

#endif // DOUBLE2STRING_FILTER_H

