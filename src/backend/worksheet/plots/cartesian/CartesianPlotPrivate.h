/***************************************************************************
    File                 : CartesianPlotPrivate.h
    Project              : LabPlot
    Description          : Private members of CartesianPlot.
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2016 Alexander Semke (alexander.semke@web.de)

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

#include <QGraphicsSceneMouseEvent>

class CartesianPlotPrivate:public AbstractPlotPrivate{
    public:
		explicit CartesianPlotPrivate(CartesianPlot* owner);
		CartesianPlot* const q;

		virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
		virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*);
		virtual void wheelEvent(QGraphicsSceneWheelEvent*);
		virtual void hoverMoveEvent(QGraphicsSceneHoverEvent*);
		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0);

		virtual void retransform();
		void retransformScales();
		void checkXRange();
		void checkYRange();
		CartesianScale* createScale(CartesianPlot::Scale type,
									double sceneStart, double sceneEnd,
									double logicalStart, double logicalEnd);

		float xMin, xMax, yMin, yMax;
		float xMinPrev, xMaxPrev, yMinPrev, yMaxPrev;
		bool autoScaleX, autoScaleY;
		float autoScaleOffsetFactor;
		CartesianPlot::Scale xScale, yScale;
		bool xRangeBreakingEnabled;
		bool yRangeBreakingEnabled;
		CartesianPlot::RangeBreaks xRangeBreaks;
		CartesianPlot::RangeBreaks yRangeBreaks;

		//cached values of minimum and maximum for all visible curves
		bool curvesXMinMaxIsDirty, curvesYMinMaxIsDirty;
		double curvesXMin, curvesXMax, curvesYMin, curvesYMax;

		bool suppressRetransform;
		bool m_printing;
		bool m_selectionBandIsShown;
		QPointF m_selectionStart;
		QPointF m_selectionEnd;
		CartesianCoordinateSystem* cSystem;
		CartesianPlot::MouseMode mouseMode;
		QLineF m_selectionStartLine;
};

#endif
