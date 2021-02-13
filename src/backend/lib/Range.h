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
#include "backend/nsl/nsl_math.h"
}

#include "macros.h"	//SET_NUMBER_LOCALE
#include <klocalizedstring.h>
#include <QStringList>

class QString;

//! Auxiliary class for a data range 
/**
 *	This class represents a data range [start, end] where start can be > end.
 *	It replaces the Interval class and has following attributes:
 *	* start
 *	* end
 *	* format (Numeric or Datetime)
 *	* datetime format ("YY-MM-DD ...")
 *	* scale (Linear, Log, ...)
 *
 *	Only types supporting comparison are supported
 */
class RangeT {	// access enum without template
public:
	enum class Format {Numeric, DateTime};
	// see https://www.originlab.com/doc/Origin-Help/AxesRef-Scale#Type
	// TODO: Reciprocal, ReciprocalOffset, Probability, Probit, Logit, Weibull
	enum class Scale {Linear, Log10, Log2, Ln, Sqrt, Square};
	const static QStringList scaleNames; // see Range.cpp
	//TODO: when we have C++17: use inline initialization
//	const static inline QStringList scaleNames{ i18n("Linear"), i18n("Log10"), i18n("Log2"), i18n("Ln"), i18n("Sqrt"), i18n("Square") };
};

template<class T>
class Range : RangeT {
public:

	Range() : m_start(0), m_end(1), m_format(Format::Numeric), m_scale(Scale::Linear) {}
	Range(T start, T end, Format format = Format::Numeric, Scale scale = Scale::Linear) {
		this->setRange(start, end, format, scale);
	}
	Range(const QString& start, const QString& end, const Format format = Format::Numeric,
	      const Scale scale = Scale::Linear) {
		SET_NUMBER_LOCALE
		//TODO: check for NAN, INF?
		this->setRange(parse(qPrintable(start.simplified()), qPrintable(numberLocale.name())),
				parse(qPrintable(end.simplified()), qPrintable(numberLocale.name())),
				format, scale);
	}

	T start() const { return m_start; }
	T end() const { return m_end; }
	T& start() { return m_start; }
	T& end() { return m_end; }
	Format format() const { return m_format; }
	Format& format() { return m_format; }
	Scale scale() const { return m_scale; }
	Scale& scale() { return m_scale; }
	QString dateTimeFormat() const { return m_dateTimeFormat; }
	const QString& dateTimeFormat() { return m_dateTimeFormat; }
	bool autoScale() const { return m_autoScale; }

	void setStart(T start) { m_start = start; }
	void setEnd(T end) { m_end = end; }
	void setRange(T start, T end, Format format = Format::Numeric,
		      Scale scale = Scale::Linear) {
		m_start = start;
		m_end = end;
		m_format = format;
		m_scale = scale;
	}
	void setFormat(Format format) { m_format = format; }
	void setScale(Scale scale) { m_scale = scale; }
	void setDateTimeFormat(QString format) { m_dateTimeFormat = format; }
	void setAutoScale(bool b) { m_autoScale = b; }

	T size() const { return m_end - m_start; }
	T length() const { return qAbs(m_end - m_start); }
	T center() const { return (m_start + m_end)/2; }
	// calculate step size from number of steps
	T stepSize(const int steps) const { return (steps > 1) ? size()/static_cast<T>(steps - 1) : 0; }
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
	QString toString() const {
		if (m_format == Format::Numeric)
			return "[" + QLocale().toString(m_start) + "; " + QLocale().toString(m_end) + "]";
		else
			return QDateTime::fromMSecsSinceEpoch(m_start).toString(m_dateTimeFormat) + " - "
				+ QDateTime::fromMSecsSinceEpoch(m_end).toString(m_dateTimeFormat);
	}
	std::string toStdString() const { return STDSTRING(toString()); }
	//extend range to nice numbers (used in auto scaling)
	void niceExtend() {
		if (length() == 0)
			return;
		DEBUG(Q_FUNC_INFO << ", range = " << toStdString() << ", size = " << size())
		//DEBUG(Q_FUNC_INFO << ", size/10 = " << size()/10.)
		const int places = nsl_math_rounded_decimals(size()/10.);
		DEBUG(Q_FUNC_INFO << ", decimal places = " << (int)places)
		if (m_start < m_end) {
			m_start = nsl_math_floor_places(m_start, places);
			m_end = nsl_math_ceil_places(m_end, places);
		} else {
			m_start = nsl_math_ceil_places(m_start, places);
			m_end = nsl_math_floor_places(m_end, places);
		}
		DEBUG(Q_FUNC_INFO << ", new range = " << toStdString())
	}
	//TODO: touches(), merge(), subtract(), split(), etc. (see Interval)

private:
	T m_start;	// start value
	T m_end;	// end value
	Format m_format{Format::Numeric};	// format (Numeric or DateTime)
	QString m_dateTimeFormat{"yyyy-MM-dd hh:mm:ss"};	// only used for DateTime
	Scale m_scale{Scale::Linear};	// scale (Linear, Log , ...)
	bool m_autoScale{true};	// auto adapt start and end to all curves using this range (in plot)
};

#endif

