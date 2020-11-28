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

#include <QString>
#include <cmath>

//! Auxiliary class for a data range 
/**
 *	This class represents a data range [left, right] with right >= left.
 *
 *	Only types supporting comparison are supported
 */
template<class T>
class Range {
public:
	Range() : m_min(0), m_max(0) {}
	Range(T min, T max) {
		this->setRange(min, max);
	}
	Range(const QString& min, const QString& max) {
		SET_NUMBER_LOCALE
		//TODO: check for NAN, INF?
		this->setRange(parse(qPrintable(min.simplified()), qPrintable(numberLocale.name())), parse(qPrintable(max.simplified()), qPrintable(numberLocale.name())));
	}
	~Range() = default;
	T min() const { return m_min; }
	T max() const { return m_max; }
	T& min() { return m_min; }
	T& max() { return m_max; }
	void setMin(T min) { m_min = min; }	// no check (use first)
	void setMax(T max) { m_max = std::max(m_min, max); }
	void setRange(T min, T max) {
		m_min = min;
		m_max = std::max(min, max);
	}
	T size() const { return m_max - m_min; }
	T length() const { return fabs(m_max - m_min); }
	// calculate step size from number of steps
	T stepSize(const int steps) const { return (steps > 1) ? size()/(T)(steps - 1) : 0; }
	bool isZero() const { return (m_max == m_min); }
	bool inside(const Range<T>& other) const { return ( m_min <= other.min() && m_max >= other.max() ); }
	bool inside(T value) const { return ( m_min <= value && m_max >= value ); }
	void translate(T offset) { m_min += offset; m_max += offset; }
	bool operator==(const Range<T>& other) const { return ( m_min == other.min() && m_max == other.max() ); }
	bool operator!=(const Range<T>& other) const { return ( m_min != other.min() || m_max != other.max() ); }
	Range<T>& operator=(const Range<T>& other) = default;

	//! Return a string in the format '[min, max]'
	QString toString() const {
		return "[" + QLocale().toString(m_min) + ", " + QLocale().toString(m_max) + "]";
	}
	//TODO: touches(), merge(), subtract(), split(), etc. (see Interval)

private:
	T m_min;	// lower limit
	T m_max;	// upper limit
};

#endif

