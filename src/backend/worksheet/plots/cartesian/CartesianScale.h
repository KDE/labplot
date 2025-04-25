/*
	File                 : CartesianScale.h
	Project              : LabPlot
	Description          : Cartesian coordinate system for plots.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2016 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANSCALE_H
#define CARTESIANSCALE_H

#include "backend/lib/Range.h"

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT CartesianScale {
#else
class CartesianScale {
#endif
public:
	virtual ~CartesianScale();

	static CartesianScale* createLinearScale(const Range<double>& range, const Range<double>& sceneRange, const Range<double>& logicalRange);
	static CartesianScale* createLogScale(const Range<double>& range, const Range<double>& sceneRange, const Range<double>& logicalRange, RangeT::Scale);
	static CartesianScale* createSqrtScale(const Range<double>& range, const Range<double>& sceneRange, const Range<double>& logicalRange);
	static CartesianScale* createSquareScale(const Range<double>& range, const Range<double>& sceneRange, const Range<double>& logicalRange);
	static CartesianScale* createInverseScale(const Range<double>& range, const Range<double>& sceneRange, const Range<double>& logicalRange);

	virtual void getProperties(Range<double>* range = nullptr, double* a = nullptr, double* b = nullptr, double* c = nullptr) const;

	inline double start() const {
		return m_range.start();
	}
	inline double end() const {
		return m_range.end();
	}
	inline Range<double> range() const {
		return m_range;
	}
	inline bool contains(double value) const {
		return m_range.contains(value);
	}

	virtual bool map(double*) const = 0;
	virtual bool inverseMap(double*) const = 0;
	virtual int direction() const = 0;

protected:
	CartesianScale(const Range<double>& range, double a, double b, double c);
	Range<double> m_range;
	/// Those parameters define the scaling
	/// They will be set by the linear, log, sqrt, ... scales
	/// See the definition of the different scales for their meaning
	double m_a;
	double m_b;
	double m_c;
};

#endif
