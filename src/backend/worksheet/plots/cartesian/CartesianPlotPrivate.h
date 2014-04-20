/***************************************************************************
    File                 : CartesianPlotPrivate.h
    Project              : LabPlot
    Description          : Private members of CartesianPlot.
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)
                           
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

#ifndef CARTESIANPLOTPRIVATE_H
#define CARTESIANPLOTPRIVATE_H

#include "CartesianPlot.h"
#include "CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/AbstractPlotPrivate.h"

#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>

class CartesianPlotPrivate:public AbstractPlotPrivate{
    public:
		CartesianPlotPrivate(CartesianPlot *owner);
		CartesianPlot* const q;

		virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*);
		virtual void wheelEvent(QGraphicsSceneWheelEvent*);
		virtual void hoverMoveEvent(QGraphicsSceneHoverEvent*);

		virtual void retransform();
		void retransformScales();
		float round(float value, int precision);
		void checkXRange();
		void checkYRange();
		CartesianCoordinateSystem::Scale* createScale(CartesianPlot::Scale type, Interval<double>& interval,
													  double sceneStart, double sceneEnd,
													  double logicalStart, double logicalEnd);

		float xMin, xMax, yMin, yMax;
		float xMinPrev, xMaxPrev, yMinPrev, yMaxPrev;
		bool autoScaleX, autoScaleY;
		float autoScaleOffsetFactor;
		CartesianPlot::Scale xScale, yScale;
		CartesianPlot::ScaleBreakings xScaleBreakings;
		CartesianPlot::ScaleBreakings yScaleBreakings;

		bool suppressRetransform;
		bool m_printing;
};

#endif