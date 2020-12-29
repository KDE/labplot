/***************************************************************************
    File                 : Range.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : basic data range class

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

#ifndef RANGE_H
#define RANGE_H

extern "C" {
#include "backend/gsl/parser.h"
}

#include "backend/lib/macros.h"	//SET_NUMBER_LOCALE

class QString;

//! Auxiliary class for a data range 
/**
 *	This class represents a data range [start, end] where start can be > end.
 *	It replaces the Interval class and has following attributes:
 *	* start
 *	* end
 *	* format (Numeric or Datetime)
 *
 *	Only types supporting comparison are supported
 */
class RangeT {	// access enum without template
public:
	enum class Format {Numeric, DateTime};
};

template<class T>
class Range : RangeT {
public:

	Range() : m_start(0), m_end(1), m_format(Format::Numeric) {}
	Range(T start, T end, Format format = Format::Numeric) {
		this->setRange(start, end, format);
	}
	Range(const QString& start, const QString& end, const Format format = Format::Numeric) {
		SET_NUMBER_LOCALE
		//TODO: check for NAN, INF?
		this->setRange(parse(qPrintable(start.simplified()), qPrintable(numberLocale.name())),
				parse(qPrintable(end.simplified()), qPrintable(numberLocale.name())),
				format);
	}

	T start() const { return m_start; }
	T end() const { return m_end; }
	Format format() const { return m_format; }
	T& start() { return m_start; }
	T& end() { return m_end; }
	Format& format() { return m_format; }
	void setMin(T start) { m_start = start; }
	void setMax(T end) { m_end = end; }
	void setRange(T start, T end, Range::Format format = Format::Numeric) {
		m_start = start;
		m_end = end;
		m_format = format;
	}
	void setFormat(Format format) { m_format = format; }

	T size() const { return m_end - m_start; }
	T length() const { return qAbs(m_end - m_start); }
	T center() const { return (m_start + m_end)/2; }
	// calculate step size from number of steps
	T stepSize(const int steps) const { return (steps > 1) ? size()/(T)(steps - 1) : 0; }
	bool isZero() const { return ( m_end == m_start ); }
	bool valid() const { return ( !qIsNaN(m_start) && !qIsNaN(m_end) ); }
	bool finite() const { return ( qIsFinite(m_start) && qIsFinite(m_end) ); }
	bool inverse() const { return (m_start > m_end); }
	bool contains(const Range<T>& other) const { return ( qMin(m_start, m_end) <= qMin(other.start(), other.end()) && qMax(m_start, m_end) >= qMax(other.start(), other.end()) ); }
	bool contains(T value) const { return ( qMin(m_start, m_end) <= value && qMax(m_start, m_end) >= value ); }
	void translate(T offset) { m_start += offset; m_end += offset; }
	void extend(T value) { m_start -= value; m_end += value; }
	bool operator==(const Range<T>& other) const { return ( m_start == other.start() && m_end == other.end() ); }
	bool operator!=(const Range<T>& other) const { return ( m_start != other.start() || m_end != other.end() ); }
	Range<T>& operator+=(const T value) { m_start += value; m_end += value; return *this; }
	Range<T>& operator*=(const T value) { m_start *= value; m_end *= value; return *this; }

	//! Return a string in the format '[start, end]'
	//TODO: DateTime format
	QString toString() const {
		return "[" + QLocale().toString(m_start) + ", " + QLocale().toString(m_end) + "]";
	}
	const char* toStdString() const { return STDSTRING(toString()); }
	//TODO: touches(), merge(), subtract(), split(), etc. (see Interval)

private:
	T m_start;	// start value
	T m_end;	// upper limit
	Range::Format m_format;	// format (Numeric or DateTime)
};

#endif

