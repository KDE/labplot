/***************************************************************************
    File                 : AbstractCoordinateSystem.h
    Project              : LabPlot
    Description          : Base class of all worksheet coordinate systems.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012 Alexander Semke (alexander.semke@web.de)

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
