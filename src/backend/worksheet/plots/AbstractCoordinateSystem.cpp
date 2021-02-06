/***************************************************************************
    File                 : AbstractCoordinateSystem.cpp
    Project              : LabPlot
    Description          : Base class of all worksheet coordinate systems.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2014 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2020 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "backend/worksheet/plots/AbstractCoordinateSystem.h"
#include "backend/worksheet/plots/AbstractPlot.h"

extern "C" {
#include "backend/nsl/nsl_math.h"
}
#include <cmath>

/**
 * \class AbstractCoordinateSystem
 * \brief Base class of all worksheet coordinate systems.
 *
 *  \ingroup backend\worksheet
 */

AbstractCoordinateSystem::AbstractCoordinateSystem(AbstractPlot* plot) {
	Q_UNUSED(plot)
}

AbstractCoordinateSystem::~AbstractCoordinateSystem() = default;

/**
 * \fn QList<QPointF> AbstractCoordinateSystem::mapLogicalToScene(const QList<QPointF> &points, const MappingFlags &flags = DefaultMapping) const;
 * \brief Map a list of points in logical coordinates into graphics scene (page) coordinates.
 *
 * The list of returned points may have less points than the input list if some points have
 * unsupported coordinates or lie in excluded areas such as coordinate gaps or outside the page rectangle.
 *
 * \param points The points to map.
 * \param flags Flags to influence the mapping behavior.
 */

/**
 * \fn QList<QPointF> AbstractCoordinateSystem::mapSceneToLogical(const QList<QPointF> &points, const MappingFlags &flags = DefaultMapping) const;
 * \brief Map a list of points in scene coordinates into logical coordinates.
 *
 * The list of returned points may have less points than the input list if some point lie in
 * areas not possible to map by the coordinate system.
 *
 * \param points The points to map.
 * \param flags Flags to influence the mapping behavior.
 */

/**
 * \fn QList<QLineF> AbstractCoordinateSystem::mapLogicalToScene(const QList<QLineF> &lines, const MappingFlags &flags = DefaultMapping) const;
 * \brief Map a list of lines in logical coordinates into graphics scene (page) coordinates.
 *
 * Lines may be clipped at coordinate gaps or completely be removed if outside the supported areas.
 *
 * \param lines The lines to map.
 * \param flags Flags to influence the mapping behavior.
 */

/**
 * \brief Line clipping using the Cohen-Sutherland algorithm.
 *
 * This is a modified version of clipLine() from Qt 4.5's qpaintengine_x11.cpp.
 *
 * \param line The line to clip.
 * \param rect The rect to clip to.
 * \param clipResult Pointer to an object describing which parts where clipped (may be NULL).
 *
 * \return false if line is completely outside, otherwise true
 */

bool AbstractCoordinateSystem::clipLineToRect(QLineF *line, const QRectF &rect, LineClipResult *clipResult) {
//	QDEBUG(Q_FUNC_INFO << ", line = " << *line << ", rect = " << rect)
	//we usually clip on large rectangles, so we don't need high precision here -> round to one float digit
	//this prevents some subtle float rounding artifacts that lead to disappearance
	//of lines along the boundaries of the rect. (e.g. axis lines).
	qreal x1 = nsl_math_trunc_places(line->x1(), 1);
	qreal x2 = nsl_math_trunc_places(line->x2(), 1);
	qreal y1 = nsl_math_trunc_places(line->y1(), 1);
	qreal y2 = nsl_math_trunc_places(line->y2(), 1);
//	DEBUG(Q_FUNC_INFO << "x1/x2 y1/y2 = " << x1 << "/" << x2 << " " << y1 << "/" << y2)

	qreal left;
	qreal right;
	qreal top;
	qreal bottom;
	rect.getCoords(&left, &top, &right, &bottom);

	if (clipResult)
		clipResult->reset();

	enum { Left, Right, Top, Bottom };
	// clip the lines, after cohen-sutherland, see e.g. http://www.nondot.org/~sabre/graphpro/line6.html
	int p1 = ((x1 < left) << Left)
	         | ((x1 > right) << Right)
	         | ((y1 < top) << Top)
	         | ((y1 > bottom) << Bottom);
	int p2 = ((x2 < left) << Left)
	         | ((x2 > right) << Right)
	         | ((y2 < top) << Top)
	         | ((y2 > bottom) << Bottom);

	if (p1 & p2)
		// completely outside
		return false;

	if (p1 | p2) {
		qreal dx = x2 - x1;
		qreal dy = y2 - y1;

		// clip x coordinates
		if (x1 < left) {
			y1 += dy/dx * (left - x1);
			x1 = left;
			if (clipResult)
				clipResult->xClippedLeft[0] = true;
		} else if (x1 > right) {
			y1 -= dy/dx * (x1 - right);
			x1 = right;
			if (clipResult)
				clipResult->xClippedRight[0] = true;
		}
		if (x2 < left) {
			y2 += dy/dx * (left - x2);
			x2 = left;
			if (clipResult)
				clipResult->xClippedLeft[1] = true;
		} else if (x2 > right) {
			y2 -= dy/dx * (x2 - right);
			x2 = right;
			if (clipResult)
				clipResult->xClippedRight[1] = true;
		}
		p1 = ((y1 < top) << Top)
		     | ((y1 > bottom) << Bottom);
		p2 = ((y2 < top) << Top)
		     | ((y2 > bottom) << Bottom);
		if (p1 & p2)
			return false;
		// clip y coordinates
		if (y1 < top) {
			x1 += dx/dy * (top - y1);
			y1 = top;
			if (clipResult) {
				clipResult->xClippedRight[0] = false;
				clipResult->xClippedLeft[0] = false;
				clipResult->yClippedTop[0] = true;
			}
		} else if (y1 > bottom) {
			x1 -= dx/dy * (y1 - bottom);
			y1 = bottom;
			if (clipResult) {
				clipResult->xClippedRight[0] = false;
				clipResult->xClippedLeft[0] = false;
				clipResult->yClippedBottom[0] = true;
			}
		}
		if (y2 < top) {
			x2 += dx/dy * (top - y2);
			y2 = top;
			if (clipResult) {
				clipResult->xClippedRight[1] = false;
				clipResult->xClippedLeft[1] = false;
				clipResult->yClippedTop[1] = true;
			}
		} else if (y2 > bottom) {
			x2 -= dx/dy * (y2 - bottom);
			y2 = bottom;
			if (clipResult) {
				clipResult->xClippedRight[1] = false;
				clipResult->xClippedLeft[1] = false;
				clipResult->yClippedBottom[1] = true;
			}
		}
		*line = QLineF(QPointF(x1, y1), QPointF(x2, y2));
	}
	return true;
}
