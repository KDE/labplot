/***************************************************************************
    File                 : CartesianCoordinateSystem.cpp
    Project              : LabPlot/SciDAVis
    Description          : Cartesian coordinate system for plots.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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

#include "worksheet/CartesianCoordinateSystem.h"
#include "worksheet/WorksheetElementContainerPrivate.h"
#include "worksheet/Worksheet.h"
#include "lib/commandtemplates.h"
#include <QPen>
#include <QtDebug>
#include <QGraphicsItemGroup>
/**
 * \class CartesianCoordinateSystem
 * \brief Cartesian coordinate system for plots.
 *
 * 
 */


class CartesianCoordinateSystemPrivate: public WorksheetElementContainerPrivate {
	public:
		CartesianCoordinateSystemPrivate(CartesianCoordinateSystem *owner) 
			: WorksheetElementContainerPrivate(owner) {
		}

		~CartesianCoordinateSystemPrivate() {
		}

		QString name() const {
			return q->name();
		}

		QPointF position; //!< scene position of the origin
		qreal scaleX; //!< X ratio logical units / scene units
		qreal scaleY; //!< Y ratio logical units / scene units
};

CartesianCoordinateSystem::CartesianCoordinateSystem(const QString &name) 
		: AbstractCoordinateSystem(name, new CartesianCoordinateSystemPrivate(this)) {
	init();
}

CartesianCoordinateSystem::CartesianCoordinateSystem(const QString &name, CartesianCoordinateSystemPrivate *dd)
    : AbstractCoordinateSystem(name, dd) {
	init();
}

void CartesianCoordinateSystem::init() {
	Q_D(CartesianCoordinateSystem);

	d->position = QPointF(0, 0); 
	d->scaleX = 1.0;
	d->scaleY = -1.0;
}

CartesianCoordinateSystem::~CartesianCoordinateSystem() {
}


// TODO: support for axes/coordinate breaks

QList<QPointF> CartesianCoordinateSystem::mapLogicalToScene(const QList<QPointF> &points, const MappingFlags &flags) const {
	Q_D(const CartesianCoordinateSystem);

	Worksheet *worksheet = ancestor<Worksheet>();
	QRectF pageRect;
	if (worksheet)
		pageRect = worksheet->pageRect();

	QList<QPointF> result;

	bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);

	foreach(QPointF point, points) {
		point.setX(point.x() / d->scaleX + d->position.x());
		point.setY(point.y() / d->scaleY + d->position.y());
		if (noPageClipping || pageRect.contains(point))
			result.append(point);
	}

	return result;
}

QList<QPointF> CartesianCoordinateSystem::mapSceneToLogical(const QList<QPointF> &points, const MappingFlags &flags) const {
	Q_D(const CartesianCoordinateSystem);

	Worksheet *worksheet = ancestor<Worksheet>();
	QRectF pageRect;
	if (worksheet)
		pageRect = worksheet->pageRect();

	QList<QPointF> result;
	bool noPageClipping = pageRect.isNull() || (flags & SuppressPageClipping);

	foreach(QPointF point, points) {
		if (noPageClipping || pageRect.contains(point)) {
			point.setX((point.x() - d->position.x()) * d->scaleX);
			point.setY((point.y() - d->position.y()) * d->scaleY);
			result.append(point);
		}
	}

	return result;
}

QList<QLineF> CartesianCoordinateSystem::mapLogicalToScene(const QList<QLineF> &lines, const MappingFlags &flags) const {
	Q_D(const CartesianCoordinateSystem);

	Worksheet *worksheet = ancestor<Worksheet>();
	QRectF pageRect;
	if (worksheet)
		pageRect = worksheet->pageRect();

	QList<QLineF> result;

	bool doPageClipping = !pageRect.isNull() && !(flags & SuppressPageClipping);

	foreach(QLineF line, lines) {
		QPointF p1 = line.p1();
		QPointF p2 = line.p2();
			
		p1.setX(p1.x() / d->scaleX + d->position.x());
		p1.setY(p1.y() / d->scaleY + d->position.y());
		p2.setX(p2.x() / d->scaleX + d->position.x());
		p2.setY(p2.y() / d->scaleY + d->position.y());

		QLineF newLine(p1, p2);
		if (doPageClipping) {
			if (AbstractCoordinateSystem::clipLineToRect(&newLine, pageRect))
				result.append(newLine);
		}
		else
			result.append(newLine);
	}

	return result;
}

/**
 * \fn CartesianCoordinateSystem::CLASS_D_ACCESSOR_DECL(QPointF, position, Position)
 * \brief Get/set the position of the coordinate system origin on the page.
 */

CLASS_SHARED_D_READER_IMPL(CartesianCoordinateSystem, QPointF, position, position);
STD_SETTER_CMD_IMPL_F(CartesianCoordinateSystem, SetPosition, QPointF, position, q->retransform);
void CartesianCoordinateSystem::setPosition(const QPointF &position) {
	Q_D(CartesianCoordinateSystem);
	exec(new CartesianCoordinateSystemSetPositionCmd(d, position, tr("%1: move")));
}


/**
 * \fn CartesianCoordinateSystem::BASIC_D_ACCESSOR_DECL(qreal, scaleX, ScaleX)
 * \brief Get/set the X coordinate logical unit / scene unit ratio.
 */

BASIC_SHARED_D_READER_IMPL(CartesianCoordinateSystem, qreal, scaleX, scaleX);
STD_SETTER_CMD_IMPL_F(CartesianCoordinateSystem, SetScaleX, qreal, scaleX, q->retransform);
void CartesianCoordinateSystem::setScaleX(qreal scale) {
	Q_D(CartesianCoordinateSystem);
	if (scale != 0.0) // no proper fp comparison needed, just avoid division by zero in mapping functions
		exec(new CartesianCoordinateSystemSetScaleXCmd(d, scale, tr("%1: set X scale")));
}


/**
 * \fn CartesianCoordinateSystem::BASIC_D_ACCESSOR_DECL(qreal, scaleY, ScaleY)
 * \brief Get/set the Y coordinate logical unit / scene unit ratio.
 */

BASIC_SHARED_D_READER_IMPL(CartesianCoordinateSystem, qreal, scaleY, scaleY);
STD_SETTER_CMD_IMPL_F(CartesianCoordinateSystem, SetScaleY, qreal, scaleY, q->retransform);
void CartesianCoordinateSystem::setScaleY(qreal scale) {
	Q_D(CartesianCoordinateSystem);
	if (scale != 0.0) // no proper fp comparison needed, just avoid division by zero in mapping functions
		exec(new CartesianCoordinateSystemSetScaleYCmd(d, scale, tr("%1: set Y scale")));
}

/**
 * \brief Determine the horizontal direction relative to the page.
 *
 * This function is needed for untransformed lengths such as axis tick length.
 * \return 1 or -1
 */
int CartesianCoordinateSystem::xDirection() const {
	return scaleX() < 0 ? -1 : 1;
}

/**
 * \brief Determine the vertical direction relative to the page.
 *
 * This function is needed for untransformed lengths such as axis tick length.
 * \return 1 or -1
 */
int CartesianCoordinateSystem::yDirection() const {
	return scaleY() < 0 ? -1 : 1;
}

QGraphicsItem *CartesianCoordinateSystem::graphicsItem() const {
	return d_ptr;
}

