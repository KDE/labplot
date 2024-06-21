/*
	File                 : CustomPoint.cpp
	Project              : LabPlot
	Description          : Custom user-defined point on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2021 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CustomPoint.h"
#include "CustomPointPrivate.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

using Dimension = CartesianCoordinateSystem::Dimension;

namespace {
namespace XML {
constexpr auto name = "customPoint";
}
}

/**
 * \class CustomPoint
 * \brief A customizable point.
 *
 * The position can be either specified by mouse events or by providing the
 * x- and y- coordinates in parent's coordinate system
 */

CustomPoint::CustomPoint(CartesianPlot* plot, const QString& name, bool loading)
	: WorksheetElement(name, new CustomPointPrivate(this), AspectType::CustomPoint) {
	Q_D(CustomPoint);
	d->m_plot = plot;

	init(loading);
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
CustomPoint::~CustomPoint() = default;

void CustomPoint::init(bool loading) {
	Q_D(CustomPoint);

	// create the symbol
	d->symbol = new Symbol(QString());
	addChild(d->symbol);
	d->symbol->setHidden(true);
	connect(d->symbol, &Symbol::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});
	connect(d->symbol, &Symbol::updatePixmapRequested, [=] {
		d->update();
		Q_EMIT changed();
	});

	// init the properties
	if (!loading) {
		KConfig config;
		d->symbol->init(config.group(QStringLiteral("CustomPoint")));

		// default position
		if (plot()) {
			d->coordinateBindingEnabled = true; // By default on
			auto cs = plot()->coordinateSystem(plot()->defaultCoordinateSystemIndex());
			const auto x = d->m_plot->range(Dimension::X, cs->index(Dimension::X)).center();
			const auto y = d->m_plot->range(Dimension::Y, cs->index(Dimension::Y)).center();
			DEBUG(Q_FUNC_INFO << ", x/y pos = " << x << " / " << y)
			d->positionLogical = QPointF(x, y);
		} else
			d->position.point = QPointF(0, 0);
		d->updatePosition(); // To update also scene coordinates
	}
}

void CustomPoint::initActions() {
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon CustomPoint::icon() const {
	return QIcon::fromTheme(QStringLiteral("draw-cross"));
}

QMenu* CustomPoint::createContextMenu() {
	// no context menu if the custom point is a child of an InfoElement,
	// everything is controlled by the parent
	if (parentAspect()->type() == AspectType::InfoElement)
		return nullptr;

	return WorksheetElement::createContextMenu();
	;
}

void CustomPoint::retransform() {
	DEBUG(Q_FUNC_INFO)
	Q_D(CustomPoint);
	d->retransform();
}

void CustomPoint::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
}

QString CustomPoint::xmlName() {
	return QLatin1String(XML::name);
}

Symbol* CustomPoint::symbol() const {
	Q_D(const CustomPoint);
	return d->symbol;
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
CustomPointPrivate::CustomPointPrivate(CustomPoint* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);
}

const CartesianPlot* CustomPointPrivate::plot() {
	return m_plot;
}

/*!
	calculates the position and the bounding box of the item/point. Called on geometry or properties changes.
 */
void CustomPointPrivate::retransform() {
	DEBUG(Q_FUNC_INFO)
	if (suppressRetransform || q->isLoading())
		return;

	updatePosition(); // needed, because CartesianPlot calls retransform if some operations are done
	recalcShapeAndBoundingRect();
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void CustomPointPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	m_shape = QPainterPath();
	if (insidePlot && symbol->style() != Symbol::Style::NoSymbols) {
		QPainterPath path = Symbol::stylePath(symbol->style());

		QTransform trafo;
		trafo.scale(symbol->size(), symbol->size());
		path = trafo.map(path);
		trafo.reset();

		if (symbol->rotationAngle() != 0.) {
			trafo.rotate(symbol->rotationAngle());
			path = trafo.map(path);
		}

		m_shape.addPath(WorksheetElement::shapeFromPath(trafo.map(path), symbol->pen()));
		m_boundingRectangle = m_shape.boundingRect();
	}

	Q_EMIT q->changed();
}

void CustomPointPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!insidePlot)
		return;

	if (symbol->style() != Symbol::Style::NoSymbols) {
		painter->setOpacity(symbol->opacity());
		painter->setPen(symbol->pen());
		painter->setBrush(symbol->brush());
		painter->drawPath(m_shape);
	}

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(m_shape);
	}

	if (isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(m_shape);
	}
}

void CustomPointPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	// don't move when the parent is a InfoElement, because there
	// the custompoint position changes by the mouse are not allowed
	if (q->parentAspect()->type() == AspectType::InfoElement)
		return;

	WorksheetElementPrivate::mouseReleaseEvent(event);
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void CustomPoint::save(QXmlStreamWriter* writer) const {
	Q_D(const CustomPoint);

	writer->writeStartElement(QStringLiteral("customPoint"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// geometry
	writer->writeStartElement(QStringLiteral("geometry"));
	WorksheetElement::save(writer);
	writer->writeEndElement();

	d->symbol->save(writer);

	writer->writeEndElement(); // close "CustomPoint" section
}

//! Load from XML
bool CustomPoint::load(XmlStreamReader* reader, bool preview) {
	Q_D(CustomPoint);

	if (!readBasicAttributes(reader))
		return false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String(XML::name))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("geometry")) {
			WorksheetElement::load(reader, preview);
			if (project()->xmlVersion() < 6) {
				// Before version 6 the position in the file was always a logical position
				d->positionLogical = d->position.point;
				d->position.point = QPointF(0, 0);
				d->coordinateBindingEnabled = true;
			}
		} else if (!preview && reader->name() == QLatin1String("symbol")) {
			d->symbol->load(reader, preview);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}
	return true;
}
