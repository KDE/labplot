/*
	File                 : Segment.cpp
	Project              : LabPlot
	Description          : Graphics-item for curve of Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Segment.h"
#include "SegmentPrivate.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/DatapickerImage.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/worksheet/Worksheet.h"

//#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QPainter>

#include <KLocalizedString>

/**
 * \class Segment
 * \brief graphics-item class for curve-segment
 */

Segment::Segment(DatapickerImage* image)
	: m_image(image)
	, d_ptr(new SegmentPrivate(this)) {
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

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
SegmentPrivate::SegmentPrivate(Segment* owner)
	: scaleFactor(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch) / QApplication::desktop()->physicalDpiX())
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setAcceptHoverEvents(true);
	setVisible(false);

	pen = QPen(Qt::green, 3, Qt::SolidLine);
}

/*!
	calculates the position and the bounding box of the item. Called on geometry or properties changes.
 */
void SegmentPrivate::retransform() {
	QMatrix matrix;
	matrix.scale(scaleFactor, scaleFactor);
	for (auto* line : q->path) {
		const QLine& scaledLine = matrix.map(*line);
		linePath.moveTo(scaledLine.p1());
		linePath.lineTo(scaledLine.p2());
	}
	recalcShapeAndBoundingRect();
}

/*!
	Returns the outer bounds of the item as a rectangle.
 */
QRectF SegmentPrivate::boundingRect() const {
	return boundingRectangle;
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
	boundingRectangle = linePath.boundingRect();
	itemShape = QPainterPath();
	itemShape.addRect(boundingRectangle);
}

void SegmentPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	painter->setPen(pen);
	painter->drawPath(linePath);

	if (m_hovered && !isSelected()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(linePath);
	}

	// 	if (isSelected()) {
	// 		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
	// 		painter->setOpacity(selectedOpacity);
	// 		painter->drawPath(itemShape);
	// 	}
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

QVariant SegmentPrivate::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
	if (change == QGraphicsItem::ItemSelectedChange && value == true) {
		auto* datapicker = static_cast<Datapicker*>(q->m_image->parentAspect());
		if (datapicker->activeCurve()) {
			int count = 0;
			QList<QPointF> posList;
			posList.clear();
			for (QLine* line : q->path) {
				const int l = (line->y1() > line->y2()) ? line->y2() : line->y1();
				const int h = (line->y1() > line->y2()) ? line->y1() : line->y2();

				for (int i = l; i <= h; ++i) {
					if (count % q->m_image->pointSeparation() == 0) {
						bool positionUsed = false;
						const auto points = datapicker->activeCurve()->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
						for (const auto* point : points) {
							if (point->position() == QPoint(line->x1(), i) * scaleFactor)
								positionUsed = true;
						}

						if (!positionUsed)
							posList << QPoint(line->x1(), i) * scaleFactor;
					}
					count++;
				}
			}

			if (!posList.isEmpty()) {
				auto* curve = datapicker->activeCurve();
				curve->beginMacro(i18n("%1: draw points over segment", datapicker->activeCurve()->name()));
				curve->suppressUpdatePoint(true);
				for (const QPointF& pos : posList)
					datapicker->addNewPoint(pos, curve);
				curve->suppressUpdatePoint(false);
				curve->endMacro();
			}
		}

		// no need to keep segment selected
		return false;
	}

	return QGraphicsItem::itemChange(change, value);
}
