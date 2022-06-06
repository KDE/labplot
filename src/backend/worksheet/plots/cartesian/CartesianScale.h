/*
	File                 : CartesianScale.h
	Project              : LabPlot
	Description          : Cartesian coordinate system for plots.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2016 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020-2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANSCALE_H
#define CARTESIANSCALE_H

#include "CartesianPlot.h"
#include "backend/lib/Range.h"

class CartesianScale {
public:
	virtual ~CartesianScale();

	static CartesianScale* createLinearScale(const Range<double>& sceneRange, const Range<double>& logicalRange);
	static CartesianScale* createLogScale(const Range<double>& range, const Range<double>& sceneRange, const Range<double>& logicalRange, RangeT::Scale);
	static CartesianScale* createSqrtScale(const Range<double>& range, const Range<double>& sceneRange, const Range<double>& logicalRange);
	static CartesianScale* createSquareScale(const Range<double>& range, const Range<double>& sceneRange, const Range<double>& logicalRange);
	static CartesianScale* createInverseScale(const Range<double>& range, const Range<double>& sceneRange, const Range<double>& logicalRange);

	virtual void getParameter(Range<double>* = nullptr, double* a = nullptr, double* b = nullptr, double* c = nullptr) const;

	inline double start() const {
		return m_clipRange.start();
	}
	inline double end() const {
		return m_clipRange.end();
	}
	inline Range<double> range() const {
		return m_range;
	}
	inline Range<double> clipRange() const {
		return m_clipRange;
	}
	// can be mapped
	inline bool valid(double value) const {
		return m_range.contains(value);
	}
	// inside logical range
	inline bool contains(double value) const {
		return m_clipRange.contains(value);
	}

	virtual bool map(double*) const = 0;
	virtual bool inverseMap(double*) const = 0;
	virtual int direction() const = 0;

protected:
	CartesianScale(const Range<double>& clipRange, double a, double b, double c, const Range<double>& range = Range<double>(-qInf(), qInf()));
	Range<double> m_range;	// valid range (where map/unmap works), like -Inf .. Inf for linear scale
	Range<double> m_clipRange;	// actual (logical) range, like 0 .. 1
	// scale parameter
	double m_a;
	double m_b;
	double m_c;
};

#endif
