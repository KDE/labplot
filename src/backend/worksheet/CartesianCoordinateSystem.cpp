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
		CartesianCoordinateSystemPrivate(CartesianCoordinateSystem *owner) : q(owner) {
		}

		~CartesianCoordinateSystemPrivate() {
		}

		QString name() const {
			return q->name();
		}

		QPointF position; //!< scene position of the origin
		qreal scaleX; //!< X ratio logical units / scene units
		qreal scaleY; //!< Y ratio logical units / scene units

		CartesianCoordinateSystem * const q;
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

QPointF CartesianCoordinateSystem::mapLogicalToScene(const QPointF &point) const {
	Q_D(const CartesianCoordinateSystem);

	QPointF result = point;
	result.setX(result.x() / d->scaleX + d->position.x());
	result.setY(result.y() / d->scaleY + d->position.y());
	return result;
}

QPointF CartesianCoordinateSystem::mapSceneToLogical(const QPointF &point) const {
	Q_D(const CartesianCoordinateSystem);

	QPointF result = point;
	result.setX((result.x() - d->position.x()) * d->scaleX);
	result.setY((result.y() - d->position.y()) * d->scaleY);
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

class TestItemGroup : public QGraphicsItemGroup {

	public:
		QRectF rect;

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                 QWidget *widget) { }

		virtual QRectF boundingRect () const { return rect; }
		virtual QPainterPath shape () const { QPainterPath path; path.addRect(rect); return path; }
};

QGraphicsItem *CartesianCoordinateSystem::graphicsItem() const {
	return d_ptr;
#if 0
//	QGraphicsRectItem *clipRect = new QGraphicsRectItem(QRectF(mapLogicalToScene(QPointF(0, 0)), mapLogicalToScene(QPointF(3, 3))));
//	qDebug() << clipRect->rect();
	TestItemGroup *clipRect = new TestItemGroup();
	clipRect->rect = QRectF(mapLogicalToScene(QPointF(0, 0)), mapLogicalToScene(QPointF(6, 6)));

	QList<QGraphicsItem *> itemList;
	QList<AbstractWorksheetElement *> childList = children<AbstractWorksheetElement>(AbstractAspect::IncludeHidden | AbstractAspect::Compress);
	foreach(const AbstractWorksheetElement *elem, childList)
	{
		itemList << elem->graphicsItems();
	}

	foreach(QGraphicsItem *item, itemList)
	{
		clipRect->addToGroup(item);
//		item->setParentItem(clipRect);
	}
	clipRect->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
//	clipRect->setPen(QPen(Qt::green));

	QGraphicsRectItem *rect2 = new QGraphicsRectItem(QRectF(mapLogicalToScene(QPointF(1, 1)), mapLogicalToScene(QPointF(4, 4))));
//	QGraphicsItemGroup *gr1 = new QGraphicsItemGroup();
//	gr1->addToGroup(rect2);
//	gr1->setParentItem(clipRect);
	clipRect->addToGroup(rect2);

//	return itemList;
	return QList<QGraphicsItem *>() << clipRect;
#endif
}

