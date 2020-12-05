
/***************************************************************************
    File                 : CartesianScale.h
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

#ifndef CARTESIANSCALE_H
#define CARTESIANSCALE_H

#include "CartesianPlot.h"
#include "backend/lib/Range.h"

class CartesianScale {
public:
	virtual ~CartesianScale();

	enum class Type {Linear, Log};

	static CartesianScale* createLinearScale(const Range<double> &range, const Range<double> &sceneRange, const Range<double> &logicalRange);
	static CartesianScale* createLogScale(const Range<double> &range, const Range<double> &sceneRange, const Range<double> &logicalRange, CartesianPlot::Scale);

	virtual void getProperties(Type *type = nullptr, Range<double> *range = nullptr, double *a = nullptr, double *b = nullptr, double *c = nullptr) const;

	inline double start() const { return m_range.start(); }
	inline double end() const { return m_range.end(); }
	inline Range<double> range() const { return m_range; }
	inline bool contains(double value) const { return m_range.contains(value); }

	virtual bool map(double*) const = 0;
	virtual bool inverseMap(double*) const = 0;
	virtual int direction() const = 0;

protected:
	CartesianScale(Type type, const Range<double> &range, double a, double b, double c);
	Type m_type;
	Range<double> m_range;
	//TODO: what are these?
	double m_a;
	double m_b;
	double m_c;
};

#endif
