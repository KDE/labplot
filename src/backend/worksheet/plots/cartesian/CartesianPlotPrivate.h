/*
    File                 : CartesianPlotPrivate.h
    Project              : LabPlot
    Description          : Private members of CartesianPlot.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef CARTESIANPLOTPRIVATE_H
#define CARTESIANPLOTPRIVATE_H

#include "CartesianPlot.h"
#include "CartesianCoordinateSystem.h"
#include "backend/worksheet/Worksheet.h"
#include "../AbstractPlotPrivate.h"

#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QStaticText>

class CartesianPlotPrivate : public AbstractPlotPrivate {
public:
	explicit CartesianPlotPrivate(CartesianPlot*);
	~CartesianPlotPrivate();

	void retransform() override;
	void retransformYScale(int index);
	void retransformXScale(int index);
	void retransformScales(int xIndex, int yIndex);
	void rangeChanged();
	void xRangeFormatChanged();
	void yRangeFormatChanged();
	void mouseMoveZoomSelectionMode(QPointF logicalPos, int cSystemIndex);
	void mouseMoveSelectionMode(QPointF logicalStart, QPointF logicalEnd);
	void mouseMoveCursorMode(int cursorNumber, QPointF logicalPos);
	bool mouseReleaseZoomSelectionMode(int cSystemIndex, bool suppressRetransform=false);
	void mouseHoverZoomSelectionMode(QPointF logicPos, int cSystemIndex);
	void mouseHoverOutsideDataRect();
	void mousePressZoomSelectionMode(QPointF logicalPos, int cSystemIndex);
	void mousePressCursorMode(int cursorNumber, QPointF logicalPos);
	void updateCursor();
	void setZoomSelectionBandShow(bool show);
	bool translateRange(int xIndex, int yIndex, const QPointF& logicalStart, const QPointF& logicalEnd, bool translateX, bool translateY);

	CartesianPlot::Type type{CartesianPlot::Type::FourAxes};
	QString theme;
	QRectF dataRect;
	CartesianPlot::RangeType rangeType{CartesianPlot::RangeType::Free};
	int rangeFirstValues{1000}, rangeLastValues{1000};

//	const Range<double> xRange() const {
//		return xRanges.at(defaultCoordinateSystem()->xIndex()).range;
//	}
//	const Range<double> yRange() const {
//		return yRanges.at(defaultCoordinateSystem()->yIndex()).range;
//	}
	Range<double>& xRange() {
		return xRanges[defaultCoordinateSystem()->xIndex()].range;
	}
	Range<double>& yRange() {
		return yRanges[defaultCoordinateSystem()->yIndex()].range;
	}
	Range<double>& xRange(int index) {
		if (index >= 0)
			return xRanges[index].range;
		return xRange();
	}
	Range<double>& yRange(int index) {
		if (index >= 0)
			return yRanges[index].range;
		return yRange();
	}
	Range<double>& xRangeAutoScale(int index) {
		return xRanges[index].autoScaleRange;
	}
	Range<double>& yRangeAutoScale(int index) {
		return yRanges[index].autoScaleRange;
	}

	bool autoScaleX(int index = -1) {
		if (index == -1) {
			for (int i = 0; i < q->xRangeCount(); i++)
				if (!autoScaleX(i))
					return false;
			return true;
		}
		return xRanges[index].range.autoScale();
	}
	bool autoScaleY(int index = -1) {
		if (index == -1) {
			for (int i = 0; i < q->yRangeCount(); i++)
				if (!autoScaleY(i))
					return false;
			return true;
		}
		return yRanges[index].range.autoScale();
	}
	void setAutoScaleX(int index = -1, bool b = true) {
		if (index == -1) {
			for (int i = 0; i < q->xRangeCount(); i++)
				setAutoScaleX(i, b);
			return;
		}
		xRanges[index].range.setAutoScale(b);
	}
	void setAutoScaleY(int index = -1, bool b = true) {
		if (index == -1) {
			for (int i = 0; i < q->yRangeCount(); i++)
				setAutoScaleY(i, b);
			return;
		}
		yRanges[index].range.setAutoScale(b);
	}

	//the following factor determines the size of the offset between the min/max points of the curves
	//and the coordinate system ranges, when doing auto scaling
	//Factor 0 corresponds to the exact match - min/max values of the curves correspond to the start/end values of the ranges.
	//TODO: make this factor optional.
	//Provide in the UI the possibility to choose between "exact" or 0% offset, 2%, 5% and 10% for the auto fit option
	double autoScaleOffsetFactor{0.0};
	//TODO: move to Range?
	bool xRangeBreakingEnabled{false}, yRangeBreakingEnabled{false};
	CartesianPlot::RangeBreaks xRangeBreaks, yRangeBreaks;

	//cached values of minimum and maximum for all visible curves
	//Range<double> curvesXRange{qInf(), -qInf()}, curvesYRange{qInf(), -qInf()};

	CartesianPlot* const q;
	int defaultCoordinateSystemIndex{0};

	struct RichRange {
		RichRange(const Range<double>& r = Range<double>(), const bool d = false): range(r), dirty(d) {}
		Range<double> range; // current range
		Range<double> prev;
		Range<double> autoScaleRange; // autoscale range. Cached to be faster in rescaling
		bool dirty{false};
	};

	QVector<RichRange> xRanges{{}}, yRanges{{}}; // at least one range must exist.

	CartesianCoordinateSystem* coordinateSystem(int index) const;
	QVector<AbstractCoordinateSystem*> coordinateSystems() const;
	CartesianCoordinateSystem* defaultCoordinateSystem() const {
		return static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems.at(defaultCoordinateSystemIndex));
	}

	CartesianPlot::MouseMode mouseMode{CartesianPlot::MouseMode::Selection};
	bool suppressRetransform{false};
	bool panningStarted{false};
	bool locked{false};
	QPointF scenePos; //current position under the mouse cursor in scene coordinates
	QPointF logicalPos; //current position under the mouse cursor in plot coordinates
	bool calledFromContextMenu{false}; //we set the current position under the cursor when "add new" is called via the context menu

	// Cursor
	bool cursor0Enable{false};
	int selectedCursor{0};
	QPointF cursor0Pos{QPointF(qQNaN(), qQNaN())};
	bool cursor1Enable{false};
	QPointF cursor1Pos{QPointF(qQNaN(), qQNaN())};
	QPen cursorPen{Qt::red, Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point), Qt::SolidLine};

	//other mouse cursor modes
	QPen zoomSelectPen{Qt::black, 3, Qt::SolidLine};
	QPen crossHairPen{Qt::black, 2, Qt::DotLine};

signals:
	void mousePressZoomSelectionModeSignal(QPointF logicalPos);
	void mousePressCursorModeSignal(QPointF logicalPos);

private:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
	void wheelEvent(QGraphicsSceneWheelEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	void hoverMoveEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void updateDataRect();
	void checkXRange(int index);
	void checkYRange(int index);
	CartesianScale* createScale(RangeT::Scale,
		const Range<double> &sceneRange, const Range<double> &logicalRange);

	bool m_insideDataRect{false};
	bool m_selectionBandIsShown{false};
	QPointF m_selectionStart;
	QPointF m_selectionEnd;
	QLineF m_selectionStartLine;
	QPointF m_panningStart;
	QPointF m_crosshairPos; //current position of the mouse cursor in scene coordinates

	QStaticText m_cursor0Text{"1"};
	QStaticText m_cursor1Text{"2"};
};

#endif
