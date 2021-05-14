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

#include "CartesianScale.h"
#include "CartesianPlot.h"
#include "backend/worksheet/plots/AbstractCoordinateSystem.h"

class CartesianCoordinateSystemPrivate;
class CartesianCoordinateSystemSetScalePropertiesCmd;

class CartesianCoordinateSystem: public AbstractCoordinateSystem {
public:
	explicit CartesianCoordinateSystem(CartesianPlot*);
	~CartesianCoordinateSystem() override;

	//TODO: document the 5 versions
	Points mapLogicalToScene(const Points&, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	void mapLogicalToScene(const Points& logicalPoints, Points& scenePoints, std::vector<bool>& visiblePoints, MappingFlags flags = MappingFlag::DefaultMapping) const;
	void mapLogicalToScene(int startIndex, int endIndex, const Points& logicalPoints, Points& scenePoints, QVector<bool>& visiblePoints, MappingFlags flags = MappingFlag::DefaultMapping) const;
	QPointF mapLogicalToScene(QPointF, bool& visible, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	Lines mapLogicalToScene(const Lines&, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	Points mapSceneToLogical(const Points&, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	QPointF mapSceneToLogical(QPointF, MappingFlags flags = MappingFlag::DefaultMapping) const override;

	int xDirection() const;
	int yDirection() const;
	bool setXScales(const QVector<CartesianScale*>&);
	QVector<CartesianScale*> xScales() const;
	bool setYScales(const QVector<CartesianScale*>&);
	QVector<CartesianScale*> yScales() const;
	int xIndex() const;
	void setXIndex(int);
	int yIndex() const;
	void setYIndex(int);

	QString info() const override;

private:
	void init();
	bool rectContainsPoint(const QRectF&, QPointF) const;
	CartesianCoordinateSystemPrivate* d;
};

#endif
