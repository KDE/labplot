/*
	File                 : CartesianPlotPrivate.h
	Project              : LabPlot
	Description          : Private members of CartesianPlot.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANPLOTPRIVATE_H
#define CARTESIANPLOTPRIVATE_H

#include "../AbstractPlotPrivate.h"
#include "CartesianCoordinateSystem.h"
#include "CartesianPlot.h"

#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QStaticText>

class CartesianPlotPrivate : public AbstractPlotPrivate {
public:
	explicit CartesianPlotPrivate(CartesianPlot*);
	~CartesianPlotPrivate();

	void retransform() override;
	void retransformScale(const Dimension, int index, bool suppressSignals = false);
	void retransformScales(int xIndex, int yIndex);
	void rangeChanged();
	void niceExtendChanged();
	void rangeFormatChanged(const Dimension dim);
	void wheelEvent(const QPointF& sceneRelPos, int delta, int xIndex, int yIndex, bool considerDimension, Dimension dim);
	void mouseMoveZoomSelectionMode(QPointF logicalPos, int cSystemIndex);
	void mouseMoveSelectionMode(QPointF logicalStart, QPointF logicalEnd);
	void mouseMoveCursorMode(int cursorNumber, QPointF logicalPos);
	void mouseReleaseZoomSelectionMode(int cSystemIndex, bool suppressRetransform = false);
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

	struct RichRange {
		RichRange(const Range<double>& r = Range<double>())
			: range(r) {
			if (!range.autoScale())
				dirty = true;
			else
				dataRange = range;
		}
		Range<double> range; // current range
		Range<double> prev{Range<double>(NAN, NAN)};
		Range<double> dataRange; // range of data in plot. Cached to be faster in autoscaling/rescaling
		bool dirty{false}; // recalculate the range before displaying, because data range or display range changed
	};

	QVector<RichRange>& ranges(const Dimension dim) {
		switch (dim) {
		case Dimension::X:
			return xRanges;
		case Dimension::Y:
			return yRanges;
		}
		return yRanges;
	}

	bool rangeDirty(const Dimension dim, int index) const {
		if (index < -1 || index >= rangeCount(dim)) {
			DEBUG(Q_FUNC_INFO << QStringLiteral("WARNING: wrong index: %1").arg(index).toStdString());
			return false;
		} else if (index == -1)
			index = defaultCoordinateSystem()->index(dim);
		switch (dim) {
		case Dimension::X:
			return xRanges.at(index).dirty;
		case Dimension::Y:
			return yRanges.at(index).dirty;
		}
		return false;
	}

	void setRangeDirty(const Dimension dim, int index, const bool dirty) {
		if (index < -1 || index >= rangeCount(dim)) {
			DEBUG(Q_FUNC_INFO << QStringLiteral("WARNING: wrong index: %1").arg(index).toStdString());
			return;
		} else if (index == -1)
			index = defaultCoordinateSystem()->index(dim);
		switch (dim) {
		case Dimension::X:
			xRanges[index].dirty = dirty;
			break;
		case Dimension::Y:
			yRanges[index].dirty = dirty;
			break;
		}
	}

	void setRange(const Dimension dim, int index, const Range<double>& range) {
		if (index < -1 || index >= rangeCount(dim)) {
			DEBUG(Q_FUNC_INFO << QStringLiteral("WARNING: wrong index: %1").arg(index).toStdString());
			return;
		} else if (index == -1)
			index = defaultCoordinateSystem()->index(dim);
		switch (dim) {
		case Dimension::X:
			xRanges[index].range = range;
			break;
		case Dimension::Y:
			yRanges[index].range = range;
			break;
		}
	}

	void setFormat(const Dimension dim, int index, RangeT::Format format) {
		if (index < -1 || index >= rangeCount(dim)) {
			DEBUG(Q_FUNC_INFO << QStringLiteral("WARNING: wrong index: %1").arg(index).toStdString());
			return;
		} else if (index == -1)
			index = defaultCoordinateSystem()->index(dim);
		switch (dim) {
		case Dimension::X:
			xRanges[index].range.setFormat(format);
			break;
		case Dimension::Y:
			yRanges[index].range.setFormat(format);
			break;
		}
	}

	void setScale(const Dimension dim, int index, RangeT::Scale scale) {
		if (index < -1 || index >= rangeCount(dim)) {
			DEBUG(Q_FUNC_INFO << QStringLiteral("WARNING: wrong index: %1").arg(index).toStdString());
			index = defaultCoordinateSystem()->index(dim);
		} else if (index == -1)
			index = defaultCoordinateSystem()->index(dim);
		switch (dim) {
		case Dimension::X:
			xRanges[index].range.setScale(scale);
			break;
		case Dimension::Y:
			yRanges[index].range.setScale(scale);
			break;
		}
	}

	Range<double>& range(const Dimension dim, int index = -1) {
		if (index < -1 || index >= rangeCount(dim)) {
			DEBUG(Q_FUNC_INFO << QStringLiteral("WARNING: wrong index: %1").arg(index).toStdString());
			index = defaultCoordinateSystem()->index(dim);
		} else if (index == -1)
			index = defaultCoordinateSystem()->index(dim);
		switch (dim) {
		case Dimension::X:
			return xRanges[index].range;
		case Dimension::Y:
			return yRanges[index].range;
		}
		return yRanges[index].range;
	}

	const Range<double>& rangeConst(const Dimension dim, int index = -1) const {
		if (index < -1 || index >= rangeCount(dim)) {
			DEBUG(Q_FUNC_INFO << QStringLiteral("WARNING: wrong index: %1").arg(index).toStdString());
			index = defaultCoordinateSystem()->index(dim);
		} else if (index == -1)
			index = defaultCoordinateSystem()->index(dim);
		switch (dim) {
		case Dimension::X:
			return xRanges[index].range;
		case Dimension::Y:
			return yRanges[index].range;
		}
		return yRanges[index].range;
	}

	Range<double>& dataRange(const Dimension dim, int index = -1) {
		if (index < -1 || index >= rangeCount(dim)) {
			DEBUG(Q_FUNC_INFO << QStringLiteral("WARNING: wrong index: %1").arg(index).toStdString());
			index = defaultCoordinateSystem()->index(dim);
		} else if (index == -1)
			index = defaultCoordinateSystem()->index(dim);

		switch (dim) {
		case Dimension::X:
			return xRanges[index].dataRange;
		case Dimension::Y:
			break;
		}
		return yRanges[index].dataRange;
	}

	bool autoScale(const Dimension dim, int index = -1) const {
		if (index < -1 || index >= rangeCount(dim)) {
			DEBUG(Q_FUNC_INFO << QStringLiteral("WARNING: wrong index: %1").arg(index).toStdString());
			return false;
		}
		if (index == -1) {
			for (int i = 0; i < rangeCount(dim); i++)
				if (!autoScale(dim, i))
					return false;
			return true;
		}

		switch (dim) {
		case Dimension::X:
			return xRanges[index].range.autoScale();
		case Dimension::Y:
			return yRanges[index].range.autoScale();
		}
		return yRanges[index].range.autoScale();
	}

	int rangeCount(const Dimension dim) const {
		switch (dim) {
		case Dimension::X:
			return xRanges.size();
		case Dimension::Y:
			return yRanges.size();
		}
		return 0;
	}

	void enableAutoScale(const Dimension dim, int index = -1, bool b = true) {
		if (index < -1 || index >= rangeCount(dim)) {
			DEBUG(Q_FUNC_INFO << QStringLiteral("WARNING: wrong index: %1").arg(index).toStdString());
			return;
		}
		if (index == -1) {
			for (int i = 0; i < rangeCount(dim); i++)
				enableAutoScale(dim, i, b);
			return;
		}

		switch (dim) {
		case Dimension::X:
			xRanges[index].range.setAutoScale(b);
			break;
		case Dimension::Y:
			yRanges[index].range.setAutoScale(b);
			break;
		}
	}

	void checkRange(Dimension, int index);
	Range<double> checkRange(const Range<double>&);
	CartesianPlot::RangeBreaks rangeBreaks(Dimension);
	bool rangeBreakingEnabled(Dimension);

	// the following factor determines the size of the offset between the min/max points of the curves
	// and the coordinate system ranges, when doing auto scaling
	// Factor 0 corresponds to the exact match - min/max values of the curves correspond to the start/end values of the ranges.
	// TODO: make this factor optional.
	// Provide in the UI the possibility to choose between "exact" or 0% offset, 2%, 5% and 10% for the auto fit option
	double autoScaleOffsetFactor{0.0};
	// TODO: move to Range?
	bool xRangeBreakingEnabled{false}, yRangeBreakingEnabled{false};
	CartesianPlot::RangeBreaks xRangeBreaks, yRangeBreaks;

	// cached values of minimum and maximum for all visible curves
	// Range<double> curvesXRange{qInf(), -qInf()}, curvesYRange{qInf(), -qInf()};

	CartesianPlot* const q;
	int defaultCoordinateSystemIndex{0};

	QVector<RichRange> xRanges{{}}, yRanges{{}}; // at least one range must exist.
	bool niceExtend{false};
	CartesianCoordinateSystem* coordinateSystem(int index) const;
	QVector<AbstractCoordinateSystem*> coordinateSystems() const;
	CartesianCoordinateSystem* defaultCoordinateSystem() const {
		return static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems.at(defaultCoordinateSystemIndex));
	}

	CartesianPlot::MouseMode mouseMode{CartesianPlot::MouseMode::Selection};
	bool panningStarted{false};
	bool interactive{true};
	QPointF scenePos; // current position under the mouse cursor in scene coordinates
	QPointF logicalPos; // current position under the mouse cursor in plot coordinates
	bool calledFromContextMenu{false}; // we set the current position under the cursor when "add new" is called via the context menu

	// Cursor
	bool cursor0Enable{false};
	int selectedCursor{0};
	QPointF cursor0Pos{QPointF(qQNaN(), qQNaN())};
	bool cursor1Enable{false};
	QPointF cursor1Pos{QPointF(qQNaN(), qQNaN())};
	Line* cursorLine{nullptr};

	// other mouse cursor modes
	QPen zoomSelectPen{Qt::black, 3, Qt::SolidLine};
	QPen crossHairPen{Qt::black, 2, Qt::DotLine};

Q_SIGNALS:
	void mousePressZoomSelectionModeSignal(QPointF logicalPos);
	void mousePressCursorModeSignal(QPointF logicalPos);

private:
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
	virtual void wheelEvent(QGraphicsSceneWheelEvent*) override;
	virtual void keyPressEvent(QKeyEvent*) override;
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent*) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void updateDataRect();
	CartesianScale* createScale(RangeT::Scale, const Range<double>& sceneRange, const Range<double>& logicalRange);

	void navigateNextPrevCurve(bool next = true) const;

	bool m_insideDataRect{false};
	bool m_selectionBandIsShown{false};
	QPointF m_selectionStart;
	QPointF m_selectionEnd;
	QLineF m_selectionStartLine;
	QPointF m_panningStart;
	QPointF m_crosshairPos; // current position of the mouse cursor in scene coordinates

	QStaticText m_cursor0Text{QStringLiteral("1")};
	QStaticText m_cursor1Text{QStringLiteral("2")};

	friend class MultiRangeTest2;
	friend class CartesianPlotTest;
};

#endif
