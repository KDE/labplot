/*
    File                 : Double2StringFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke (knut.franke*gmx.de)
    SPDX-FileCopyrightText: 2007 Tilman Benkert (thzs@gmx.net)
    SPDX-FileCopyrightText: 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Locale-aware conversion filter double -> QString.

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Text; }

private:
	friend class Double2StringFilterSetFormatCmd;
	friend class Double2StringFilterSetDigitsCmd;
	//! Format character as in QString::number
	char m_format;
	//! Display digits or precision as in QString::number
	int m_digits;

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

