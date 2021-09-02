/*
    File                 : CartesianScale.cpp
    Project              : LabPlot
    Description          : Cartesian coordinate system for plots.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2012-2016 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2020-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianScale.h"

extern "C" {
#include <gsl/gsl_math.h>
}

/**
 * \class CartesianScale
 * \brief Base class for cartesian coordinate system scales.
 */
CartesianScale::CartesianScale(const Range<double> &range, double a, double b, double c)
	: m_range(range), m_a(a), m_b(b), m_c(c) {
}

CartesianScale::~CartesianScale() = default;

void CartesianScale::getProperties(Range<double> *range,
		double *a, double *b, double *c) const {
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
 * \brief implementation of a linear scale for cartesian coordinate systems
 * y =  b * x + a. a - offset, b - gradient
 */
class LinearScale : public CartesianScale {
public:
	LinearScale(const Range<double> &range, double offset, double gradient)
		: CartesianScale(range, offset, gradient, 0) {
			Q_ASSERT(gradient != 0.0);
		}

	~LinearScale() override = default;

	bool map(double *value) const override {
		*value = *value * m_b + m_a;
		return true;
	}

	bool inverseMap(double *value) const override {
		CHECK(m_b != 0.0)
		*value = (*value - m_a) / m_b;
		return true;
	}

	int direction() const override {
		return m_b < 0 ? -1 : 1;
	}
};

/**
 * \class CartesianCoordinateSystem::LogScale
 * \brief implementation of a logarithmic scale for cartesian coordinate systems
 * y = TODO. a - offset, b - scaleFactor, c - base
 */
class LogScale : public CartesianScale {
public:
	LogScale(const Range<double> &range, double offset, double scaleFactor, double base)
		: CartesianScale(range, offset, scaleFactor, base) {
			Q_ASSERT(scaleFactor != 0.0);
			Q_ASSERT(base > 0.0);
	}

	~LogScale() override = default;

	bool map(double *value) const override {
		CHECK(m_c > 0)
		CHECK(*value != 0)

		*value = log(qAbs(*value))/log(m_c) * m_b + m_a;

		return true;
	}

	bool inverseMap(double *value) const override {
		CHECK(m_c > 0)

		*value = pow(m_c, (*value - m_a) / m_b);
		return true;
	}
	int direction() const override {
		return m_b < 0 ? -1 : 1;
	}
};

/**
 * \class CartesianCoordinateSystem::SqrtScale
 * \brief implementation of a square-root scale for cartesian coordinate systems
 * y = TODO. a - offset, b - scaleFactor
 */
class SqrtScale : public CartesianScale {
public:
	SqrtScale(const Range<double> &range, double offset, double scaleFactor)
		: CartesianScale(range, offset, scaleFactor, 0) {
			Q_ASSERT(scaleFactor != 0.0);
	}

	~SqrtScale() override = default;

	bool map(double *value) const override {
		*value = sqrt(qAbs(*value)) * m_b + m_a;
		return true;
	}

	bool inverseMap(double *value) const override {
		CHECK(m_b != 0)

		*value = gsl_pow_2((*value - m_a) / m_b);
		return true;
	}
	int direction() const override {
		return m_b < 0 ? -1 : 1;
	}
};

/**
 * \class CartesianCoordinateSystem::SquareScale
 * \brief implementation of a square scale for cartesian coordinate systems
 * y = TODO. a - offset, b - scaleFactor
 */
class SquareScale : public CartesianScale {
public:
	SquareScale(const Range<double> &range, double offset, double scaleFactor)
		: CartesianScale(range, offset, scaleFactor, 0) {
			Q_ASSERT(scaleFactor != 0.0);
	}

	~SquareScale() override = default;

	bool map(double *value) const override {
		*value = gsl_pow_2(*value) * m_b + m_a;
		return true;
	}

	bool inverseMap(double *value) const override {
		CHECK(m_b != 0)

		*value = sqrt(qAbs((*value - m_a) / m_b));
		return true;
	}
	int direction() const override {
		return m_b < 0 ? -1 : 1;
	}
};

/**
 * \class CartesianCoordinateSystem::InverseScale
 * \brief implementation of an inverse scale for cartesian coordinate systems
 * y = TODO. a - offset, b - scaleFactor
 */
class InverseScale : public CartesianScale {
public:
	InverseScale(const Range<double> &range, double offset, double scaleFactor)
		: CartesianScale(range, offset, scaleFactor, 0) {
			Q_ASSERT(scaleFactor != 0.0);
	}

	~InverseScale() override = default;

	bool map(double *value) const override {
		CHECK(*value != 0)

		*value = m_b/(*value) + m_a;
		return true;
	}

	bool inverseMap(double *value) const override {
		CHECK(*value != m_a)

		*value = m_b/(*value - m_a);
		return true;
	}
	int direction() const override {
		return m_b < 0 ? -1 : 1;
	}
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

CartesianScale* CartesianScale::createLogScale(const Range<double> &range,
		const Range<double> &sceneRange, const Range<double> &logicalRange, RangeT::Scale scale) {

	if (logicalRange.start() <= 0.0 || logicalRange.end() <= 0.0 || logicalRange.isZero()) {
		DEBUG(Q_FUNC_INFO << ", WARNING: invalid range for log scale : " << logicalRange.toStdString())
		return nullptr;
	}

	double base;
	if (scale == RangeT::Scale::Log10)
		base = 10.0;
	else if (scale == RangeT::Scale::Log2)
		base = 2.0;
	else	// RangeT::Scale::Ln
		base = M_E;

	const double lDiff = (log(logicalRange.end()) - log(logicalRange.start())) / log(base);
	double b = sceneRange.size() / lDiff;
	double a = sceneRange.start() - b * log(logicalRange.start()) / log(base);

	return new LogScale(range, a, b, base);
}

CartesianScale* CartesianScale::createSqrtScale(const Range<double> &range,
		const Range<double> &sceneRange, const Range<double> &logicalRange) {

	if (logicalRange.start() < 0.0 || logicalRange.end() < 0.0 || logicalRange.isZero()) {
		DEBUG(Q_FUNC_INFO << ", WARNING: invalid range for sqrt scale : " << logicalRange.toStdString())
		return nullptr;
	}

	const double lDiff = sqrt(logicalRange.end()) - sqrt(logicalRange.start());
	double b = sceneRange.size() / lDiff;
	double a = sceneRange.start() - b * sqrt(logicalRange.start());

	return new SqrtScale(range, a, b);
}

CartesianScale* CartesianScale::createSquareScale(const Range<double> &range,
		const Range<double> &sceneRange, const Range<double> &logicalRange) {

	const double lDiff = logicalRange.end()*logicalRange.end() - logicalRange.start()*logicalRange.start();
	double b = sceneRange.size() / lDiff;
	double a = sceneRange.start() - b * logicalRange.start()*logicalRange.start();

	return new SquareScale(range, a, b);
}

CartesianScale* CartesianScale::createInverseScale(const Range<double> &range,
		const Range<double> &sceneRange, const Range<double> &logicalRange) {

	const double lDiff = 1./logicalRange.end() - 1./logicalRange.start();
	double b = sceneRange.size() / lDiff;
	double a = sceneRange.start() - b / logicalRange.start();

	return new InverseScale(range, a, b);
}
