
/***************************************************************************
    File                 : CartesianScale.h
    Project              : LabPlot
    Description          : Cartesian coordinate system for plots.
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2016 by Alexander Semke (alexander.semke@web.de)

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
#include "backend/lib/Interval.h"

class CartesianScale {
public:
	virtual ~CartesianScale();

	enum class Type {Linear, Log};

	static CartesianScale* createLinearScale(const Interval<double> &interval, double sceneStart, double sceneEnd,
		double logicalStart, double logicalEnd);
	static CartesianScale* createLogScale(const Interval<double> &interval, double sceneStart, double sceneEnd,
		double logicalStart, double logicalEnd, CartesianPlot::Scale);

	virtual void getProperties(Type *type = nullptr, Interval<double> *interval = nullptr,
			double *a = nullptr, double *b = nullptr, double *c = nullptr) const;

	inline double start() const {
		return m_interval.start();
	}
	inline double end() const {
		return m_interval.end();
	}
	inline bool contains(double value) const {
		return m_interval.contains(value);
	}

	virtual bool map(double*) const = 0;
	virtual bool inverseMap(double*) const = 0;
	virtual int direction() const = 0;

protected:
	CartesianScale(Type type, const Interval<double> &interval, double a, double b, double c);
	Type m_type;
	Interval<double> m_interval;
	//TODO: what are these?
	double m_a;
	double m_b;
	double m_c;
};

#endif
