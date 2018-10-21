/***************************************************************************
    File                 : Segment.cpp
    Project              : LabPlot
    Description          : Graphics-item for curve of Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
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

#include "Segment.h"
#include "SegmentPrivate.h"
#include "backend/datapicker/DatapickerImage.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/datapicker/Datapicker.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QDesktopWidget>

#include <KLocalizedString>

/**
 * \class Segment
 * \brief graphics-item class for curve-segment
 */

Segment::Segment(DatapickerImage* image):
	yLast(0), length(0), m_image(image), d_ptr(new SegmentPrivate(this)) {
	m_image->scene()->addItem(this->graphicsItem());
}

QGraphicsItem* Segment::graphicsItem() const {
	return d_ptr;
}

void Segment::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(Segment);
	d->setParentItem(item);
}

void Segment::retransform() {
	Q_D(Segment);
	d->retransform();
}

bool Segment::isVisible() const {
	Q_D(const Segment);
	return d->isVisible();
}

void Segment::setVisible(bool on) {
	Q_D(Segment);
	d->setVisible(on);
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
SegmentPrivate::SegmentPrivate(Segment *owner) :
	scaleFactor(Worksheet::convertToSceneUnits(1, Worksheet::Inch)/QApplication::desktop()->physicalDpiX()),
	m_hovered(false),
	q(owner) {

	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setAcceptHoverEvents(true);
	setVisible(false);
}

/*!
    calculates the position and the bounding box of the item. Called on geometry or properties changes.
 */
void SegmentPrivate::retransform() {
	for (auto* line : q->path) {
		linePath.moveTo(line->p1());
		linePath.lineTo(line->p2());
	}
	boundingRectangle = linePath.boundingRect();
	recalcShapeAndBoundingRect();
}

/*!
    Returns the outer bounds of the item as a rectangle.
 */
QRectF SegmentPrivate::boundingRect() const {
	return transformedBoundingRectangle;
}

/*!
    Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath SegmentPrivate::shape() const {
	return itemShape;
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void SegmentPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();
	QMatrix matrix;
	matrix.scale(scaleFactor, scaleFactor);
	transformedBoundingRectangle = matrix.mapRect(boundingRectangle);
	itemShape = QPainterPath();
	itemShape.addRect(transformedBoundingRectangle);
}

void SegmentPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	QPen pen(Qt::green);
	QPen hoveredPen = QPen(QColor(128,179,255), 3, Qt::SolidLine);
	qreal hoveredOpacity = 0.6;
	QPen selectedPen = QPen(Qt::darkBlue, 3, Qt::SolidLine);
	qreal selectedOpacity = 0.3;

	painter->save();
	painter->setPen(pen);
	painter->scale(scaleFactor, scaleFactor);
	painter->drawPath(linePath);
	painter->restore();

	if (m_hovered && !isSelected()) {
		painter->setPen(hoveredPen);
		painter->setOpacity(hoveredOpacity);
		painter->drawPath(itemShape);
	}

	if (isSelected()) {
		painter->setPen(selectedPen);
		painter->setOpacity(selectedOpacity);
		painter->drawPath(itemShape);
	}
}

void SegmentPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		update();
	}
}

void SegmentPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		update();
	}
}

QVariant SegmentPrivate::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
	if ( change == QGraphicsItem::ItemSelectedChange && value == true ) {
		auto* datapicker = dynamic_cast<Datapicker*>(q->m_image->parentAspect());
		Q_ASSERT(datapicker);
		if (datapicker->activeCurve()) {
			int count = 0;
			QList<QPointF> posList;
			posList.clear();
			for (QLine* line : q->path) {
				const int l = (line->y1() > line->y2())?line->y2():line->y1();
				const int h = (line->y1() > line->y2())?line->y1():line->y2();

				for (int i = l; i <= h; ++i) {
					if (count%q->m_image->pointSeparation() == 0) {
						bool positionUsed = false;
						const QVector<DatapickerPoint*> curvePointsList = datapicker->activeCurve()->children<DatapickerPoint>(AbstractAspect::IncludeHidden);
						for (const auto* point : curvePointsList) {
							if ( point->position() == QPoint(line->x1(), i)*scaleFactor )
								positionUsed = true;
						}

						if (!positionUsed)
							posList<<QPoint(line->x1(), i)*scaleFactor;
					}
					count++;
				}
			}

			if (!posList.isEmpty()) {
				datapicker->activeCurve()->beginMacro(i18n("%1: draw points over segment", datapicker->activeCurve()->name()));
				for (const QPointF& pos : posList)
					datapicker->addNewPoint(pos, datapicker->activeCurve());
				datapicker->activeCurve()->endMacro();
			}
		}

		//no need to keep segment selected
		return false;
	}

	return QGraphicsItem::itemChange(change, value);
}
