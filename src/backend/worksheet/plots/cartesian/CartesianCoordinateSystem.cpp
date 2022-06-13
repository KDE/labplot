/*
	File                 : CartesianCoordinateSystem.cpp
	Project              : LabPlot
	Description          : Cartesian coordinate system for plots.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2016 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystemPrivate.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

extern "C" {
#include "backend/nsl/nsl_math.h"
}

/* ============================================================================ */
/* ========================= coordinate system ================================ */
/* ============================================================================ */
/**
 * \class CartesianCoordinateSystem
 * \brief Cartesian coordinate system for plots.
 */
CartesianCoordinateSystem::CartesianCoordinateSystem(CartesianPlot* plot)
	: AbstractCoordinateSystem(plot)
	, d(new CartesianCoordinateSystemPrivate(this)) {
	d->plot = plot;
	// TODO: set some standard scales
}

CartesianCoordinateSystem::~CartesianCoordinateSystem() {
	delete d;
}

QString CartesianCoordinateSystem::info() const {
	DEBUG(Q_FUNC_INFO)
	if (d->plot)
		return QString(QLatin1String("x = ") + d->plot->xRange(d->xIndex).toString() + QLatin1String(", y = ") + d->plot->yRange(d->yIndex).toString());

	return i18n("no info available");
}

//##############################################################################
//######################### logical to scene mappers ###########################
//##############################################################################
Points CartesianCoordinateSystem::mapLogicalToScene(const Points& points, MappingFlags flags) const {
	// DEBUG(Q_FUNC_INFO << ", (points with flags)")
	const QRectF pageRect = d->plot->dataRect();
	const bool noPageClipping = pageRect.isNull() || (flags & MappingFlag::SuppressPageClipping);
	const bool noPageClippingY = flags & MappingFlag::SuppressPageClippingY;
	const bool limit = flags & MappingFlag::Limit;
	const double xPage = pageRect.x(), yPage = pageRect.y();
	const double w = pageRect.width(), h = pageRect.height();

	// DEBUG(Q_FUNC_INFO << ", xScales/YScales size: " << d->xScales.size() << '/' << d->yScales.size())

	Points result;
	result.reserve(points.size());
	for (const auto* xScale : d->xScales) {
		if (!xScale)
			continue;

		for (const auto* yScale : d->yScales) {
			if (!yScale)
				continue;

			for (const auto& point : points) {
				double x = point.x(), y = point.y();

				if (!xScale->contains(x) || !yScale->contains(y))
					continue;
				if (!xScale->map(&x) || !yScale->map(&y))
					continue;

				if (limit) {
					// set to max/min if passed over
					x = qBound(xPage, x, xPage + w);
					y = qBound(yPage, y, yPage + h);
				}

				if (noPageClippingY)
					y = yPage + h / 2.;

				const QPointF mappedPoint(x, y);
				if (noPageClipping || limit || rectContainsPoint(pageRect, mappedPoint))
					result.append(mappedPoint);
			}
		}
	}
	result.squeeze();

	return result;
}

/*!
	Maps the points in logical coordinates from @p points and fills the @p visiblePoints with the points in logical coordinates restricted to the current
   intervals.
	@param logicalPoints List of points in logical coordinates
	@param scenePoints List for the points in scene coordinates
	@param visiblePoints List for the logical coordinates restricted to the current region of the coordinate system
	@param flags
 */
void CartesianCoordinateSystem::mapLogicalToScene(const Points& logicalPoints,
												  Points& scenePoints,
												  std::vector<bool>& visiblePoints,
												  MappingFlags flags) const {
	// DEBUG(Q_FUNC_INFO << ", (curve with all points)")
	const QRectF pageRect = d->plot->dataRect();
	const bool noPageClipping = pageRect.isNull() || (flags & MappingFlag::SuppressPageClipping);
	const bool noPageClippingY = flags & MappingFlag::SuppressPageClippingY;
	const bool limit = flags & MappingFlag::Limit;
	const double xPage = pageRect.x(), yPage = pageRect.y();
	const double w = pageRect.width(), h = pageRect.height();

	// DEBUG(Q_FUNC_INFO << ", xScales/YScales size: " << d->xScales.size() << '/' << d->yScales.size())

	for (const auto* xScale : d->xScales) {
		if (!xScale)
			continue;

		for (const auto* yScale : d->yScales) {
			if (!yScale)
				continue;

			int i = 0;
			for (const auto& point : logicalPoints) {
				double x = point.x(), y = point.y();
				if (!xScale->contains(x) || !yScale->contains(y))
					continue;
				if (!xScale->map(&x) || !yScale->map(&y))
					continue;

				if (limit) {
					// set to max/min if passed over
					x = qBound(xPage, x, xPage + w);
					y = qBound(yPage, y, yPage + h);
				}

				if (noPageClippingY)
					y = yPage + h / 2.;

				const QPointF mappedPoint(x, y);
				if (noPageClipping || limit || rectContainsPoint(pageRect, mappedPoint)) {
					scenePoints.append(mappedPoint);
					visiblePoints[i] = true;
				} else
					visiblePoints[i] = false;

				i++;
			}
		}
	}
}

/*!
	Maps the points in logical coordinates from \c points and fills the \c visiblePoints with the points in logical coordinates restricted to the current
   intervals. If there are points, that lie on another one they will not be added a second time.
	@param logicalPoints List of points in logical coordinates
	@param scenePoints List for the points in scene coordinates
	@param visiblePoints List for the logical coordinates restricted to the current region of the coordinate system
 */
void CartesianCoordinateSystem::mapLogicalToScene(int startIndex,
												  int endIndex,
												  const Points& logicalPoints,
												  Points& scenePoints,
												  std::vector<bool>& visiblePoints,
												  MappingFlags flags) const {
	// DEBUG(Q_FUNC_INFO << ", (curve points)")
	const QRectF pageRect = d->plot->dataRect();
	const bool noPageClipping = pageRect.isNull() || (flags & MappingFlag::SuppressPageClipping);
	const bool noPageClippingY = flags & MappingFlag::SuppressPageClippingY;
	const bool limit = flags & MappingFlag::Limit;
	const double xPage = pageRect.x(), yPage = pageRect.y();
	const double w = pageRect.width(), h = pageRect.height();

	const int numberOfPixelX = pageRect.width();
	const int numberOfPixelY = pageRect.height();

	if (numberOfPixelX <= 0 || numberOfPixelY <= 0)
		return;

	// eliminate multiple scene points (size (numberOfPixelX + 1) * (numberOfPixelY + 1))
	QVector<QVector<bool>> scenePointsUsed(numberOfPixelX + 1);
	for (auto& col : scenePointsUsed)
		col.resize(numberOfPixelY + 1);

	const double minLogicalDiffX = pageRect.width() / numberOfPixelX;
	const double minLogicalDiffY = pageRect.height() / numberOfPixelY;

	// DEBUG(Q_FUNC_INFO << ", xScales/YScales size: " << d->xScales.size() << '/' << d->yScales.size())

	for (const auto* xScale : d->xScales) {
		if (!xScale)
			continue;

		for (const auto* yScale : d->yScales) {
			if (!yScale)
				continue;

			for (int i = startIndex; i <= endIndex; i++) {
				const QPointF& point = logicalPoints.at(i);

				double x = point.x(), y = point.y();
				if (!xScale->contains(x) || !yScale->contains(y))
					continue;
				if (!xScale->map(&x) || !yScale->map(&y))
					continue;

				if (limit) {
					// set to max/min if passed over
					x = qBound(xPage, x, xPage + w);
					y = qBound(yPage, y, yPage + h);
				}

				if (noPageClippingY)
					y = yPage + h / 2.;

				const QPointF mappedPoint(x, y);
				// DEBUG(mappedPoint.x() << ' ' << mappedPoint.y())
				if (noPageClipping || limit || rectContainsPoint(pageRect, mappedPoint)) {
					// TODO: check
					const int indexX = qRound((x - xPage) / minLogicalDiffX);
					const int indexY = qRound((y - yPage) / minLogicalDiffY);
					if (scenePointsUsed.at(indexX).at(indexY))
						continue;

					scenePointsUsed[indexX][indexY] = true;
					scenePoints.append(mappedPoint);
					// DEBUG(mappedPoint.x() << ' ' << mappedPoint.y())
					visiblePoints[i] = true;
				} else
					visiblePoints[i] = false;
			}
		}
	}
}

/*
 * Map a single point
 * */
QPointF CartesianCoordinateSystem::mapLogicalToScene(QPointF logicalPoint, bool& visible, MappingFlags flags) const {
	// DEBUG(Q_FUNC_INFO << ", (single point)")
	const QRectF pageRect = d->plot->dataRect();
	const bool noPageClipping = pageRect.isNull() || (flags & MappingFlag::SuppressPageClipping);
	const bool noPageClippingY = flags & MappingFlag::SuppressPageClippingY;
	const bool limit = flags & MappingFlag::Limit;

	double x = logicalPoint.x(), y = logicalPoint.y();
	const double xPage = pageRect.x(), yPage = pageRect.y();
	const double w = pageRect.width(), h = pageRect.height();

	// DEBUG(Q_FUNC_INFO << ", xScales/YScales size: " << d->xScales.size() << '/' << d->yScales.size())

	for (const auto* xScale : d->xScales) {
		if (!xScale)
			continue;

		for (const auto* yScale : d->yScales) {
			//DEBUG("SCALE " << yScale->range().toStdString())
			if (!yScale)
				continue;

			if (!xScale->valid(x) || !yScale->valid(y)) {
				//DEBUG("FAIL contains. y = " << y << ", y scale = " << yScale->range().toStdString())
				continue;
			}
			if (!xScale->map(&x) || !yScale->map(&y)) {
				//DEBUG("FAIL map")
				continue;
			}

			if (limit) {
				// set to max/min if passed over
				x = qBound(xPage, x, xPage + w);
				y = qBound(yPage, y, yPage + h);
			}

			if (noPageClippingY)
				y = pageRect.y() + h / 2.;

			QPointF mappedPoint(x, y);
			if (noPageClipping || limit || rectContainsPoint(pageRect, mappedPoint)) {
				visible = true;
				return mappedPoint;
			}
		}
	}

	visible = false;
	return QPointF{};
}

QLineF CartesianCoordinateSystem::gapMarker(qreal x, qreal y, qreal gap, bool xGap) const {
	if (xGap) {
		const auto xBreaks = d->plot->xRangeBreaks();
		const auto style = xBreaks.list.at(0).style;	// first break style
		QDEBUG(Q_FUNC_INFO << ", x gap style = " << style)
		switch (style) {
		case CartesianPlot::RangeBreakStyle::Vertical:
			return QLineF(x, y - gap / 2., x, y + gap / 2.);
		case CartesianPlot::RangeBreakStyle::Sloped:
			return QLineF(x + gap / 4., y - gap / 2., x - gap / 4., y + gap / 2.);
		case CartesianPlot::RangeBreakStyle::Simple:
		default:
			return {};
		}
	} else { // y gap
		const auto yBreaks = d->plot->yRangeBreaks();
		const auto style = yBreaks.list.at(0).style;	// first break style
		QDEBUG(Q_FUNC_INFO << ", y gap style = " << style)
		switch (style) {
		case CartesianPlot::RangeBreakStyle::Vertical:
			return QLineF(x + gap / 2., y, x - gap / 2., y);
		case CartesianPlot::RangeBreakStyle::Sloped:
			return QLineF(x + gap / 2., y - gap / 4., x - gap / 2., y + gap / 4.);
		case CartesianPlot::RangeBreakStyle::Simple:
		default:
			return {};
		}
	}
}

Lines CartesianCoordinateSystem::mapLogicalToScene(const Lines& lines, MappingFlags flags) const {
	DEBUG(Q_FUNC_INFO << ", FLAGS = " << (int)flags)
	QRectF pageRect = d->plot->dataRect();
	Lines result;
	const bool doPageClipping = !pageRect.isNull() && !(flags & MappingFlag::SuppressPageClipping);

	double xGapBefore;
	double xGapAfter = qQNaN();
	double yGapBefore;
	double yGapAfter = qQNaN();

	DEBUG(Q_FUNC_INFO << ", number of x/y scales: " << d->xScales.size() << " / " << d->yScales.size())

	QVectorIterator<CartesianScale*> xIterator(d->xScales);
	while (xIterator.hasNext()) {
		const CartesianScale* xScale = xIterator.next();
		if (!xScale)
			continue;

		xGapBefore = xGapAfter;
		if (xIterator.hasNext()) {
			const CartesianScale* nextXScale = xIterator.peekNext();
			if (!nextXScale)
				continue;
			Range<double> nextXRange;
			nextXScale->getParameter(&nextXRange);

			double x1 = xScale->end();
			double x2 = nextXScale->start();

			bool valid = xScale->map(&x1);
			if (valid)
				valid = nextXScale->map(&x2);
			if (valid)
				xGapAfter = x2 - x1;
			else
				xGapAfter = qQNaN();
		} else
			xGapAfter = qQNaN();

		QVectorIterator<CartesianScale*> yIterator(d->yScales);
		while (yIterator.hasNext()) {
			const CartesianScale* yScale = yIterator.next();
			if (!yScale)
				continue;

			yGapBefore = yGapAfter;
			if (yIterator.hasNext()) {
				const CartesianScale* nextYScale = yIterator.peekNext();
				if (!nextYScale)
					continue;

				double y1 = yScale->end();
				double y2 = nextYScale->start();

				bool valid = yScale->map(&y1);
				if (valid)
					valid = nextYScale->map(&y2);
				if (valid)
					yGapAfter = y2 - y1;
				else
					yGapAfter = qQNaN();
			} else
				yGapAfter = qQNaN();

			const QRectF scaleRect = QRectF(xScale->start(), yScale->start(), xScale->end() - xScale->start(), yScale->end() - yScale->start()).normalized();
			//QDEBUG("SCALE RECT:" << scaleRect)

			//QDEBUG("LINES:" << lines)
			for (auto line : lines) {
				// QDEBUG(Q_FUNC_INFO << ", LINE " << line)
				LineClipResult clipResult;
				if (!AbstractCoordinateSystem::clipLineToRect(&line, scaleRect, &clipResult))
					continue;

				double x1 = line.x1();
				if (!xScale->map(&x1))
					continue;

				double x2 = line.x2();
				if (!xScale->map(&x2))
					continue;

				double y1 = line.y1();
				if (!yScale->map(&y1))
					continue;

				double y2 = line.y2();
				if (!yScale->map(&y2))
					continue;

				QLineF mappedLine(QPointF(x1, y1), QPointF(x2, y2));
				if (doPageClipping) {
					if (!AbstractCoordinateSystem::clipLineToRect(&mappedLine, pageRect)) {
						DEBUG(Q_FUNC_INFO << ", WARNING: OMIT mapped line!")
						continue;
					}
				}
				//QDEBUG(Q_FUNC_INFO << ", append line " << mappedLine)
				result.append(mappedLine);

				// gap marker
				if (flags & MappingFlag::MarkGaps) {
					// end of x gap
					if (!std::isnan(xGapBefore)) {
						if (clipResult.xClippedLeft[0]) {
							// if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
							result.append(gapMarker(x1, y1, xGapBefore, 1));
						}
						if (clipResult.xClippedLeft[1]) {
							// if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
							result.append(gapMarker(x2, y2, xGapBefore, 1));
						}
					}

					// start of x gap
					if (!std::isnan(xGapAfter)) {
						if (clipResult.xClippedRight[0]) {
							// if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
							result.append(gapMarker(x1, y1, xGapAfter, 1));
						}
						if (clipResult.xClippedRight[1]) {
							// if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
							result.append(gapMarker(x2, y2, xGapAfter, 1));
						}
					}

					// end of y gap
					if (!std::isnan(yGapBefore)) {
						if (clipResult.yClippedTop[0]) {
							// if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
							result.append(gapMarker(x1, y1, yGapBefore, 0));
						}
						if (clipResult.yClippedTop[1]) {
							// if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
							result.append(gapMarker(x2, y2,yGapBefore, 0));
						}
					}

					// start of y gap
					if (!std::isnan(yGapAfter)) {
						if (clipResult.yClippedBottom[0]) {
							// if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
							result.append(gapMarker(x1, y1,yGapAfter, 0));
						}
						if (clipResult.yClippedBottom[1]) {
							// if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
							result.append(gapMarker(x2, y2, yGapAfter, 0));
						}
					}
				}

			}
		}
	}

	return result;
}

//##############################################################################
//######################### scene to logical mappers ###########################
//##############################################################################
Points CartesianCoordinateSystem::mapSceneToLogical(const Points& points, MappingFlags flags) const {
	DEBUG(Q_FUNC_INFO)
	QRectF pageRect = d->plot->dataRect();
	const bool noPageClipping = pageRect.isNull() || (flags & MappingFlag::SuppressPageClipping);
	const bool limit = flags & MappingFlag::Limit;
	const bool noPageClippingY = flags & MappingFlag::SuppressPageClippingY;
	const double xPage = pageRect.x();
	const double yPage = pageRect.y();
	const double w = pageRect.width();
	const double h = pageRect.height();

	Points result;
	for (const auto& point : points) {
		double x = point.x();
		double y = point.y();
		if (limit) {
			// set to max/min if passed over
			x = qBound(xPage, x, xPage + w);
			y = qBound(yPage, y, yPage + h);
		}

		if (noPageClippingY)
			y = yPage + h / 2.;

		if (noPageClipping || limit || pageRect.contains(point)) {
			bool found = false;

			for (const auto* xScale : d->xScales) {
				if (found)
					break;
				if (!xScale)
					continue;

				for (const auto* yScale : d->yScales) {
					if (found)
						break;
					if (!yScale)
						continue;

					if (!xScale->inverseMap(&x)) {
						x = point.x();
						continue;
					}
					//DEBUG("x = " << x)

					if (!yScale->inverseMap(&y)) {
						y = point.y();
						continue;
					}
					//DEBUG("y = " << y)

					if (!xScale->contains(x)) {
						x = point.x();
						continue;
					}

					if (!yScale->contains(y)) {
						y = point.y();
						continue;
					}

					//DEBUG("APPEND x/y")
					result.append(QPointF(x, y));
					found = true;
				}
			}
		}
	}

	return result;
}

QPointF CartesianCoordinateSystem::mapSceneToLogical(QPointF point, MappingFlags flags) const {
	QDEBUG(Q_FUNC_INFO << ", point =" << point)
	QRectF pageRect = d->plot->dataRect();
	QDEBUG(Q_FUNC_INFO << ", page rect =" << pageRect)
	bool noPageClipping = pageRect.isNull() || (flags & MappingFlag::SuppressPageClipping);
	bool limit = flags & MappingFlag::Limit;
	const bool noPageClippingY = flags & MappingFlag::SuppressPageClippingY;

	if (limit) {
		// set to max/min if out of limit
		point.setX(qBound(pageRect.x(), point.x(), pageRect.x() + pageRect.width()));
		point.setY(qBound(pageRect.y(), point.y(), pageRect.y() + pageRect.height()));
	}

	if (noPageClippingY)
		point.setY(pageRect.y() + pageRect.height() / 2.);

	// DEBUG(Q_FUNC_INFO << ", xScales/YScales size: " << d->xScales.size() << '/' << d->yScales.size())

	if (noPageClipping || limit || pageRect.contains(point)) {

		double x = point.x();
		double y = point.y();
		DEBUG(Q_FUNC_INFO << ", x/y = " << x << " " << y)

		for (const auto* xScale : d->xScales) {
			DEBUG(Q_FUNC_INFO << ", x scale " << xScale->clipRange().toStdString())
			if (!xScale)
				continue;
			for (const auto* yScale : d->yScales) {

				DEBUG(Q_FUNC_INFO << ", y scale " << yScale->clipRange().toStdString())
				if (!yScale)
					continue;

				if (!xScale->inverseMap(&x)) {
					x = point.x();
					continue;
				}
				if (!yScale->inverseMap(&y)) {
					y = point.y();
					continue;
				}

				if (!xScale->contains(x)) {
					x = point.x();
					continue;
				}
				if (!yScale->contains(y)) {
					y = point.y();
					continue;
				}
				//DEBUG(Q_FUNC_INFO << ", x/y = " << x << " / " << y << " NOT in scales!")

				DEBUG(Q_FUNC_INFO << ", FOUND in " << xScale->clipRange().toStdString() << " / " << yScale->clipRange().toStdString())
				DEBUG(Q_FUNC_INFO << ", RETURN " << x << " / " << y)
				return {x, y};
			}
		}
	}

	return {};
}

/**************************************************************************************/

/**
 * \brief Determine the horizontal direction relative to the page.
 *
 * This function is needed for untransformed lengths such as axis tick length.
 * \return 1 or -1
 */
int CartesianCoordinateSystem::xDirection() const {
	if (d->xScales.isEmpty() || !d->xScales.at(0)) {
		DEBUG(Q_FUNC_INFO << ", WARNING: no x scale!")
		return 1;
	}

	return d->xScales.at(0)->direction();
}

/**
 * \brief Determine the vertical direction relative to the page.
 *
 * This function is needed for untransformed lengths such as axis tick length.
 * \return 1 or -1
 */
int CartesianCoordinateSystem::yDirection() const {
	if (d->yScales.isEmpty() || !d->yScales.at(0)) {
		DEBUG(Q_FUNC_INFO << ", WARNING: no y scale!")
		return 1;
	}

	return d->yScales.at(0)->direction();
}

// TODO: design elegant, flexible and undo-aware API for changing scales
bool CartesianCoordinateSystem::setXScales(const QVector<CartesianScale*>& scales) {
	while (!d->xScales.isEmpty())
		delete d->xScales.takeFirst();

	d->xScales = scales;
	return true; // TODO: check scales validity
}

QVector<CartesianScale*> CartesianCoordinateSystem::xScales() const {
	return d->xScales; // TODO: should rather return a copy of the scales here
}

bool CartesianCoordinateSystem::setYScales(const QVector<CartesianScale*>& scales) {
	while (!d->yScales.isEmpty())
		delete d->yScales.takeFirst();

	d->yScales = scales;
	return true; // TODO: check scales validity
}

QVector<CartesianScale*> CartesianCoordinateSystem::yScales() const {
	return d->yScales; // TODO: should rather return a copy of the scales here
}

int CartesianCoordinateSystem::xIndex() const {
	return d->xIndex;
}
void CartesianCoordinateSystem::setXIndex(int index) {
	d->xIndex = index;
}

int CartesianCoordinateSystem::yIndex() const {
	return d->yIndex;
}
void CartesianCoordinateSystem::setYIndex(int index) {
	d->yIndex = index;
}

/*!
 * Adjusted the function QRectF::contains(QPointF) from Qt 4.8.4 to handle the
 * comparison of float numbers correctly.
 * TODO: check whether the newer versions of Qt do the comparison correctly.
 */
bool CartesianCoordinateSystem::rectContainsPoint(const QRectF& rect, QPointF point) const {
	qreal l = rect.x();
	qreal r = rect.x();
	qreal w = rect.width();
	qreal h = rect.height();
	if (w < 0)
		l += w;
	else
		r += w;
	if (nsl_math_essentially_equal(l, r)) // null rect
		return false;

	if (nsl_math_definitely_less_than(point.x(), l) || nsl_math_definitely_greater_than(point.x(), r))
		return false;

	qreal t = rect.y();
	qreal b = rect.y();
	if (h < 0)
		t += h;
	else
		b += h;
	if (nsl_math_essentially_equal(t, b)) // null rect
		return false;

	if (nsl_math_definitely_less_than(point.y(), t) || nsl_math_definitely_greater_than(point.y(), b))
		return false;

	return true;
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
CartesianCoordinateSystemPrivate::CartesianCoordinateSystemPrivate(CartesianCoordinateSystem* owner)
	: q(owner) {
}

CartesianCoordinateSystemPrivate::~CartesianCoordinateSystemPrivate() {
	while (!xScales.isEmpty())
		delete xScales.takeFirst();

	while (!yScales.isEmpty())
		delete yScales.takeFirst();
}
