/*
    File                 : CartesianCoordinateSystem.h
    Project              : LabPlot
    Description          : Cartesian coordinate system for plots.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2012-2016 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
