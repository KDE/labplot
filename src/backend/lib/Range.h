/***************************************************************************
    File                 : Range.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2020-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include <QtMath>
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
	// TODO: InverseOffset, Prob, Probit, Logit, Weibull
	enum class Scale {Linear, Log10, Log2, Ln, Sqrt, Square, Inverse};
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
			return QLocale().toString(m_start) + " .. " + QLocale().toString(m_end);
			//return "[" + QLocale().toString(m_start) + "; " + QLocale().toString(m_end) + "]";
		else
			return QDateTime::fromMSecsSinceEpoch(m_start).toString(m_dateTimeFormat) + " .. "
				+ QDateTime::fromMSecsSinceEpoch(m_end).toString(m_dateTimeFormat);
	}
	std::string toStdString() const { return STDSTRING(toString()); }
//extend/shrink range to nice numbers (used in auto scaling)
	// get nice size to extend to (see Glassner: Graphic Gems)
	double niceSize(double size, bool round) {
		const double exponent = qFloor(log10(size));
		const double fraction = size / qPow(10, exponent);

		double niceFraction; // nice (rounded) fraction
		if (round) {
			if (fraction < 1.5)
				niceFraction = 1;
			else if (fraction < 3)
				niceFraction = 2;
			else if (fraction < 7)
				niceFraction = 5;
			else
				niceFraction = 10;
		} else {
			if (fraction <= 1)
				niceFraction = 1;
			else if (fraction <= 2)
				niceFraction = 2;
			else if (fraction <= 5)
				niceFraction = 5;
			else
				niceFraction = 10;
		}
		DEBUG(Q_FUNC_INFO << ", fraction = " << fraction);
		DEBUG(Q_FUNC_INFO << ", nice fraction = " << niceFraction);

		return niceFraction * qPow(10, exponent);
	}
	void niceShrink() { niceExtend(false); }
	void niceExtend(bool extend = true) {	// extend == false means shrink
		if (length() == 0)
			return;

		const double newSize = niceSize(size(), false);
		DEBUG(Q_FUNC_INFO << ", new size = " << newSize);
		const double maxTicks = 10;	// TODO: parameter?
		const double spacing = niceSize(newSize / (maxTicks - 1), true);
		if ((extend && m_start < m_end) || (!extend && m_start > m_end)) {
			m_start = qFloor(m_start / spacing) * spacing;
			m_end = qCeil(m_end / spacing) * spacing;
		} else {
			m_start = qCeil(m_start / spacing) * spacing;
			m_end = qFloor(m_end / spacing) * spacing;
		}
	}
/* Old version
 * void niceExtend(bool extend = true) {	// extend == false means shrink
		if (length() == 0)
			return;
		DEBUG(Q_FUNC_INFO << ", range : " << toStdString() << ", size = " << size())
		DEBUG(Q_FUNC_INFO << ", size/10 = " << size()/10.)
		int places = nsl_math_rounded_decimals(size()/10.);
		DEBUG(Q_FUNC_INFO << ", decimal places (rounded) = " << (int)places)
		const double scaledSize = size() * pow(10., places);	// range: 4.5 - 44.9
		DEBUG(Q_FUNC_INFO << ", scaled size  = " << scaledSize)

		// keep certain sizes that can be handled by autoTickCount()
		if ( qFuzzyCompare(scaledSize, 10.5) )
			return;

		double factor = 1.;
		// use special rounding for certain values
		if ( (scaledSize > 32. && scaledSize < 35.)
			|| (scaledSize > 42. && scaledSize < 45.) ) {	// 0.5 steps
			factor = 2.;
			places -= 1;
		} else if (scaledSize > 7.2 && scaledSize < 7.5) {
			factor = 2.;
		} else if ( (scaledSize > 12. && scaledSize < 12.5)
			|| (scaledSize > 16. && scaledSize < 17.5)
			|| (scaledSize > 21. && scaledSize < 22.5)
			|| (scaledSize > 36. && scaledSize < 40.) ) {	// 0.25 steps
			factor = 4.;
			places -= 1;
		} else if ( (scaledSize > 10. && scaledSize < 12.)
			|| (scaledSize > 12.5 && scaledSize < 14.)
			|| (scaledSize > 17.5 && scaledSize < 20.)
			|| (scaledSize > 22.5 && scaledSize < 24.)
			|| (scaledSize > 27. && scaledSize < 32.)
			|| (scaledSize > 35. && scaledSize < 36.)
			|| (scaledSize > 40. && scaledSize < 42.) ) {	// 0.2 steps
			factor = 5.;
			places -= 1;
		} else if ( (scaledSize > 4.6 && scaledSize < 5.)
                        || (scaledSize > 5.2 && scaledSize < 5.6)
			|| (scaledSize > 6.2 && scaledSize < 6.4)
                        || (scaledSize > 7. && scaledSize < 7.2)
			|| (scaledSize > 8.2 && scaledSize < 8.4)
			|| (scaledSize > 9.4 && scaledSize < 9.6) ) {
			factor = 5.;
		} else if (scaledSize > 8. && scaledSize < 8.1) {	// .1 steps
			factor = 10.;
		} else if (scaledSize > 25. && scaledSize < 26.) {	// .3 steps -> 27.
			factor = 1./.3;
			places -= 1;
		} else if ( (scaledSize > 4.5 && scaledSize < 4.6)	// .3 steps -> 4.8
			|| (scaledSize > 6 && scaledSize < 6.2)		// -> 6.3
			|| (scaledSize > 8.1 && scaledSize < 8.2) ) {	// -> 8.4
			factor = 1./.3;
		} else if ( (scaledSize > 5. && scaledSize < 5.2)	// .6 steps -> 5.4
			|| (scaledSize > 9. && scaledSize < 9.4) ) {	// -> 9.6
			factor = 1./.6;
		}
		DEBUG(Q_FUNC_INFO << ", factor = " << factor << ", places = " << places)

		// round to decimal places
		if ((extend && m_start < m_end) || (!extend && m_start > m_end)) {
			m_start = nsl_math_floor_places(factor * m_start, places);
			m_end = nsl_math_ceil_places(factor * m_end, places);
		} else {
			m_start = nsl_math_ceil_places(factor * m_start, places);
			m_end = nsl_math_floor_places(factor * m_end, places);
		}
		m_start /= factor;
		m_end /= factor;

		DEBUG(Q_FUNC_INFO << ", new range : " << toStdString())
	}
*/
	int autoTickCount() const {
		if (length() == 0)
			return 0;

		DEBUG(Q_FUNC_INFO << ", range = " << toStdString() << ", size = " << size())
		const double order = pow(10.0, qFloor(log10(size())));;
		DEBUG(Q_FUNC_INFO << ", order of magnitude = " << order)
		const int factor = qRound(100 * size() / order);
		DEBUG(Q_FUNC_INFO << ", factor = " << factor)

		// set number of ticks for certain multiple of small numbers
		if (factor % 30 == 0)
			return 3+1;
		if (factor % 40 == 0)
			return 4+1;
		if (factor % 70 == 0)
			return 7+1;
		if (factor % 50 == 0)
			return 5+1;
		if (factor % 90 == 0)
			return 9+1;
		if (factor % 175 == 0)
			return 7+1;
		if (factor % 25 == 0)
			return 5+1;
		if (factor % 105 == 0)
			return 7+1;
		if (factor % 115 == 0)
			return 5+1;

		return 7+1;
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

