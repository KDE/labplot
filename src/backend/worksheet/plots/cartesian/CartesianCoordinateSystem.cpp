
/***************************************************************************
    File                 : CartesianCoordinateSystem.cpp
    Project              : LabPlot
    Description          : Cartesian coordinate system for plots.
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2016 by Alexander Semke (alexander.semke@web.de)

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

#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

/* ============================================================================ */
/* =================================== scales ================================= */
/* ============================================================================ */
/**
 * \class CartesianScale
 * \brief Base class for cartesian coordinate system scales.
 */
CartesianScale::CartesianScale(ScaleType type, const Interval<double> &interval, double a, double b, double c)
	: m_type(type), m_interval(interval), m_a(a), m_b(b), m_c(c) {
}

CartesianScale::~CartesianScale() = default;

void CartesianScale::getProperties(ScaleType *type, Interval<double> *interval,
		double *a, double *b, double *c) const {
	if (type)
		*type = m_type;
	if (interval)
		*interval = m_interval;
	if (a)
		*a = m_a;
	if (b)
		*b = m_b;
	if (c)
		*c = m_c;
}

double CartesianScale::start() const {
	return m_interval.start();
}

double CartesianScale::end() const {
	return m_interval.end();
}

bool CartesianScale::contains(double value) const {
	return m_interval.contains(value);
}

/**
 * \class CartesianCoordinateSystem::LinearScale
 * \brief implementation of the linear scale for cartesian coordinate system.
 */
class LinearScale : public CartesianScale {
public:
	LinearScale(const Interval<double> &interval, double offset, double gradient)
		: CartesianScale(ScaleLinear, interval, offset, gradient, 0) {
			Q_ASSERT(gradient != 0.0);

		}

	~LinearScale() override = default;

	bool map(double *value) const override {
		*value = *value * m_b + m_a;
		return true;
	}

	bool inverseMap(double *value) const override {
		if (m_b == 0.0)
			return false;
		*value = (*value - m_a) / m_b;
		return true;
	}

	int direction() const override {
		return m_b < 0 ? -1 : 1;
	}
};

/**
 * \class CartesianCoordinateSystem::LinearScale
 * \brief implementation of the linear scale for cartesian coordinate system.
 */
class LogScale : public CartesianScale {
public:
	LogScale(const Interval<double> &interval, double offset, double scaleFactor, double base)
		: CartesianScale(ScaleLog, interval, offset, scaleFactor, base) {
			Q_ASSERT(scaleFactor != 0.0);
			Q_ASSERT(base > 0.0);
	}

	~LogScale() override = default;

	bool map(double *value) const override {
		if (*value > 0.0)
			*value = log(*value)/log(m_c) * m_b + m_a;
		else
			return false;

		return true;
	}

	bool inverseMap(double *value) const override {
		if (m_a == 0.0)
			return false;
		if (m_c <= 0.0)
			return false;

		*value = pow(m_c, (*value - m_a) / m_b);
		return true;
	}
	int direction() const override {
		return m_b < 0 ? -1 : 1;
	}
};

/* ============================================================================ */
/* ========================= coordinate system ================================ */
/* ============================================================================ */
//TODO: own header
class CartesianCoordinateSystemPrivate {
public:
	CartesianCoordinateSystemPrivate(CartesianCoordinateSystem *owner);
	~CartesianCoordinateSystemPrivate();

	CartesianCoordinateSystem* const q;
	CartesianPlot* plot{nullptr};
	QVector<CartesianScale*> xScales;
	QVector<CartesianScale*> yScales;
};

/**
 * \class CartesianCoordinateSystem
 * \brief Cartesian coordinate system for plots.
 */
CartesianCoordinateSystem::CartesianCoordinateSystem(CartesianPlot* plot)
		: AbstractCoordinateSystem(plot), d(new CartesianCoordinateSystemPrivate(this)) {
			d->plot = plot;
	// TODO: set some standard scales
}

CartesianCoordinateSystem::~CartesianCoordinateSystem() {
	delete d;
}

CartesianScale *CartesianScale::createScale(ScaleType type, const Interval<double> &interval, double a, double b, double c) {
	switch (type) {
		case ScaleLinear:
			return new LinearScale(interval, a, b);
		case ScaleLog:
			return new LogScale(interval, a, b, c);
		default:
			return nullptr;
	}
}

CartesianScale *CartesianScale::createLinearScale(const Interval<double> &interval,
		double sceneStart, double sceneEnd, double logicalStart, double logicalEnd) {
	DEBUG("CartesianScale::createLinearScale() scene start/end = " << sceneStart << '/' << sceneEnd << ", logical start/end = " << logicalStart << '/' << logicalEnd);

	double lDiff = logicalEnd - logicalStart;
	if (lDiff == 0.0)
		return nullptr;

	double b = (sceneEnd - sceneStart) / lDiff;
	double a = sceneStart - b * logicalStart;

	return new LinearScale(interval, a, b);
}

CartesianScale *CartesianScale::createLogScale(const Interval<double> &interval,
		double sceneStart, double sceneEnd, double logicalStart, double logicalEnd, double base) {

	if (base < 0.0 || base == 0.0)
		return nullptr;
	if (logicalStart < 0.0 || logicalStart == 0.0)
		return nullptr;
	if (logicalEnd < 0.0 || logicalEnd == 0.0)
		return nullptr;

	double lDiff = (log(logicalEnd) - log(logicalStart)) / log(base);
	if (lDiff == 0.0)
		return nullptr;

	double b = (sceneEnd - sceneStart) / lDiff;
	double a = sceneStart - b * log(logicalStart)/log(base);

	return new LogScale(interval, a, b, base);
}

//##############################################################################
//######################### logical to scene mappers ###########################
//##############################################################################
QVector<QPointF> CartesianCoordinateSystem::mapLogicalToScene(const QVector<QPointF> &points, MappingFlags flags) const {
	const QRectF pageRect = d->plot->dataRect();
	QVector<QPointF> result;
	bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);

	for (const auto* xScale : d->xScales) {
		if (!xScale) continue;

		for (const auto* yScale : d->yScales) {
			if (!yScale) continue;

			for (const auto& point : points) {
				double x = point.x();
				double y = point.y();

				if (!xScale->contains(x))
					continue;

				if (!yScale->contains(y))
					continue;

				if (!xScale->map(&x))
					continue;

				if (!yScale->map(&y))
					continue;

				const QPointF mappedPoint(x, y);
				if (noPageClipping || rectContainsPoint(pageRect, mappedPoint))
					result.append(mappedPoint);
			}
		}
	}

	return result;
}

/*!
	Maps the points in logical coordinates from \c points and fills the \c visiblePoints with the points in logical coordinates restricted to the current intervals.
	\param logicalPoints List of points in logical coordinates
	\param scenePoints List for the points in scene coordinates
	\param visiblePoints List for the logical coordinates restricted to the current region of the coordinate system
	\param flags
 */
void CartesianCoordinateSystem::mapLogicalToScene(const QVector<QPointF>& logicalPoints,
		QVector<QPointF>& scenePoints, std::vector<bool>& visiblePoints, MappingFlags flags) const {
	const QRectF pageRect = d->plot->dataRect();
	const bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);

	for (const auto* xScale : d->xScales) {
		if (!xScale) continue;

		for (const auto* yScale : d->yScales) {
			if (!yScale) continue;

			for (int i=0; i<logicalPoints.size(); ++i) {
				const QPointF& point = logicalPoints.at(i);

				double x = point.x();
				if (!xScale->contains(x))
					continue;

				if (!xScale->map(&x))
					continue;

				double y = point.y();
				if (!yScale->contains(y))
					continue;

				if (!yScale->map(&y))
					continue;

				const QPointF mappedPoint(x, y);
				if (noPageClipping || rectContainsPoint(pageRect, mappedPoint)) {
					scenePoints.append(mappedPoint);
					visiblePoints[i].flip();
				}
			}
		}
	}
}

QPointF CartesianCoordinateSystem::mapLogicalToScene(QPointF logicalPoint, MappingFlags flags) const {
	const QRectF pageRect = d->plot->dataRect();
	bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);

	double x = logicalPoint.x();
	double y = logicalPoint.y();

	for (const auto* xScale : d->xScales) {
		if (!xScale) continue;

		for (const auto* yScale : d->yScales) {
			if (!yScale) continue;

			if (!xScale->contains(x))
				continue;

			if (!xScale->map(&x))
				continue;

			if (!yScale->contains(y))
				continue;

			if (!yScale->map(&y))
				continue;

			QPointF mappedPoint(x, y);
			if (noPageClipping || rectContainsPoint(pageRect, mappedPoint))
				return mappedPoint;
		}
	}

	return QPointF{};
}

QVector<QLineF> CartesianCoordinateSystem::mapLogicalToScene(const QVector<QLineF> &lines, MappingFlags flags) const {
	QRectF pageRect = d->plot->dataRect();
	QVector<QLineF> result;
	const bool doPageClipping = !pageRect.isNull() && !(flags & SuppressPageClipping);

	double xGapBefore = NAN;
	double xGapAfter = NAN;
	double yGapBefore = NAN;
	double yGapAfter = NAN;

 	QVectorIterator<CartesianScale *> xIterator(d->xScales);
	while (xIterator.hasNext()) {
		const CartesianScale* xScale = xIterator.next();
		if (!xScale) continue;

		xGapBefore = xGapAfter;
		if (xIterator.hasNext()) {
			const CartesianScale* nextXScale = xIterator.peekNext();
			if (!nextXScale) continue;
			Interval<double> nextXInterval;
			nextXScale->getProperties(nullptr, &nextXInterval);

			double x1 = xScale->end();
			double x2 = nextXScale->start();

			bool valid = xScale->map(&x1);
			if (valid)
				valid = nextXScale->map(&x2);
			if (valid)
				xGapAfter = x2 - x1;
			else
				xGapAfter = NAN;
		} else
			xGapAfter = NAN;

		QVectorIterator<CartesianScale*> yIterator(d->yScales);
		while (yIterator.hasNext()) {
			const CartesianScale* yScale = yIterator.next();
			if (!yScale) continue;

			yGapBefore = yGapAfter;
			if (yIterator.hasNext()) {
				const CartesianScale* nextYScale = yIterator.peekNext();
				if (!nextYScale) continue;

				double y1 = yScale->end();
				double y2 = nextYScale->start();

				bool valid = yScale->map(&y1);
				if (valid)
					valid = nextYScale->map(&y2);
				if (valid)
					yGapAfter = y2 - y1;
				else
					yGapAfter = NAN;
			} else
				yGapAfter = NAN;

			const QRectF scaleRect = QRectF(xScale->start(), yScale->start(),
								xScale->end() - xScale->start(), yScale->end() - yScale->start()).normalized();

			for (auto line : lines) {
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

				if (flags & MarkGaps) {
					//mark the end of the gap
					if (!std::isnan(xGapBefore)) {
						if (clipResult.xClippedLeft[0]) {
							QLineF gapMarker(x1 + xGapBefore/4, y1 - xGapBefore/2, x1 - xGapBefore/4, y1 + xGapBefore/2);
// 							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
						if (clipResult.xClippedLeft[1]) {
							QLineF gapMarker(x2 + xGapBefore/4, y2 - xGapBefore/2, x2 - xGapBefore/4, y2 + xGapBefore/2);
// 							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
					}

					//mark the beginning of the gap
					if (!std::isnan(xGapAfter)) {
						if (clipResult.xClippedRight[0]) {
							QLineF gapMarker(x1 + xGapAfter/4, y1 - xGapAfter/2, x1 - xGapAfter/4, y1 + xGapAfter/2);
// 							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
						if (clipResult.xClippedRight[1]) {
							QLineF gapMarker(x2 + xGapAfter/4, y2 - xGapAfter/2, x2 - xGapAfter/4, y2 + xGapAfter/2);
// 							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
					}

					if (!std::isnan(yGapBefore)) {
						if (clipResult.yClippedTop[0]) {
							QLineF gapMarker(x1 + yGapBefore/2, y1 - yGapBefore/4, x1 - yGapBefore/2, y1 + yGapBefore/4);
// 							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
						if (clipResult.yClippedTop[1]) {
							QLineF gapMarker(x2 + yGapBefore/2, y2 - yGapBefore/4, x2 - yGapBefore/2, y2 + yGapBefore/4);
// 							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
					}

					if (!std::isnan(yGapAfter)) {
						if (clipResult.yClippedBottom[0]) {
							QLineF gapMarker(QPointF(x1 + yGapAfter / 2, y1 - yGapAfter / 4),
									QPointF(x1 - yGapAfter / 2, y1 + yGapAfter / 4));
							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
						if (clipResult.yClippedBottom[1]) {
							QLineF gapMarker(QPointF(x2 + yGapAfter / 2, y2 - yGapAfter / 4),
									QPointF(x2 - yGapAfter / 2, y2 + yGapAfter / 4));
							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
					}
				}

				QLineF mappedLine(QPointF(x1, y1), QPointF(x2, y2));
				if (doPageClipping) {
					if (!AbstractCoordinateSystem::clipLineToRect(&mappedLine, pageRect))
						continue;
				}

				result.append(mappedLine);
			}
		}
	}

	return result;
}

//##############################################################################
//######################### scene to logical mappers ###########################
//##############################################################################
QVector<QPointF> CartesianCoordinateSystem::mapSceneToLogical(const QVector<QPointF>& points, MappingFlags flags) const {
	QRectF pageRect = d->plot->dataRect();
	QVector<QPointF> result;
	bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);

	for (const auto& point : points) {
		if (noPageClipping || pageRect.contains(point)) {
			bool found = false;

			double x = point.x();
			double y = point.y();

			for (const auto* xScale : d->xScales) {
				if (found) break;
				if (!xScale) continue;

				for (const auto* yScale : d->yScales) {
					if (found) break;
					if (!yScale) continue;

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

					result.append(QPointF(x, y));
					found = true;
				}
			}
		}
	}

	return result;
}

QPointF CartesianCoordinateSystem::mapSceneToLogical(QPointF logicalPoint, MappingFlags flags) const {
	QRectF pageRect = d->plot->dataRect();
	QPointF result;
	bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);

	if (noPageClipping || pageRect.contains(logicalPoint)) {
		double x = logicalPoint.x();
		double y = logicalPoint.y();

		for (const auto* xScale : d->xScales) {
			if (!xScale) continue;

			for (const auto* yScale : d->yScales) {
				if (!yScale) continue;

				if (!xScale->inverseMap(&x))
					continue;

				if (!yScale->inverseMap(&y))
					continue;

				if (!xScale->contains(x))
					continue;

				if (!yScale->contains(y))
					continue;

				result.setX(x);
				result.setY(y);
				return result;
			}
		}
	}

	return result;
}

/**
 * \brief Determine the horizontal direction relative to the page.
 *
 * This function is needed for untransformed lengths such as axis tick length.
 * \return 1 or -1
 */
int CartesianCoordinateSystem::xDirection() const {
	if (d->xScales.isEmpty())
		return 1;

	return d->xScales.at(0)->direction();
}

/**
 * \brief Determine the vertical direction relative to the page.
 *
 * This function is needed for untransformed lengths such as axis tick length.
 * \return 1 or -1
 */
int CartesianCoordinateSystem::yDirection() const {
	if (d->yScales.isEmpty())
		return 1;

	return d->yScales.at(0)->direction();
}

// TODO: design elegant, flexible and undo-aware API for changing scales
bool CartesianCoordinateSystem::setXScales(const QVector<CartesianScale *> &scales) {
	while (!d->xScales.isEmpty())
		delete d->xScales.takeFirst();

	d->xScales = scales;
	return true; // TODO: check scales validity
}

QVector<CartesianScale*> CartesianCoordinateSystem::xScales() const {
	return d->xScales; // TODO: should rather return a copy of the scales here
}

bool CartesianCoordinateSystem::setYScales(const QVector<CartesianScale*> &scales) {
	while (!d->yScales.isEmpty())
		delete d->yScales.takeFirst();

	d->yScales = scales;
	return true; // TODO: check scales validity
}

QVector<CartesianScale*> CartesianCoordinateSystem::yScales() const {
	return d->yScales; // TODO: should rather return a copy of the scales here
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
	if ( AbstractCoordinateSystem::essentiallyEqual(l, r)) // null rect
		return false;

	if ( AbstractCoordinateSystem::definitelyLessThan(point.x(), l)
		|| AbstractCoordinateSystem::definitelyGreaterThan(point.x(), r) )
 	return false;

	qreal t = rect.y();
	qreal b = rect.y();
	if (h < 0)
		t += h;
	else
		b += h;
 	if ( AbstractCoordinateSystem::essentiallyEqual(t, b) ) // null rect
		return false;

	if ( AbstractCoordinateSystem::definitelyLessThan(point.y(), t)
		|| AbstractCoordinateSystem::definitelyGreaterThan(point.y(), b) )
		return false;

	return true;
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
CartesianCoordinateSystemPrivate::CartesianCoordinateSystemPrivate(CartesianCoordinateSystem *owner) :q(owner) {
}

CartesianCoordinateSystemPrivate::~CartesianCoordinateSystemPrivate() {
	while (!xScales.isEmpty())
		delete xScales.takeFirst();

	while (!yScales.isEmpty())
		delete yScales.takeFirst();
}
