/***************************************************************************
    File                 : CartesianPlotPrivate.h
    Project              : LabPlot
    Description          : Private members of CartesianPlot.
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2017 Alexander Semke (alexander.semke@web.de)

 *******************************************************7*******************/

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

class CartesianPlotPrivate : public AbstractPlotPrivate {

public:
	explicit CartesianPlotPrivate(CartesianPlot*);

	void retransform() override;
	void retransformScales();
	void rangeChanged();
	void xRangeFormatChanged();
	void yRangeFormatChanged();

	QRectF dataRect;
	CartesianPlot::RangeType rangeType{CartesianPlot::RangeFree};
	CartesianPlot::RangeFormat xRangeFormat{CartesianPlot::Numeric};
	CartesianPlot::RangeFormat yRangeFormat{CartesianPlot::Numeric};
	QString xRangeDateTimeFormat;
	QString yRangeDateTimeFormat;
	int rangeFirstValues{1000};
	int rangeLastValues{1000};
	double xMin{0.0}, xMax{1.0}, yMin{0.0}, yMax{1.0};
	float xMinPrev{0.0}, xMaxPrev{1.0}, yMinPrev{0.0}, yMaxPrev{1.0};
	bool autoScaleX{true}, autoScaleY{true};
	float autoScaleOffsetFactor{0.0f};
	CartesianPlot::Scale xScale{CartesianPlot::ScaleLinear}, yScale{CartesianPlot::ScaleLinear};
	bool xRangeBreakingEnabled{false};
	bool yRangeBreakingEnabled{false};
	CartesianPlot::RangeBreaks xRangeBreaks;
	CartesianPlot::RangeBreaks yRangeBreaks;
	QString theme;

	//cached values of minimum and maximum for all visible curves
	bool curvesXMinMaxIsDirty{false}, curvesYMinMaxIsDirty{false};
	double curvesXMin{INFINITY}, curvesXMax{-INFINITY}, curvesYMin{INFINITY}, curvesYMax{-INFINITY};

	CartesianPlot* const q;
	CartesianPlot::MouseMode mouseMode{CartesianPlot::SelectionMode};
	CartesianCoordinateSystem* cSystem{nullptr};
	bool suppressRetransform{false};
	bool panningStarted{false};
	bool locked{false};

private:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
	void wheelEvent(QGraphicsSceneWheelEvent*) override;
	void hoverMoveEvent(QGraphicsSceneHoverEvent*) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void updateDataRect();
	void checkXRange();
	void checkYRange();
	CartesianScale* createScale(CartesianPlot::Scale type,
		double sceneStart, double sceneEnd,
		double logicalStart, double logicalEnd);

	bool m_selectionBandIsShown{false};
	QPointF m_selectionStart;
	QPointF m_selectionEnd;
	QLineF m_selectionStartLine;
	QPointF m_panningStart;
};

#endif
