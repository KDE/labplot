/***************************************************************************
    File                 : String2DoubleFilter.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
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
#include "lib/XmlStreamReader.h"
#include <QXmlStreamWriter>

//! Locale-aware conversion filter QString -> double.
class String2DoubleFilter : public AbstractSimpleFilter
{
	Q_OBJECT

	public:
		String2DoubleFilter() : m_use_default_locale(true) {}
		void setNumericLocale(QLocale locale) { m_numeric_locale = locale; m_use_default_locale = false; }
		void setNumericLocaleToDefault() { m_use_default_locale = true; }

		virtual double valueAt(int row) const {
			if (!m_inputs.value(0)) return 0;
			if (m_use_default_locale) // we need a new QLocale instance here in case the default changed since the last call
				return QLocale().toDouble(m_inputs.value(0)->textAt(row));
			return m_numeric_locale.toDouble(m_inputs.value(0)->textAt(row));
		}
		virtual bool isInvalid(int row) const { 
			if (!m_inputs.value(0)) return false;
			bool ok;
			if (m_use_default_locale)
				QLocale().toDouble(m_inputs.value(0)->textAt(row), &ok);
			else
				m_numeric_locale.toDouble(m_inputs.value(0)->textAt(row), &ok);
			return !ok;
		}
		virtual bool isInvalid(Interval<int> i) const {
			if (!m_inputs.value(0)) return false;
			QLocale locale;
			if (!m_use_default_locale)
				locale = m_numeric_locale;
			for (int row = i.start(); row <= i.end(); row++) {
				bool ok;
				locale.toDouble(m_inputs.value(0)->textAt(row), &ok);
				if (ok)
					return false;
			}
			return true;
		}
		virtual QList< Interval<int> > invalidIntervals() const 
		{
			IntervalAttribute<bool> validity;
			if (m_inputs.value(0)) {
				int rows = m_inputs.value(0)->rowCount();
				for (int i=0; i<rows; i++) 
					validity.setValue(i, isInvalid(i));
			}
			return validity.intervals();
		}


		//! Return the data type of the column
		virtual SciDAVis::ColumnMode columnMode() const { return SciDAVis::Numeric; }

	protected:
		//! Using typed ports: only string inputs are accepted.
		virtual bool inputAcceptable(int, const AbstractColumn *source) {
			return source->columnMode() == SciDAVis::Text;
		}

	private:
		QLocale m_numeric_locale;
		bool m_use_default_locale;
};

#endif // ifndef STRING2DOUBLE_FILTER_H

