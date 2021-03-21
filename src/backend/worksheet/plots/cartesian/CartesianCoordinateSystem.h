/***************************************************************************
    File                 : CartesianCoordinateSystem.h
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

#ifndef CARTESIANCOORDINATESYSTEM_H
#define CARTESIANCOORDINATESYSTEM_H

#include "backend/worksheet/plots/AbstractCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/lib/Interval.h"

#include <vector>

class CartesianCoordinateSystemPrivate;
class CartesianCoordinateSystemSetScalePropertiesCmd;

class CartesianScale {
public:
	virtual ~CartesianScale();

	//TODO: remove "Scale"
	enum class ScaleType {ScaleLinear, ScaleLog, ScaleSqrt, ScaleSquare};

	static CartesianScale* createLinearScale(const Interval<double> &interval, double sceneStart, double sceneEnd,
		double logicalStart, double logicalEnd);
	static CartesianScale* createLogScale(const Interval<double> &interval, double sceneStart, double sceneEnd,
		double logicalStart, double logicalEnd, CartesianPlot::Scale);
	static CartesianScale* createSqrtScale(const Interval<double> &interval, double sceneStart, double sceneEnd,
		double logicalStart, double logicalEnd);
	static CartesianScale* createSquareScale(const Interval<double> &interval, double sceneStart, double sceneEnd,
		double logicalStart, double logicalEnd);

	virtual void getProperties(ScaleType *type = nullptr, Interval<double> *interval = nullptr,
			double *a = nullptr, double *b = nullptr, double *c = nullptr) const;

	inline double start() const;
	inline double end() const;
	inline bool contains(double) const;
	virtual bool map(double*) const = 0;
	virtual bool inverseMap(double*) const = 0;
	virtual int direction() const = 0;

protected:
	CartesianScale(ScaleType type, const Interval<double> &interval, double a, double b, double c);
	ScaleType m_type;
	Interval<double> m_interval;
	//TODO: what are these?
	double m_a;
	double m_b;
	double m_c;
};

class CartesianCoordinateSystem: public AbstractCoordinateSystem {
public:
	explicit CartesianCoordinateSystem(CartesianPlot*);
	~CartesianCoordinateSystem() override;

	//TODO: document the 5 versions
	QVector<QPointF> mapLogicalToScene(const QVector<QPointF>&, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	void mapLogicalToScene(const QVector<QPointF>& logicalPoints, QVector<QPointF>& scenePoints, std::vector<bool>& visiblePoints, MappingFlags flags = MappingFlag::DefaultMapping) const;
	void mapLogicalToScene(int startIndex, int endIndex, const QVector<QPointF>& logicalPoints, QVector<QPointF>& scenePoints, QVector<bool>& visiblePoints, QVector<QVector<bool>>& scenePointsUsed, double minLogicalDiffX, double minLogicalDiffY, MappingFlags flags = MappingFlag::DefaultMapping) const;
	QPointF mapLogicalToScene(QPointF, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	QVector<QLineF> mapLogicalToScene(const QVector<QLineF>&, MappingFlags flags = MappingFlag::DefaultMapping) const override;

	QVector<QPointF> mapSceneToLogical(const QVector<QPointF>&, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	QPointF mapSceneToLogical(QPointF, MappingFlags flags = MappingFlag::DefaultMapping) const override;

	int xDirection() const;
	int yDirection() const;
	bool setXScales(const QVector<CartesianScale*>&);
	QVector<CartesianScale*> xScales() const;
	bool setYScales(const QVector<CartesianScale*>&);
	QVector<CartesianScale*> yScales() const;

private:
	void init();
	bool rectContainsPoint(const QRectF&, QPointF) const;
	CartesianCoordinateSystemPrivate* d;
};

#endif
