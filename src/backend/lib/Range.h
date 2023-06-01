/*
	File                 : Range.h
	Project              : LabPlot
	Description          : basic data range class
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RANGE_H
#define RANGE_H

#include "backend/gsl/parser.h"
#include "macros.h" //const auto numberLocale = QLocale();

extern "C" {
#include "backend/nsl/nsl_math.h"
}

#include <QStringList>
#include <cmath>
#include <klocalizedstring.h>

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
class RangeT { // access enum without template
	Q_GADGET
public:
	enum class Format { Numeric, DateTime };
	Q_ENUM(Format)
	// see https://www.originlab.com/doc/Origin-Help/AxesRef-Scale#Type
	// TODO: InverseOffset, Prob, Probit, Logit, Weibull
	enum class Scale { Linear, Log10, Log2, Ln, Sqrt, Square, Inverse };
	Q_ENUM(Scale)
	static const QStringList& scaleNames(); // see Range.cpp
	// TODO: when we have C++17: use inline initialization
	//	const static inline QStringList scaleNames{ i18n("Linear"), i18n("Log10"), i18n("Log2"), i18n("Ln"), i18n("Sqrt"), i18n("Square") };
	static bool isLogScale(Scale scale) {
		if (scale == Scale::Log10 || scale == Scale::Log2 || scale == Scale::Ln)
			return true;
		return false;
	}
};

template<class T>
class Range : RangeT {
public:
	Range() {
	}
	Range(T start, T end, Format format = Format::Numeric, Scale scale = Scale::Linear) {
		this->setRange(start, end, format, scale);
	}
	// start and end as localized strings like "pi + 1.5" (will be parsed)
	Range(const QString& start, const QString& end, const Format format = Format::Numeric, const Scale scale = Scale::Linear) {
		const auto numberLocale = QLocale();
		// min
		double min = parse(qPrintable(start.simplified()), qPrintable(numberLocale.name()));
		if (parse_errors() > 0) // if parsing fails, try default locale
			min = parse(qPrintable(start.simplified()), "en_US");
		if (parse_errors() > 0)
			min = 0;

		// max
		double max = parse(qPrintable(end.simplified()), qPrintable(numberLocale.name()));
		if (parse_errors() > 0) // if parsing fails, try default locale
			max = parse(qPrintable(end.simplified()), "en_US");
		if (parse_errors() > 0)
			max = 1.;

		// TODO: check for NAN, INF?
		this->setRange(min, max, format, scale);
	}

	T start() const {
		return m_start;
	}
	T end() const {
		return m_end;
	}
	T& start() {
		return m_start;
	}
	T& end() {
		return m_end;
	}
	Format format() const {
		return m_format;
	}
	Format& format() {
		return m_format;
	}
	Scale scale() const {
		return m_scale;
	}
	Scale& scale() {
		return m_scale;
	}
	QString dateTimeFormat() const {
		return m_dateTimeFormat;
	}
	const QString& dateTimeFormat() {
		return m_dateTimeFormat;
	}
	bool autoScale() const {
		return m_autoScale;
	}

	void setStart(T start) {
		m_start = start;
	}
	void setEnd(T end) {
		m_end = end;
	}
	void setRange(T start, T end, Format format, Scale scale) {
		m_start = start;
		m_end = end;
		m_format = format;
		m_scale = scale;
	}
	void setRange(T start, T end) { // set range keeping format and scale
		m_start = start;
		m_end = end;
	}
	void setFormat(Format format) {
		m_format = format;
	}
	void setScale(Scale scale) {
		m_scale = scale;
	}
	void setDateTimeFormat(const QString& format) {
		m_dateTimeFormat = format;
	}
	void setAutoScale(bool b) {
		m_autoScale = b;
	}

	T size() const {
		return m_end - m_start;
	}
	T length() const {
		return qAbs(m_end - m_start);
	}
	T center() const {
		return (m_start + m_end) / 2;
	}
	// calculate step size from number of steps
	T stepSize(const int steps) const {
		return (steps > 1) ? size() / static_cast<T>(steps - 1) : 0;
	}
	bool isZero() const {
		return (m_end == m_start);
	}
	bool valid() const {
		return (!qIsNaN(m_start) && !qIsNaN(m_end));
	}
	bool finite() const {
		return (qIsFinite(m_start) && qIsFinite(m_end));
	}
	bool inverse() const {
		return (m_start > m_end);
	}
	int direction() const {
		return m_start <= m_end ? 1 : -1;
	}
	// relative precision (high when interval is small compared to range). At least 4 digits
	int relativePrecision() const {
		return qMax(4, -nsl_math_rounded_decimals(std::abs(m_start) / length()) + 1);
	}
	bool contains(const Range<T>& other) const {
		return (qMin(m_start, m_end) <= qMin(other.start(), other.end()) && qMax(m_start, m_end) >= qMax(other.start(), other.end()));
	}
	bool contains(T value) const {
		return (qMin(m_start, m_end) <= value && qMax(m_start, m_end) >= value);
	}
	void translate(T offset) {
		m_start += offset;
		m_end += offset;
	}
	void extend(T value) {
		m_start -= value;
		m_end += value;
	}
	bool operator==(const Range<T>& other) const {
		return (m_start == other.start() && m_end == other.end());
	}
	bool operator!=(const Range<T>& other) const {
		return (m_start != other.start() || m_end != other.end());
	}
	Range<T>& operator+=(const T value) {
		m_start += value;
		m_end += value;
		return *this;
	}
	Range<T>& operator*=(const T value) {
		m_start *= value;
		m_end *= value;
		return *this;
	}

	//! Return a string in the format 'start .. end' and uses system locale (specialization see below)
	// round == true rounds the double values
	QString toString(bool round = true, QLocale locale = QLocale()) const {
		Q_UNUSED(round)
		if (m_format == Format::Numeric)
			return locale.toString(m_start) + QStringLiteral(" .. ") + locale.toString(m_end);
		else
			return QDateTime::fromMSecsSinceEpoch(m_start, Qt::UTC).toString(m_dateTimeFormat) + QStringLiteral(" .. ")
				+ QDateTime::fromMSecsSinceEpoch(m_end, Qt::UTC).toString(m_dateTimeFormat);
	}
	std::string toStdString(bool round = true) const {
		return STDSTRING(toString(round));
	}
	//! Return a string in the format 'start .. end' and uses number locale
	QString toLocaleString(bool round = true) const {
		return this->toString(round, QLocale());
	}
	// extend/shrink range to nice numbers (used in auto scaling)
	//  get nice size to extend to (see Glassner: Graphic Gems)
	double niceSize(double size, bool round) {
		const double exponent = std::floor(log10(size));
		const double fraction = size / std::pow(10., exponent);

		double niceFraction; // nice (rounded) fraction
		if (round) {
			if (fraction < 1.5)
				niceFraction = 1;
			else if (fraction <= 2.5)
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
		DEBUG(Q_FUNC_INFO << ", round = " << round << ", fraction = " << fraction);
		DEBUG(Q_FUNC_INFO << ", nice fraction = " << niceFraction);

		return niceFraction * std::pow(10., exponent);
	}
	void niceShrink() {
		niceExtend(false);
	}
	void niceExtend(bool extend = true) { // extend == false means shrink
		if (length() == 0)
			return;

		if (isLogScale(scale()) && (m_start <= 0 || m_end <= 0))
			return;

		double base = 10.;
		double oldSize = size();
		switch (scale()) {
		case Scale::Linear:
		case Scale::Log10:
			break; // default base
		case Scale::Log2:
			base = 2.;
			break;
		case Scale::Ln:
			base = M_E;
			break;
		case Scale::Sqrt:
			oldSize = sqrt(oldSize);
			break;
		case Scale::Square:
			oldSize *= oldSize;
			break;
		case Scale::Inverse:
			oldSize = 1. / oldSize;
		}

		if (isLogScale(scale())) {
			if ((extend && m_start < m_end) || (!extend && m_start > m_end)) {
				m_start = nsl_math_round_basex(m_start, -1, base);
				m_end = nsl_math_round_basex(m_end, -1, base) * base;
			} else {
				m_start = nsl_math_round_basex(m_start, -1, base) * base;
				m_end = nsl_math_round_basex(m_end, -1, base);
			}
			return; // log scales end here
		}

		DEBUG("scale = " << (int)scale() << ", old size = " << oldSize)

		const double newSize = niceSize(oldSize, false);
		DEBUG(Q_FUNC_INFO << ", new size = " << newSize);
		const double maxTicks = 10; // TODO: parameter?
		const double spacing = niceSize(newSize / (maxTicks - 1), true);
		DEBUG("spacing = " << spacing)

		// extend/shrink range
		double new_start = m_start, new_end = m_end;
		switch (scale()) {
		case Scale::Linear:
		case Scale::Log10: // log-scales already done
		case Scale::Log2:
		case Scale::Ln:
			break;
		case Scale::Sqrt:
			if (m_start < 0 || m_end < 0)
				return;
			new_start = sqrt(m_start);
			new_end = sqrt(m_end);
			break;
		case Scale::Square:
			new_start = m_start * m_start;
			new_end = m_end * m_end;
			break;
		case Scale::Inverse:
			if (m_start == 0 || m_end == 0)
				return;
			new_start = 1. / m_start;
			new_end = 1. / m_end;
		}

		if ((extend && m_start < m_end) || (!extend && m_start > m_end)) {
			new_start = std::floor(new_start / spacing) * spacing;
			new_end = std::ceil(new_end / spacing) * spacing;
		} else {
			new_start = std::ceil(new_start / spacing) * spacing;
			new_end = std::floor(new_end / spacing) * spacing;
		}
		DEBUG(" tmp new range: " << new_start << " .. " << new_end)

		switch (scale()) {
		case Scale::Linear:
		case Scale::Log10: // log-scales already done
		case Scale::Log2:
		case Scale::Ln:
			break;
		case Scale::Sqrt:
			new_start *= new_start;
			new_end *= new_end;
			break;
		case Scale::Square:
			if (new_start < 0 || new_end < 0)
				return;
			new_start = sqrt(new_start);
			new_end = sqrt(new_end);
			break;
		case Scale::Inverse:
			if (new_start == 0 || new_end == 0)
				return;
			new_start = 1. / new_start;
			new_end = 1. / new_end;
		}

		if (std::abs(new_end - new_start) == 0) // avoid empty range
			return;

		m_start = new_start;
		m_end = new_end;
		DEBUG(Q_FUNC_INFO << ", new range: " << toStdString())
	}
	int autoTickCount() const {
		DEBUG(Q_FUNC_INFO << ", scale = " << (int)scale())

		if (length() == 0)
			return 0;

		if (isLogScale(scale()) && (m_start <= 0 || m_end <= 0))
			return 1; // datetime test expects value > 0

		int order = 0;
		switch (scale()) {
		case Scale::Linear:
		case Scale::Sqrt:
		case Scale::Square:
		case Scale::Inverse:
			break;
		case Scale::Log10:
			order = log10(m_end) - log10(m_start);
			break;
		case Scale::Log2:
			order = log2(m_end) - log2(m_start);
			break;
		case Scale::Ln:
			order = log(m_end) - log(m_start);
			break;
		}

		if (isLogScale(scale())) {
			DEBUG(Q_FUNC_INFO << ", order = " << order)
			if (order >= 0)
				return order + 1;
			else // reverse range
				return -order + 1;
		} // log scales end here

		DEBUG(Q_FUNC_INFO << ", range = " << toStdString() << ", length() = " << length())
		const double order_of_magnitude = pow(10.0, std::floor(log10(length())));

		DEBUG(Q_FUNC_INFO << ", order of magnitude = " << order_of_magnitude)
		const int factor = qRound(100 * length() / order_of_magnitude);
		DEBUG(Q_FUNC_INFO << ", factor = " << factor)

		// set number of ticks for certain multiple of small numbers
		if (factor % 30 == 0)
			return 3 + 1;
		if (factor % 40 == 0)
			return 4 + 1;
		if (factor % 70 == 0)
			return 7 + 1;
		if (factor % 50 == 0)
			return 5 + 1;
		if (factor % 90 == 0)
			return 9 + 1;
		if (factor % 175 == 0)
			return 7 + 1;
		if (factor % 25 == 0)
			return 5 + 1;
		if (factor % 105 == 0)
			return 7 + 1;
		if (factor % 115 == 0)
			return 5 + 1;

		return 11 + 1;
	}
	// TODO: touches(), merge(), subtract(), split(), etc. (see Interval)

private:
	T m_start{0}; // start value
	T m_end{1}; // end value
	Format m_format{Format::Numeric}; // format (Numeric or DateTime)
	QString m_dateTimeFormat{QLatin1String("yyyy-MM-dd hh:mm:ss")}; // only used for DateTime
	Scale m_scale{Scale::Linear}; // scale (Linear, Log , ...)
	bool m_autoScale{true}; // auto adapt start and end to all curves using this range (in plot)
};

// specialization
template<>
inline QString Range<double>::toString(bool round, QLocale locale) const {
	if (m_format == Format::Numeric) {
		if (round) {
			const int relPrec = relativePrecision();
			// DEBUG(Q_FUNC_INFO << ", rel prec = " << relPrec)
			return locale.toString(nsl_math_round_precision(m_start, relPrec), 'g', relPrec) + QLatin1String(" .. ")
				+ locale.toString(nsl_math_round_precision(m_end, relPrec), 'g', relPrec);
		} else
			return locale.toString(m_start, 'g', 12) + QLatin1String(" .. ") + locale.toString(m_end, 'g', 12);
	} else
		return QDateTime::fromMSecsSinceEpoch(m_start, Qt::UTC).toString(m_dateTimeFormat) + QLatin1String(" .. ")
			+ QDateTime::fromMSecsSinceEpoch(m_end, Qt::UTC).toString(m_dateTimeFormat);
}

#endif
