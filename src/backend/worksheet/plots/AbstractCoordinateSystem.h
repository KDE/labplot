/*
    File                 : AbstractCoordinateSystem.h
    Project              : LabPlot
    Description          : Base class of all worksheet coordinate systems.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert (thzs@gmx.net)
    SPDX-FileCopyrightText: 2012 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef ABSTRACTCOORDINATESYSTEM_H
#define ABSTRACTCOORDINATESYSTEM_H

#include "backend/worksheet/plots/AbstractPlot.h"
#include <QVector>

typedef QVector<QPointF> Points;
typedef QVector<QLineF> Lines;

class AbstractCoordinateSystem {
public:
	enum class MappingFlag {
		DefaultMapping = 0x00,
		SuppressPageClipping = 0x01,
		MarkGaps = 0x02,
		Limit = 0x04, // set limits, when point crosses the limits
		SuppressPageClippingY = 0x08,
	};
	Q_DECLARE_FLAGS(MappingFlags, MappingFlag)

	explicit AbstractCoordinateSystem(AbstractPlot*);
	virtual ~AbstractCoordinateSystem();

	virtual Points mapLogicalToScene(const Points&, MappingFlags flags = MappingFlag::DefaultMapping) const = 0;
	virtual QPointF mapLogicalToScene(QPointF, bool& visible, MappingFlags flags = MappingFlag::DefaultMapping) const = 0;
	virtual Lines mapLogicalToScene(const Lines&, MappingFlags flags = MappingFlag::DefaultMapping) const = 0;
	virtual Points mapSceneToLogical(const Points&, MappingFlags flags = MappingFlag::DefaultMapping) const = 0;
	virtual QPointF mapSceneToLogical(QPointF, MappingFlags flags = MappingFlag::DefaultMapping) const = 0;

	virtual QString info() const { return QString(); };

	class LineClipResult {
	public:
		LineClipResult() {
			reset();
		}
		inline void reset() {
			for (int i = 0; i < 2; i++) {
				xClippedRight[i] = false;
				xClippedLeft[i] = false;
				yClippedTop[i] = false;
				yClippedBottom[i] = false;
			}
		}
		bool xClippedRight[2];
		bool xClippedLeft[2];
		bool yClippedTop[2];
		bool yClippedBottom[2];
	};

	//static members
	static bool clipLineToRect(QLineF *line, const QRectF &rect, LineClipResult *clipResult = nullptr);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractCoordinateSystem::MappingFlags)

#endif
