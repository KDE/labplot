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
#include "lib/macros.h"
#include "lib/commandtemplates.h"
#include <QUndoCommand>

/**
 * \class CartesianCoordinateSystem
 * \brief Cartesian coordinate system for plots.
 *
 * 
 */


class CartesianCoordinateSystem::Private {
	public:
		Private(CartesianCoordinateSystem *owner) : q(owner) {
		}

		~Private() {
		}

		QString name() const {
			return q->name();
		}

		QPointF position; //!< scene position of the origin
		qreal scaleX; //!< X ratio logical units / scene units
		qreal scaleY; //!< Y ratio logical units / scene units

	private:
		CartesianCoordinateSystem * const q;
};

CartesianCoordinateSystem::CartesianCoordinateSystem(const QString &name) 
	: AbstractCoordinateSystem(name), d(new Private(this)) {
		d->position = QPointF(0, 0); 
		d->scaleX = 1.0;
		d->scaleY = 1.0;
}

CartesianCoordinateSystem::~CartesianCoordinateSystem() {
	delete d;
}

QPointF CartesianCoordinateSystem::mapLogicalToScene(const QPointF &point) const {
	QPointF result = point;
	result.setX(result.x() / d->scaleX + d->position.x());
	result.setY(result.y() / d->scaleY + d->position.y());
	return result;
}

QPointF CartesianCoordinateSystem::mapSceneToLogical(const QPointF &point) const {
	QPointF result = point;
	result.setX((result.x() - d->position.x()) * d->scaleX);
	result.setY((result.y() - d->position.y()) * d->scaleY);
	return result;
}

class CartesianCoordinateSystemSetPositionCmd: public StandardClassSetterCmd<CartesianCoordinateSystem::Private, QPointF> {
	public:
		CartesianCoordinateSystemSetPositionCmd(CartesianCoordinateSystem::Private *target, const QPointF &newValue, const QString &description)
			: StandardClassSetterCmd<CartesianCoordinateSystem::Private, QPointF>(target, newValue, description) {}
		virtual void *targetFieldAddress() { return &(m_target->position); }
};

/**
 * \fn CartesianCoordinateSystem::CLASS_D_ACCESSOR_DECL(QPointF, position, Position)
 * \brief Get/set the position of the coordinate system origin on the page.
 */

CLASS_D_READER_IMPL(CartesianCoordinateSystem, QPointF, position, position);
void CartesianCoordinateSystem::setPosition(const QPointF &position) {
	exec(new CartesianCoordinateSystemSetPositionCmd(d, position, tr("%1: move")));
}

class CartesianCoordinateSystemSetScaleXCmd: public StandardBasicSetterCmd<CartesianCoordinateSystem::Private, qreal> {
	public:
		CartesianCoordinateSystemSetScaleXCmd(CartesianCoordinateSystem::Private *target, qreal newValue, const QString &description)
			: StandardBasicSetterCmd<CartesianCoordinateSystem::Private, qreal>(target, newValue, description) {}
		virtual void *targetFieldAddress() { return &(m_target->scaleX); }
};

/**
 * \fn CartesianCoordinateSystem::BASIC_D_ACCESSOR_DECL(qreal, scaleX, ScaleX)
 * \brief Get/set the X coordinate logical unit / scene unit ratio.
 */

BASIC_D_READER_IMPL(CartesianCoordinateSystem, qreal, scaleX, scaleX);
void CartesianCoordinateSystem::setScaleX(qreal scale) {
	if (scale != 0.0) // no proper fp comparison needed, just avoid division by zero in mapping functions
		exec(new CartesianCoordinateSystemSetScaleXCmd(d, scale, tr("%1: set X scale")));
}

class CartesianCoordinateSystemSetScaleYCmd: public StandardBasicSetterCmd<CartesianCoordinateSystem::Private, qreal> {
	public:
		CartesianCoordinateSystemSetScaleYCmd(CartesianCoordinateSystem::Private *target, qreal newValue, const QString &description)
			: StandardBasicSetterCmd<CartesianCoordinateSystem::Private, qreal>(target, newValue, description) {}
		virtual void *targetFieldAddress() { return &(m_target->scaleY); }
};

/**
 * \fn CartesianCoordinateSystem::BASIC_D_ACCESSOR_DECL(qreal, scaleY, ScaleY)
 * \brief Get/set the Y coordinate logical unit / scene unit ratio.
 */

BASIC_D_READER_IMPL(CartesianCoordinateSystem, qreal, scaleY, scaleY);
void CartesianCoordinateSystem::setScaleY(qreal scale) {
	if (scale != 0.0) // no proper fp comparison needed, just avoid division by zero in mapping functions
		exec(new CartesianCoordinateSystemSetScaleYCmd(d, scale, tr("%1: set Y scale")));
}


