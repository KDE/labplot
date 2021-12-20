/*
    File                 : Range.h
    Project              : LabPlot
    Description          : basic data range class
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RANGE_H
#define RANGE_H

#include "macros.h"	//SET_NUMBER_LOCALE

extern "C" {
#include "backend/gsl/parser.h"
#include "backend/nsl/nsl_math.h"
}

#include <klocalizedstring.h>
#include <QStringList>
#include <cmath>

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
	Q_GADGET
public:
	enum class Format {Numeric, DateTime};
	Q_ENUM(Format)
	// see https://www.originlab.com/doc/Origin-Help/AxesRef-Scale#Type
	// TODO: InverseOffset, Prob, Probit, Logit, Weibull
	enum class Scale {Linear, Log10, Log2, Ln, Sqrt, Square, Inverse};
	Q_ENUM(Scale)
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
		// min
		double min = parse(qPrintable(start.simplified()), qPrintable(numberLocale.name()));
		if (parse_errors() > 0)	// if parsing fails, try default locale
			min = parse(qPrintable(start.simplified()), "en_US");
		if (parse_errors() > 0)
			min = 0;

		// max
		double max = parse(qPrintable(end.simplified()), qPrintable(numberLocale.name()));
		if (parse_errors() > 0)	// if parsing fails, try default locale
			max = parse(qPrintable(end.simplified()), "en_US");
		if (parse_errors() > 0)
			max = 1.;

		//TODO: check for NAN, INF?
		this->setRange(min, max, format, scale);
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
	int direction() const { return m_start <= m_end ? 1 : -1; }
	// relative precision (high when interval is small compared to range). At least 4 digits
	int relativePrecision() const { return qMax(4, -nsl_math_rounded_decimals(std::abs(m_start)/length()) + 1); }
	bool contains(const Range<T>& other) const { return ( qMin(m_start, m_end) <= qMin(other.start(), other.end()) && qMax(m_start, m_end) >= qMax(other.start(), other.end()) ); }
	bool contains(T value) const { return ( qMin(m_start, m_end) <= value && qMax(m_start, m_end) >= value ); }
	void translate(T offset) { m_start += offset; m_end += offset; }
	void extend(T value) { m_start -= value; m_end += value; }
	bool operator==(const Range<T>& other) const { return ( m_start == other.start() && m_end == other.end() ); }
	bool operator!=(const Range<T>& other) const { return ( m_start != other.start() || m_end != other.end() ); }
	Range<T>& operator+=(const T value) { m_start += value; m_end += value; return *this; }
	Range<T>& operator*=(const T value) { m_start *= value; m_end *= value; return *this; }

	//! Return a string in the format 'start .. end' and uses system locale (specialization see below)
	// round == true rounds the double values
	QString toString(bool round = true) const {
		Q_UNUSED(round)
		if (m_format == Format::Numeric)
			return QLocale().toString(m_start) + " .. " + QLocale().toString(m_end);
		else
			return QDateTime::fromMSecsSinceEpoch(m_start).toString(m_dateTimeFormat) + " .. "
				+ QDateTime::fromMSecsSinceEpoch(m_end).toString(m_dateTimeFormat);
	}
	std::string toStdString() const { return STDSTRING(toString()); }
	//! Return a string in the format 'start .. end' and uses number locale
	QString toLocaleString(bool round = true) const {
		SET_NUMBER_LOCALE
		return this->toString(round);
	}
//extend/shrink range to nice numbers (used in auto scaling)
	// get nice size to extend to (see Glassner: Graphic Gems)
	double niceSize(double size, bool round) {
		const double exponent = std::floor(log10(size));
		const double fraction = size / std::pow(10., exponent);

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

		return niceFraction * std::pow(10., exponent);
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
			m_start = std::floor(m_start / spacing) * spacing;
			m_end = std::ceil(m_end / spacing) * spacing;
		} else {
			m_start = std::ceil(m_start / spacing) * spacing;
			m_end = std::floor(m_end / spacing) * spacing;
		}
		DEBUG(Q_FUNC_INFO << ", new range : " << toStdString())
	}
	int autoTickCount() const {
		if (length() == 0)
			return 0;

		DEBUG(Q_FUNC_INFO << ", range = " << toStdString() << ", length() = " << length())
		const double order = pow(10.0, std::floor(log10(length())));;
		DEBUG(Q_FUNC_INFO << ", order of magnitude = " << order)
		const int factor = qRound(100 * length() / order);
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

		return 11+1;
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

// specialization
template<>
inline QString Range<double>::toString(bool round) const {
	if (m_format == Format::Numeric) {
		if (round) {
			const int relPrec = relativePrecision();
			//DEBUG(Q_FUNC_INFO << ", rel prec = " << relPrec)
			return QLocale().toString(nsl_math_round_precision(m_start, relPrec), 'g', relPrec) + " .. " + 
				QLocale().toString(nsl_math_round_precision(m_end, relPrec), 'g', relPrec);
		} else
			return QLocale().toString(m_start, 'g', 12) + " .. " + QLocale().toString(m_end, 'g', 12);
	} else
		return QDateTime::fromMSecsSinceEpoch(m_start).toString(m_dateTimeFormat) + " .. "
			+ QDateTime::fromMSecsSinceEpoch(m_end).toString(m_dateTimeFormat);
}

#endif

