/*
	File                 : CartesianCoordinateSystem.h
	Project              : LabPlot
	Description          : Cartesian coordinate system for plots.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2016 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANCOORDINATESYSTEM_H
#define CARTESIANCOORDINATESYSTEM_H

#include "CartesianScale.h"
#include "backend/worksheet/plots/AbstractCoordinateSystem.h"

class CartesianCoordinateSystemPrivate;
class CartesianCoordinateSystemSetScalePropertiesCmd;
class CartesianPlot;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT CartesianCoordinateSystem : public AbstractCoordinateSystem {
#else
class CartesianCoordinateSystem : public AbstractCoordinateSystem {
#endif
	Q_GADGET
public:
	enum class Dimension { X, Y };
	Q_ENUM(Dimension)

	explicit CartesianCoordinateSystem(CartesianPlot*);
	~CartesianCoordinateSystem() override;

	static QString dimensionToString(Dimension);

	// TODO: document the 5 versions
	Points mapLogicalToScene(const Points&, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	void mapLogicalToScene(const Points& logicalPoints,
						   Points& scenePoints,
						   std::vector<bool>& visiblePoints,
						   MappingFlags flags = MappingFlag::DefaultMapping) const;
	void mapLogicalToScene(int startIndex,
						   int endIndex,
						   const Points& logicalPoints,
						   Points& scenePoints,
						   std::vector<bool>& visiblePoints,
						   MappingFlags flags = MappingFlag::DefaultMapping) const;
	QPointF mapLogicalToScene(QPointF, bool& visible, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	Lines mapLogicalToScene(const Lines&, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	Points mapSceneToLogical(const Points&, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	QPointF mapSceneToLogical(QPointF, MappingFlags flags = MappingFlag::DefaultMapping) const override;
	virtual bool isValid() const override;

	int direction(const Dimension) const;
	bool setScales(const Dimension, const QVector<CartesianScale*>&);
	QVector<CartesianScale*> scales(const Dimension) const;
	int index(const Dimension) const;
	void setIndex(const Dimension, const int);

	QString info() const override;

private:
	void init();
	bool rectContainsPoint(const QRectF&, QPointF) const;
	CartesianCoordinateSystemPrivate* d;
};

using Dimension = CartesianCoordinateSystem::Dimension;
#endif
