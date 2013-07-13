/***************************************************************************
    File                 : CartesianCoordinateSystem.cpp
    Project              : LabPlot/SciDAVis
    Description          : Cartesian coordinate system for plots.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2012 by Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses) 
                           
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
#include "backend/worksheet/Worksheet.h"
#include "backend/core/AbstractAspect.h"
#include "backend/lib/XmlStreamReader.h"
#include <cmath>
#include <QUndoCommand>
#include <QtGlobal>
#include <QDebug>
#include <limits>

/**
 * \class CartesianCoordinateSystem
 * \brief Cartesian coordinate system for plots.
 *
 * 
 */

/**
 * \class CartesianCoordinateSystem::Scale
 * \brief Base class for cartesian coordinate system scales.
 *
 * 
 */

/* ============================================================================ */
/* =================================== scales ================================= */
/* ============================================================================ */
CartesianCoordinateSystem::Scale::Scale(ScaleType type, const Interval<double> &interval, double a, double b, double c)
	: m_type(type), m_interval(interval), m_a(a), m_b(b), m_c(c) {
}

CartesianCoordinateSystem::Scale::~Scale() {}

void CartesianCoordinateSystem::Scale::getProperties(ScaleType *type, Interval<double> *interval, 
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

class CartesianCoordinateSystemSetScalePropertiesCmd : public QUndoCommand {
	public:
		CartesianCoordinateSystemSetScalePropertiesCmd(CartesianCoordinateSystem::Scale *target, 
				const Interval<double> &interval, double a, double b, double c) 
			: m_target(target), m_interval(interval), m_a(a), m_b(b), m_c(c) {
				// use in macro only
			}

		template <typename T> void swap(T *a, T *b) {
			T temp = *a;
			*a = *b;
			*b = temp;
		}

		virtual void redo() {
			swap< Interval<double> >(&m_interval, &m_target->m_interval);
			swap< double >(&m_a, &m_target->m_a);
			swap< double >(&m_b, &m_target->m_b);
			swap< double >(&m_c, &m_target->m_c);
		}

		virtual void undo() { redo(); }

	private:
		CartesianCoordinateSystem::Scale *m_target;
		Interval<double> m_interval;
		double m_a;
		double m_b;
		double m_c;
};


class LinearScale: public CartesianCoordinateSystem::Scale {
	public:
		virtual ~LinearScale() {}
		LinearScale(const Interval<double> &interval, double offset, double gradient) 
			: CartesianCoordinateSystem::Scale(ScaleLinear, interval, offset, gradient, 0) { Q_ASSERT(gradient != 0.0); }

		virtual bool map(double *value) const {
			*value = *value * m_b + m_a;
			return true;
		}

		virtual bool inverseMap(double *value) const {
			if (m_a == 0.0)
				return false;
			*value = (*value - m_a) / m_b;
			return true;
		}
		virtual int direction() const {
			return m_b < 0 ? -1 : 1;
		}
		virtual void getPropertiesOnResize(double ratio,
				ScaleType *type, Interval<double> *interval, double *a, double *b, double *c) const {
				*type = m_type;
				*interval = Interval<double>(m_interval.start() * ratio, m_interval.end() * ratio);
				*a = m_a * ratio;
				*b = m_b * ratio;
				*c = m_c;

		}
};

class LogScale: public CartesianCoordinateSystem::Scale {
	public:
		virtual ~LogScale() {}
		LogScale(const Interval<double> &interval, double offset, double scaleFactor, double base) 
			: CartesianCoordinateSystem::Scale(ScaleLog, interval, offset, scaleFactor, base) { 
				Q_ASSERT(scaleFactor != 0.0);
				Q_ASSERT(base > 0.0);
		}

		virtual bool map(double *value) const {
			if (*value > 0.0)
				*value = log(*value)/log(m_c) * m_b + m_a;
			else
				return false;
			
			return true;
		}

		virtual bool inverseMap(double *value) const {
			if (m_a == 0.0)
				return false;
			if (m_c <= 0.0)
				return false;

			*value = pow(m_c, (*value - m_a) / m_b);
			return true;
		}
		virtual int direction() const {
			return m_b < 0 ? -1 : 1;
		}
		virtual void getPropertiesOnResize(double ratio,
				ScaleType *type, Interval<double> *interval, double *a, double *b, double *c) const {
				*type = m_type;
				*interval = Interval<double>(m_interval.start() * ratio, m_interval.end() * ratio);
				*a = m_a * ratio;
				*b = m_b * ratio;
				*c = m_c;

		}
};


CartesianCoordinateSystem::Scale *CartesianCoordinateSystem::Scale::createScale(ScaleType type, const Interval<double> &interval, double a, double b, double c) {
	switch (type) {
		case ScaleLinear:
			return new LinearScale(interval, a, b);
		case ScaleLog:
			return new LogScale(interval, a, b, c);
		default:
			return NULL;
	}
}

CartesianCoordinateSystem::Scale *CartesianCoordinateSystem::Scale::createLinearScale(const Interval<double> &interval, 
		double sceneStart, double sceneEnd, double logicalStart, double logicalEnd) {

	double lDiff = logicalEnd - logicalStart;
	if (qFuzzyCompare(1 + lDiff, 1))
		return NULL;

	double b = (sceneEnd - sceneStart) / lDiff;
	double a = sceneStart - b * logicalStart;

	return new LinearScale(interval, a, b);
}

CartesianCoordinateSystem::Scale *CartesianCoordinateSystem::Scale::createLogScale(const Interval<double> &interval, 
		double sceneStart, double sceneEnd, double logicalStart, double logicalEnd, double base) {

	if (base < 0.0 || qFuzzyCompare(1 + base, 1))
		return NULL;
	if (logicalStart < 0.0 || qFuzzyCompare(1 + logicalStart, 1))
		return NULL;
	if (logicalEnd < 0.0 || qFuzzyCompare(1 + logicalEnd, 1))
		return NULL;

	double lDiff = (log(logicalEnd) - log(logicalStart)) / log(base);
	if (qFuzzyCompare(1 + lDiff, 1))
		return NULL;

	double b = (sceneEnd - sceneStart) / lDiff;
	double a = sceneStart - b * log(logicalStart)/log(base);

	return new LogScale(interval, a, b, base);
}

/* ============================================================================ */
/* ========================= coordinate system ================================ */
/* ============================================================================ */
class CartesianCoordinateSystemPrivate{
public:
		CartesianCoordinateSystemPrivate(CartesianCoordinateSystem *owner);
		~CartesianCoordinateSystemPrivate();

		CartesianCoordinateSystem* const q;
		CartesianPlot* plot;
		QList<CartesianCoordinateSystem::Scale *> xScales;
		QList<CartesianCoordinateSystem::Scale *> yScales;
};

CartesianCoordinateSystem::CartesianCoordinateSystem(CartesianPlot* plot) 
		: AbstractCoordinateSystem(plot), d(new CartesianCoordinateSystemPrivate(this)){
			d->plot=plot;
	// TODO: set some standard scales
}


CartesianCoordinateSystem::~CartesianCoordinateSystem(){
	delete d;
}

QList<QPointF> CartesianCoordinateSystem::mapLogicalToScene(const QList<QPointF> &points, const MappingFlags &flags) const {
	const QRectF pageRect = d->plot->plotRect();
	QList<QPointF> result;
	bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);

	foreach (Scale *xScale, d->xScales) {
		Interval<double> xInterval;
		xScale->getProperties(NULL, &xInterval);

		foreach (Scale *yScale, d->yScales) {
			Interval<double> yInterval;
			yScale->getProperties(NULL, &yInterval);

			foreach(QPointF point, points) {
				bool valid = true;
				double x = point.x();
				double y = point.y();

				if (!(xInterval.fuzzyContains(x) && yInterval.fuzzyContains(y)))
					continue;

				valid = xScale->map(&x);
				if (!valid)
					continue;

				valid = yScale->map(&y);
				if (!valid)
					continue;

				QPointF mappedPoint(x, y);
				if (noPageClipping || rectContainsPoint(pageRect, mappedPoint))
					result.append(mappedPoint);
			}
		}
	}

	return result;
}

/*!
	Maps the points in logical coordinates from \c points and fills the \c restrictedPoints with the points in logical coordinates restricted to the current intervals.
	\param logicalPoints List of points in logical coordinates
	\param scenePoints List for the points in scene coordinates
	\param restrictedLogicalPoints List for the logical coordinates restricted to the current region of the coordinate system
	\param flags
 */
void CartesianCoordinateSystem::mapLogicalToScene(const QList<QPointF>& logicalPoints,
												  QList<QPointF>& scenePoints,
												  std::vector<bool>& visiblePoints,
												  const MappingFlags& flags) const{
	const QRectF pageRect = d->plot->plotRect();
	QList<QPointF> result;
	bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);
	bool valid = true;

	foreach (Scale *xScale, d->xScales){
		Interval<double> xInterval;
		xScale->getProperties(NULL, &xInterval);

		foreach (Scale *yScale, d->yScales){
			Interval<double> yInterval;
			yScale->getProperties(NULL, &yInterval);

			for (int i=0; i<logicalPoints.size(); ++i) {
				const QPointF& point = logicalPoints.at(i);
				double x = point.x();
				double y = point.y();

				if (!(xInterval.fuzzyContains(x) && yInterval.fuzzyContains(y)))
					continue;

				valid = xScale->map(&x);
				if (!valid)
					continue;

				valid = yScale->map(&y);
				if (!valid)
					continue;

				QPointF mappedPoint(x, y);
				if (noPageClipping || rectContainsPoint(pageRect, mappedPoint)){
					scenePoints.append(mappedPoint);
					visiblePoints[i].flip();
				}
			}
		}
	}
}

QPointF CartesianCoordinateSystem::mapLogicalToScene(const QPointF& logicalPoint, const MappingFlags& flags) const{
	const QRectF pageRect = d->plot->plotRect();
	QList<QPointF> result;
	bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);
	bool valid = true;

	foreach (Scale *xScale, d->xScales){
		Interval<double> xInterval;
		xScale->getProperties(NULL, &xInterval);

		foreach (Scale *yScale, d->yScales){
			Interval<double> yInterval;
			yScale->getProperties(NULL, &yInterval);

			double x = logicalPoint.x();
			double y = logicalPoint.y();

			if (!(xInterval.fuzzyContains(x) && yInterval.fuzzyContains(y)))
				continue;

			valid = xScale->map(&x);
			if (!valid)
				continue;

			valid = yScale->map(&y);
			if (!valid)
				continue;

			QPointF mappedPoint(x, y);
			if (noPageClipping || rectContainsPoint(pageRect, mappedPoint))
				return mappedPoint;
		}
	}
	return QPointF();
}

QList<QPointF> CartesianCoordinateSystem::mapSceneToLogical(const QList<QPointF> &points, const MappingFlags &flags) const{
	QRectF pageRect = d->plot->rect();
	QList<QPointF> result;
	bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);

	foreach(QPointF point, points) {
		if (noPageClipping || pageRect.contains(point)) {
			bool found = false;

			double x = point.x();
			double y = point.y();
			
			foreach (Scale *xScale, d->xScales) {
				if (found) break;

				Interval<double> xInterval;
				xScale->getProperties(NULL, &xInterval);

				foreach (Scale *yScale, d->yScales) {
					if (found) break;

					Interval<double> yInterval;
					yScale->getProperties(NULL, &yInterval);

					bool valid = true;

					valid = xScale->inverseMap(&x);
					if (!valid)
						continue;

					valid = yScale->inverseMap(&y);
					if (!valid)
						continue;

					if (!(xInterval.fuzzyContains(x) && yInterval.fuzzyContains(y)))
						continue;

					result.append(QPointF(x, y));
					found = true;
				}
			}
		}
	}

	return result;
}

QPointF CartesianCoordinateSystem::mapSceneToLogical(const QPointF& logicalPoint, const MappingFlags& flags) const {
	QRectF pageRect = d->plot->rect();
	QPointF result;
	bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);

	if (noPageClipping || pageRect.contains(logicalPoint)) {
		double x = logicalPoint.x();
		double y = logicalPoint.y();

		foreach (Scale *xScale, d->xScales) {
			Interval<double> xInterval;
			xScale->getProperties(NULL, &xInterval);

			foreach (Scale *yScale, d->yScales) {
				Interval<double> yInterval;
				yScale->getProperties(NULL, &yInterval);

				bool valid = true;

				valid = xScale->inverseMap(&x);
				if (!valid)
					continue;

				valid = yScale->inverseMap(&y);
				if (!valid)
					continue;

				if (!(xInterval.fuzzyContains(x) && yInterval.fuzzyContains(y)))
					continue;

				result.setX(x);
				result.setY(y);
				return result;
			}
		}
	}

	return result;
}

QList<QLineF> CartesianCoordinateSystem::mapLogicalToScene(const QList<QLineF> &lines, const MappingFlags &flags) const{
	//determine the plot rect in local coordinates
	QRectF pageRect = d->plot->graphicsItem()->mapFromScene( d->plot->rect() ).boundingRect();
	pageRect.setX(pageRect.x() + d->plot->horizontalPadding());
	pageRect.setY(pageRect.y() + d->plot->verticalPadding());
	pageRect.setWidth(pageRect.width() - d->plot->horizontalPadding());
	pageRect.setHeight(pageRect.height() - d->plot->verticalPadding());
	
	QList<QLineF> result;
	bool doPageClipping = !pageRect.isNull() && !(flags & SuppressPageClipping);

	double xGapBefore = NAN;
	double xGapAfter = NAN;
	double yGapBefore = NAN;
	double yGapAfter = NAN;

 	QListIterator<Scale *> xIterator(d->xScales);
	while (xIterator.hasNext()) {
		Scale *xScale = xIterator.next();
		Interval<double> xInterval;
		xScale->getProperties(NULL, &xInterval);
		
		xGapBefore = xGapAfter;
		if (xIterator.hasNext()) {
			Scale *nextXScale = xIterator.peekNext();
			Interval<double> nextXInterval;
			nextXScale->getProperties(NULL, &nextXInterval);
			double x1 = xInterval.end();
			double x2 = nextXInterval.start();
			bool valid = true;
			valid = xScale->map(&x1);
			if (valid)
				valid = nextXScale->map(&x2);
			if (valid)
				xGapAfter = x2 - x1;
			else
				xGapAfter = NAN;
		} else
			xGapAfter = NAN;

		QListIterator<Scale *> yIterator(d->yScales);
		while (yIterator.hasNext()) {
			Scale *yScale = yIterator.next();
			Interval<double> yInterval;
			yScale->getProperties(NULL, &yInterval);

			yGapBefore = yGapAfter;
			if (yIterator.hasNext()) {
				Scale *nextYScale = yIterator.peekNext();
				Interval<double> nextYInterval;
				nextYScale->getProperties(NULL, &nextYInterval);
				double y1 = yInterval.end();
				double y2 = nextYInterval.start();
				bool valid = true;
				valid = yScale->map(&y1);
				if (valid)
					valid = nextYScale->map(&y2);
				if (valid)
					yGapAfter = y2 - y1;
				else
					yGapAfter = NAN;
			} else
				yGapAfter = NAN;

			QRectF scaleRect = QRectF(xInterval.start(), yInterval.start(), 
					xInterval.end() - xInterval.start(), yInterval.end() - yInterval.start()).normalized();

			foreach(QLineF line, lines) {

				LineClipResult clipResult;
				if (!AbstractCoordinateSystem::clipLineToRect(&line, scaleRect, &clipResult))
					continue;

				double x1 = line.x1();
				double x2 = line.x2();
				double y1 = line.y1();
				double y2 = line.y2();

				bool valid = true;

				valid = xScale->map(&x1);
				if (!valid)
					continue;

				valid = xScale->map(&x2);
				if (!valid)
					continue;

				valid = yScale->map(&y1);
				if (!valid)
					continue;

				valid = yScale->map(&y2);
				if (!valid)
					continue;
	

				if (flags & MarkGaps) {
					if (!isnan(xGapBefore)) {
						if (clipResult.xClippedLeft[0]) {
							QLineF gapMarker(QPointF(x1 + xGapBefore / 4, y1 - xGapBefore / 2), 
									QPointF(x1 - xGapBefore / 4, y1 + xGapBefore / 2));
							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
						if (clipResult.xClippedLeft[1]) {
							QLineF gapMarker(QPointF(x2 + xGapBefore / 4, y2 - xGapBefore / 2), 
									QPointF(x2 - xGapBefore / 4, y2 + xGapBefore / 2));
							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
					}

					if (!isnan(xGapAfter)) {
						if (clipResult.xClippedRight[0]) {
							QLineF gapMarker(QPointF(x1 + xGapAfter / 4, y1 - xGapAfter / 2), 
									QPointF(x1 - xGapAfter / 4, y1 + xGapAfter / 2));
							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
						if (clipResult.xClippedRight[1]) {
							QLineF gapMarker(QPointF(x2 + xGapAfter / 4, y2 - xGapAfter / 2), 
									QPointF(x2 - xGapAfter / 4, y2 + xGapAfter / 2));
							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
					}

					if (!isnan(yGapBefore)) {
						if (clipResult.yClippedTop[0]) {
							QLineF gapMarker(QPointF(x1 + yGapBefore / 2, y1 - yGapBefore / 4), 
									QPointF(x1 - yGapBefore / 2, y1 + yGapBefore / 4));
							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
						if (clipResult.yClippedTop[1]) {
							QLineF gapMarker(QPointF(x2 + yGapBefore / 2, y2 - yGapBefore / 4), 
									QPointF(x2 - yGapBefore / 2, y2 + yGapBefore / 4));
							if (AbstractCoordinateSystem::clipLineToRect(&gapMarker, pageRect))
								result.append(gapMarker);
						}
					}

					if (!isnan(yGapAfter)) {
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
int CartesianCoordinateSystem::yDirection() const{
	if (d->yScales.isEmpty())
		return 1;

	return d->yScales.at(0)->direction();
}

// TODO: design elegant, flexible and undo-avare API for changing scales
bool CartesianCoordinateSystem::setXScales(const QList<Scale *> &scales) {
	d->xScales = scales;
	return true; // TODO: check scales validity
}

QList<CartesianCoordinateSystem::Scale *> CartesianCoordinateSystem::xScales() const {
	return d->xScales; // TODO: should rather return a copy of the scales here
}

bool CartesianCoordinateSystem::setYScales(const QList<Scale *> &scales) {
	d->yScales = scales;
	return true; // TODO: check scales validity
}

QList<CartesianCoordinateSystem::Scale *> CartesianCoordinateSystem::yScales() const {
	return d->yScales; // TODO: should rather return a copy of the scales here
}

void CartesianCoordinateSystem::handlePageResize(double horizontalRatio, double verticalRatio) {
	Scale::ScaleType type;
	Interval<double> interval;
	double a, b, c;

	d->plot->beginMacro(QObject::tr("adjust to page size"));
	foreach (Scale *xScale, d->xScales) {
		xScale->getPropertiesOnResize(horizontalRatio, &type, &interval, &a, &b, &c);
		d->plot->exec(new CartesianCoordinateSystemSetScalePropertiesCmd(xScale, interval, a, b, c));	
	}

	foreach (Scale *yScale, d->yScales) {
		yScale->getPropertiesOnResize(verticalRatio, &type, &interval, &a, &b, &c);
		d->plot->exec(new CartesianCoordinateSystemSetScalePropertiesCmd(yScale, interval, a, b, c));
	}
	d->plot->endMacro();
}

/*!
 * Adjusted the function QRectF::contains(QPointF) from Qt 4.8.4 to handle the 
 * comparison of float numbers correctly.
 * TODO: check whether the newer versions of Qt do the comparison correctly.
 */
bool CartesianCoordinateSystem::rectContainsPoint(const QRectF& rect, const QPointF& point) const{
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
CartesianCoordinateSystemPrivate::CartesianCoordinateSystemPrivate(CartesianCoordinateSystem *owner)
	:q(owner), plot(0){
}

CartesianCoordinateSystemPrivate::~CartesianCoordinateSystemPrivate(){
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
void CartesianCoordinateSystem::save(QXmlStreamWriter* writer) const{
	Q_UNUSED(writer);
}

bool CartesianCoordinateSystem::load(XmlStreamReader* reader){
	Q_UNUSED(reader);
	return true;
}
