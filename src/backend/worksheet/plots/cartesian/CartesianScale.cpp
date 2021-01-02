
/***************************************************************************
    File                 : CartesianScale.cpp
    Project              : LabPlot
    Description          : Cartesian coordinate system for plots.
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2016 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2020 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "CartesianScale.h"

/**
 * \class CartesianScale
 * \brief Base class for cartesian coordinate system scales.
 */
CartesianScale::CartesianScale(Type type, const Range<double> &range, double a, double b, double c)
	: m_type(type), m_range(range), m_a(a), m_b(b), m_c(c) {
}

CartesianScale::~CartesianScale() = default;

void CartesianScale::getProperties(Type *type, Range<double> *range,
		double *a, double *b, double *c) const {
	if (type)
		*type = m_type;
	if (range)
		*range = m_range;
	if (a)
		*a = m_a;
	if (b)
		*b = m_b;
	if (c)
		*c = m_c;
}

/**
 * \class CartesianCoordinateSystem::LinearScale
 * \brief implementation of the linear scale for cartesian coordinate system.
 */
class LinearScale : public CartesianScale {
public:
	LinearScale(const Range<double> &range, double offset, double gradient)
		: CartesianScale(Type::Linear, range, offset, gradient, 0) {
			Q_ASSERT(gradient != 0.0);

		}

	~LinearScale() override = default;

	bool map(double *value) const override {
		*value = *value * m_b + m_a;
		return true;
	}

	bool inverseMap(double *value) const override {
		if (m_b == 0.0)
			return false;
		*value = (*value - m_a) / m_b;
		return true;
	}

	int direction() const override {
		return m_b < 0 ? -1 : 1;
	}
};

/**
 * \class CartesianCoordinateSystem::LinearScale
 * \brief implementation of the linear scale for cartesian coordinate system.
 */
class LogScale : public CartesianScale {
public:
	LogScale(const Range<double> &range, double offset, double scaleFactor, double base, bool abs)
		: CartesianScale(Type::Log, range, offset, scaleFactor, base), m_abs(abs) {
			Q_ASSERT(scaleFactor != 0.0);
			Q_ASSERT(base > 0.0);
	}

	~LogScale() override = default;

	bool map(double *value) const override {
		if (*value > 0.0)
			*value = log(*value)/log(m_c) * m_b + m_a;
		else if (m_abs)
			*value = log(fabs(*value))/log(m_c) * m_b + m_a;
		else
			return false;

		return true;
	}

	bool inverseMap(double *value) const override {
		if (m_a == 0.0)
			return false;
		if (m_c <= 0.0)
			return false;

		*value = pow(m_c, (*value - m_a) / m_b);
		return true;
	}
	int direction() const override {
		return m_b < 0 ? -1 : 1;
	}

private:
	bool m_abs;
};


/***************************************************************/

CartesianScale* CartesianScale::createLinearScale(const Range<double> &range,
		const Range<double> &sceneRange, const Range<double> &logicalRange) {

	if (logicalRange.size() == 0.0)
		return nullptr;

	double b = sceneRange.size() / logicalRange.size();
	double a = sceneRange.start() - b * logicalRange.start();

	return new LinearScale(range, a, b);
}

// obsolete version
CartesianScale* CartesianScale::createLogScale(const Range<double> &range,
		const Range<double> &sceneRange, const Range<double> &logicalRange, CartesianPlot::Scale type) {

	double base;
	if (type == CartesianPlot::Scale::Log10 || type == CartesianPlot::Scale::Log10Abs)
		base = 10.0;
	else if (type == CartesianPlot::Scale::Log2 || type == CartesianPlot::Scale::Log2Abs)
		base = 2.0;
	else
		base = M_E;


	if (logicalRange.start() <= 0.0 || logicalRange.end() <= 0.0)
		return nullptr;

	const double lDiff = (log(logicalRange.end()) - log(logicalRange.start())) / log(base);
	if (lDiff == 0.0)
		return nullptr;

	double b = sceneRange.size() / lDiff;
	double a = sceneRange.start() - b * log(logicalRange.start())/log(base);

	bool abs = (type == CartesianPlot::Scale::Log10Abs || type == CartesianPlot::Scale::Log2Abs || type == CartesianPlot::Scale::LnAbs);
	return new LogScale(range, a, b, base, abs);
}
CartesianScale* CartesianScale::createLogScale(const Range<double> &range,
		const Range<double> &sceneRange, const Range<double> &logicalRange, RangeT::Scale scale) {

	double base;
	if (scale == RangeT::Scale::Log10 || scale == RangeT::Scale::Log10Abs)
		base = 10.0;
	else if (scale == RangeT::Scale::Log2 || scale == RangeT::Scale::Log2Abs)
		base = 2.0;
	else
		base = M_E;


	if (logicalRange.start() <= 0.0 || logicalRange.end() <= 0.0)
		return nullptr;

	const double lDiff = (log(logicalRange.end()) - log(logicalRange.start())) / log(base);
	if (lDiff == 0.0)
		return nullptr;

	double b = sceneRange.size() / lDiff;
	double a = sceneRange.start() - b * log(logicalRange.start())/log(base);

	bool abs = (scale == RangeT::Scale::Log10Abs || scale == RangeT::Scale::Log2Abs || scale == RangeT::Scale::LnAbs);
	return new LogScale(range, a, b, base, abs);
}

