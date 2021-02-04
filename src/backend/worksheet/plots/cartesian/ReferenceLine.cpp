/***************************************************************************
    File                 : ReferenceLine.cpp
    Project              : LabPlot
    Description          : Custom user-defined point on the plot
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "ReferenceLine.h"
#include "ReferenceLinePrivate.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "kdefrontend/GuiTools.h"

#include <QActionGroup>
#include <QPainter>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class ReferenceLine
 * \brief A customizable point.
 *
 * The position can be either specified by mouse events or by providing the
 * x- and y- coordinates in parent's coordinate system
 */

ReferenceLine::ReferenceLine(CartesianPlot* plot, const QString& name)
	: WorksheetElement(name, AspectType::ReferenceLine), d_ptr(new ReferenceLinePrivate(this)) {

	m_plot = plot;
	init();
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
ReferenceLine::~ReferenceLine() = default;

void ReferenceLine::init() {
	Q_D(ReferenceLine);

	KConfig config;
	KConfigGroup group = config.group("ReferenceLine");

	d->orientation = (Orientation)group.readEntry("Orientation", static_cast<int>(Orientation::Vertical));
	d->position = group.readEntry("Position", m_plot->xRange().center());

	d->pen.setStyle( (Qt::PenStyle) group.readEntry("Style", (int)Qt::SolidLine) );
	d->pen.setColor( group.readEntry("Color", QColor(Qt::black)) );
	d->pen.setWidthF( group.readEntry("Width", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)) );
	d->opacity = group.readEntry("Opacity", 1.0);
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon ReferenceLine::icon() const {
	return  QIcon::fromTheme(QLatin1String("draw-line"));
}

void ReferenceLine::initActions() {
	visibilityAction = new QAction(i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &ReferenceLine::visibilityChangedSlot);

	//Orientation
	auto* orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, &QActionGroup::triggered, this, &ReferenceLine::orientationChangedSlot);

	orientationHorizontalAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-axis-horizontal")), i18n("Horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);

	orientationVerticalAction = new QAction(QIcon::fromTheme(QLatin1String("labplot-axis-vertical")), i18n("Vertical"), orientationActionGroup);
	orientationVerticalAction->setCheckable(true);

	//Line
	lineStyleActionGroup = new QActionGroup(this);
	lineStyleActionGroup->setExclusive(true);
	connect(lineStyleActionGroup, &QActionGroup::triggered, this, &ReferenceLine::lineStyleChanged);

	lineColorActionGroup = new QActionGroup(this);
	lineColorActionGroup->setExclusive(true);
	connect(lineColorActionGroup, &QActionGroup::triggered, this, &ReferenceLine::lineColorChanged);
}

void ReferenceLine::initMenus() {
	this->initActions();

	//Orientation
	orientationMenu = new QMenu(i18n("Orientation"));
	orientationMenu->setIcon(QIcon::fromTheme(QLatin1String("labplot-axis-horizontal")));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);

	//Line
	lineMenu = new QMenu(i18n("Line"));
	lineMenu->setIcon(QIcon::fromTheme(QLatin1String("draw-line")));
	lineStyleMenu = new QMenu(i18n("Style"), lineMenu);
	lineStyleMenu->setIcon(QIcon::fromTheme(QLatin1String("object-stroke-style")));
	lineMenu->setIcon(QIcon::fromTheme(QLatin1String("draw-line")));
	lineMenu->addMenu(lineStyleMenu);

	lineColorMenu = new QMenu(i18n("Color"), lineMenu);
	lineColorMenu->setIcon(QIcon::fromTheme(QLatin1String("fill-color")));
	GuiTools::fillColorMenu(lineColorMenu, lineColorActionGroup);
	lineMenu->addMenu(lineColorMenu);
}

QMenu* ReferenceLine::createContextMenu() {
	if (!orientationMenu)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	Q_D(const ReferenceLine);

	//Orientation
	if (d->orientation == Orientation::Horizontal)
		orientationHorizontalAction->setChecked(true);
	else
		orientationVerticalAction->setChecked(true);
	menu->insertMenu(firstAction, orientationMenu);

	//Line styles
	GuiTools::updatePenStyles(lineStyleMenu, lineStyleActionGroup, d->pen.color());
	GuiTools::selectPenStyleAction(lineStyleActionGroup, d->pen.style());
	GuiTools::selectColorAction(lineColorActionGroup, d->pen.color() );

	menu->insertMenu(firstAction, lineMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

QGraphicsItem* ReferenceLine::graphicsItem() const {
	return d_ptr;
}

void ReferenceLine::retransform() {
	Q_D(ReferenceLine);
	d->retransform();
}

void ReferenceLine::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	Q_UNUSED(horizontalRatio)
	Q_UNUSED(verticalRatio)
	Q_UNUSED(pageResize)
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(ReferenceLine, ReferenceLine::Orientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(ReferenceLine, double, position, position)
BASIC_SHARED_D_READER_IMPL(ReferenceLine, QPen, pen, pen)
BASIC_SHARED_D_READER_IMPL(ReferenceLine, qreal, opacity, opacity)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(ReferenceLine, SetOrientation, ReferenceLine::Orientation, orientation, retransform)
void ReferenceLine::setOrientation(Orientation orientation) {
	Q_D(ReferenceLine);
	if (orientation != d->orientation)
		exec(new ReferenceLineSetOrientationCmd(d, orientation, ki18n("%1: set orientation")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceLine, SetPosition, double, position, retransform)
void ReferenceLine::setPosition(double position) {
	Q_D(ReferenceLine);
	if (position != d->position)
		exec(new ReferenceLineSetPositionCmd(d, position, ki18n("%1: set position")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceLine, SetPen, QPen, pen, recalcShapeAndBoundingRect)
void ReferenceLine::setPen(const QPen& pen) {
	Q_D(ReferenceLine);
	if (pen != d->pen)
		exec(new ReferenceLineSetPenCmd(d, pen, ki18n("%1: set line style")));
}

STD_SETTER_CMD_IMPL_F_S(ReferenceLine, SetOpacity, qreal, opacity, update);
void ReferenceLine::setOpacity(qreal opacity) {
	Q_D(ReferenceLine);
	if (opacity != d->opacity)
		exec(new ReferenceLineSetOpacityCmd(d, opacity, ki18n("%1: set line opacity")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL_F(ReferenceLine, SetVisible, bool, swapVisible, update);
void ReferenceLine::setVisible(bool on) {
	Q_D(ReferenceLine);
	exec(new ReferenceLineSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool ReferenceLine::isVisible() const {
	Q_D(const ReferenceLine);
	return d->isVisible();
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void ReferenceLine::orientationChangedSlot(QAction* action) {
	if (action == orientationHorizontalAction)
		this->setOrientation(Orientation::Horizontal);
	else
		this->setOrientation(Orientation::Vertical);
}

void ReferenceLine::lineStyleChanged(QAction* action) {
	Q_D(const ReferenceLine);
	QPen pen = d->pen;
	pen.setStyle(GuiTools::penStyleFromAction(lineStyleActionGroup, action));
	this->setPen(pen);
}

void ReferenceLine::lineColorChanged(QAction* action) {
	Q_D(const ReferenceLine);
	QPen pen = d->pen;
	pen.setColor(GuiTools::colorFromAction(lineColorActionGroup, action));
	this->setPen(pen);
}

void ReferenceLine::visibilityChangedSlot() {
	Q_D(const ReferenceLine);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
ReferenceLinePrivate::ReferenceLinePrivate(ReferenceLine* owner) :	q(owner) {
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(true);
}

QString ReferenceLinePrivate::name() const {
	return q->name();
}

/*!
    calculates the position and the bounding box of the item/point. Called on geometry or properties changes.
 */
void ReferenceLinePrivate::retransform() {
	if (suppressRetransform || ! q->cSystem)
		return;

	const auto xRange{ q->m_plot->xRange(q->cSystem->xIndex()) };
	const auto yRange{ q->m_plot->yRange(q->cSystem->yIndex()) };

	//calculate the position in the scene coordinates
	QVector<QPointF> listLogical;
	if (orientation == ReferenceLine::Orientation::Vertical)
		listLogical << QPointF(position, yRange.center());
	else
		listLogical << QPointF(xRange.center(), position);

	QVector<QPointF> listScene = q->cSystem->mapLogicalToScene(listLogical);

	if (!listScene.isEmpty()) {
		positionScene = listScene.at(0);
		m_visible = true;
		suppressItemChangeEvent = true;
		setPos(positionScene);
		suppressItemChangeEvent = false;

		//determine the length of the line to be drawn
		QVector<QPointF> pointsLogical;
		if (orientation == ReferenceLine::Orientation::Vertical)
			pointsLogical << QPointF(position, yRange.start()) << QPointF(position, yRange.end());
		else
			pointsLogical << QPointF(xRange.start(), position) << QPointF(xRange.end(), position);

		QVector<QPointF> pointsScene = q->cSystem->mapLogicalToScene(pointsLogical);

		if (pointsScene.size() > 1) {
			if (orientation == ReferenceLine::Orientation::Vertical)
				length = pointsScene.at(0).y() - pointsScene.at(1).y();
			else
				length = pointsScene.at(0).x() - pointsScene.at(1).x();
		}
	} else
		m_visible = false;

	recalcShapeAndBoundingRect();
}

bool ReferenceLinePrivate::swapVisible(bool on) {
	bool oldValue = isVisible();

	//When making a graphics item invisible, it gets deselected in the scene.
	//In this case we don't want to deselect the item in the project explorer.
	//We need to supress the deselection in the view.
	auto* worksheet = static_cast<Worksheet*>(q->parent(AspectType::Worksheet));
	worksheet->suppressSelectionChangedEvent(true);
	setVisible(on);
	worksheet->suppressSelectionChangedEvent(false);

// 	emit q->changed();
	emit q->visibleChanged(on);
	return oldValue;
}

/*!
    Returns the outer bounds of the item as a rectangle.
 */
QRectF ReferenceLinePrivate::boundingRect() const {
	return boundingRectangle;
}

/*!
    Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath ReferenceLinePrivate::shape() const {
	return lineShape;
}

/*!
  recalculates the outer bounds and the shape of the item.
*/
void ReferenceLinePrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	lineShape = QPainterPath();
	if (m_visible) {
		QPainterPath path;
		if (orientation == ReferenceLine::Orientation::Horizontal) {
			path.moveTo(-length/2, 0);
			path.lineTo(length/2, 0);
		} else {
			path.moveTo(0, length/2);
			path.lineTo(0, -length/2);
		}
		lineShape.addPath(WorksheetElement::shapeFromPath(path, pen));
		boundingRectangle = lineShape.boundingRect();
	}
}

void ReferenceLinePrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (!m_visible)
		return;

	painter->setOpacity(opacity);
	painter->setPen(pen);
	if (orientation == ReferenceLine::Orientation::Horizontal)
		painter->drawLine(-length/2, 0, length/2, 0);
	else
		painter->drawLine(0, length/2, 0, -length/2);

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(lineShape);
	}

	if (isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(lineShape);
	}
}

QVariant ReferenceLinePrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (suppressItemChangeEvent)
		return value;

	if (change != QGraphicsItem::ItemPositionChange)
		return QGraphicsItem::itemChange(change, value);

	QPointF positionSceneNew = value.toPointF();

	//don't allow to move the line outside of the plot rect
	//in the direction orthogonal to the orientation of the line
	if (orientation == ReferenceLine::Orientation::Horizontal) {
		if (positionSceneNew.x() != positionScene.x())
			positionSceneNew.setX(positionScene.x());
	} else {
		if (positionSceneNew.y() != positionScene.y())
			positionSceneNew.setY(positionScene.y());
	}

	if (q->m_plot->dataRect().contains(positionSceneNew)) {
		//emit the signals in order to notify the UI (dock widget and status bar) about the new logical position.
		//we don't set the position related member variables during the mouse movements.
		//this is done on mouse release events only.
		//TODO
		const auto* cSystem{ q->m_plot->defaultCoordinateSystem() };
		QPointF positionLogical = cSystem->mapSceneToLogical(positionSceneNew);
		if (orientation == ReferenceLine::Orientation::Horizontal) {
			emit q->positionChanged(positionLogical.y());
			emit q->statusInfo(QLatin1String("y=") + QString::number(positionLogical.y()));
		} else {
			emit q->positionChanged(positionLogical.x());
			emit q->statusInfo(QLatin1String("x=") + QString::number(positionLogical.x()));
		}
	} else {
		//line is moved outside of the plot, keep it at the plot boundary
		if (orientation == ReferenceLine::Orientation::Horizontal) {
			if (positionSceneNew.y() < q->m_plot->dataRect().y())
				positionSceneNew.setY(q->m_plot->dataRect().y());
			else
				positionSceneNew.setY(q->m_plot->dataRect().y() + q->m_plot->dataRect().height());
		} else {
			if (positionSceneNew.x() < q->m_plot->dataRect().x())
				positionSceneNew.setX(q->m_plot->dataRect().x());
			else
				positionSceneNew.setX(q->m_plot->dataRect().x() + q->m_plot->dataRect().width());
		}
	}

	return QGraphicsItem::itemChange(change, QVariant(positionSceneNew));
}

void ReferenceLinePrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	//position was changed -> set the position member variables
	suppressRetransform = true;
	QPointF positionLogical = q->cSystem->mapSceneToLogical(pos());
	if (orientation == ReferenceLine::Orientation::Horizontal)
		q->setPosition(positionLogical.y());
	else
		q->setPosition(positionLogical.x());
	suppressRetransform = false;

	QGraphicsItem::mouseReleaseEvent(event);
}

void ReferenceLinePrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void ReferenceLinePrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		emit q->hovered();
		update();
	}
}

void ReferenceLinePrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		emit q->unhovered();
		update();
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void ReferenceLine::save(QXmlStreamWriter* writer) const {
	Q_D(const ReferenceLine);

	writer->writeStartElement("referenceLine");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement("general");
	writer->writeAttribute("orientation", QString::number(static_cast<int>(d->orientation)));
	writer->writeAttribute("position", QString::number(d->position));
	writer->writeAttribute( "plotRangeIndex", QString::number(m_cSystemIndex) );
	writer->writeAttribute("visible", QString::number(d->isVisible()));
	writer->writeEndElement();

	writer->writeStartElement("line");
	WRITE_QPEN(d->pen);
	writer->writeAttribute("opacity", QString::number(d->opacity));
	writer->writeEndElement();

	writer->writeEndElement(); // close "ReferenceLine" section
}

//! Load from XML
bool ReferenceLine::load(XmlStreamReader* reader, bool preview) {
	Q_D(ReferenceLine);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "referenceLine")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (!preview && reader->name() == "general") {
			attribs = reader->attributes();
			READ_DOUBLE_VALUE("position", position);
			READ_INT_VALUE("orientation", orientation, Orientation);

			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("visible").toString());
			else
				d->setVisible(str.toInt());
		} else if (!preview && reader->name() == "line") {
			attribs = reader->attributes();
			READ_QPEN(d->pen);
			READ_DOUBLE_VALUE("opacity", opacity);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	if (!preview)
		retransform();

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void ReferenceLine::loadThemeConfig(const KConfig& config) {
	//for the properties of the line read the properties of the axis line
	const KConfigGroup& group = config.group("Axis");
	QPen p;
	this->setOpacity(group.readEntry("LineOpacity", 1.0));
	p.setStyle((Qt::PenStyle)group.readEntry("LineStyle", (int)Qt::SolidLine));
	p.setColor(group.readEntry("LineColor", QColor(Qt::black)));
	p.setWidthF(group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	this->setPen(p);
}
