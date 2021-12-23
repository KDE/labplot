/*
    File                 : WorksheetElement.cpp
    Project              : LabPlot
    Description          : Base class for all Worksheet children.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2012-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/WorksheetElementPrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/core/Project.h"
#include "plots/PlotArea.h"
#include "plots/AbstractPlot.h"
#include "plots/cartesian/CartesianCoordinateSystem.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMenu>
#include <QPen>
#include <QKeyEvent>
#include <QXmlStreamReader>
#include <QStringLiteral>
#include <KLocalizedString>


/**
 * \class WorksheetElement
 * \brief Base class for all Worksheet children.
 *
 */
//WorksheetElement::WorksheetElement(const QString& name, AspectType type)
//: AbstractAspect(name, type), d_ptr(new WorksheetElementPrivate(this)) {
//	init();
//}


WorksheetElement::WorksheetElement(const QString& name, WorksheetElementPrivate* dd, AspectType type)
	: AbstractAspect(name, type), d_ptr(dd){
	init();
}

void WorksheetElement::init() {

	d_ptr->setData(0, static_cast<quint64>(type()));
	m_drawingOrderMenu = new QMenu(i18n("Drawing &order"));
	m_drawingOrderMenu->setIcon(QIcon::fromTheme("layer-bottom"));
	m_moveBehindMenu = new QMenu(i18n("Move &behind"));
	m_moveBehindMenu->setIcon(QIcon::fromTheme("draw-arrow-down"));
	m_moveInFrontOfMenu = new QMenu(i18n("Move in &front of"));
	m_moveInFrontOfMenu->setIcon(QIcon::fromTheme("draw-arrow-up"));
	m_drawingOrderMenu->addMenu(m_moveBehindMenu);
	m_drawingOrderMenu->addMenu(m_moveInFrontOfMenu);

	connect(m_drawingOrderMenu, &QMenu::aboutToShow, this, &WorksheetElement::prepareDrawingOrderMenu);
	connect(m_moveBehindMenu, &QMenu::triggered, this, &WorksheetElement::execMoveBehind);
	connect(m_moveInFrontOfMenu, &QMenu::triggered, this, &WorksheetElement::execMoveInFrontOf);
}

WorksheetElement::~WorksheetElement() {
	delete m_moveBehindMenu;
	delete m_moveInFrontOfMenu;
	delete m_drawingOrderMenu;
}

void WorksheetElement::finalizeAdd() {
	DEBUG(Q_FUNC_INFO)
	if (!m_plot) {
		/*Not in every case the parentAspect is a cartesian plot. When creating an infoelement, the parent
		 * of a custom point is not the CartesianPlot (and so this function returns a nullptr), but the InfoElement.
		 * So the plot is set manally in the custompoint and therefore the plot should not be set anymore.
		*/
		m_plot = dynamic_cast<CartesianPlot*>(parentAspect());
	}

	if (m_plot) {
		cSystem = dynamic_cast<const CartesianCoordinateSystem*>(m_plot->coordinateSystem(m_cSystemIndex));
		Q_EMIT plotRangeListChanged();
	} else
		DEBUG(Q_FUNC_INFO << ", WARNING: no plot available.")
}

/**
 * \fn QGraphicsItem *WorksheetElement::graphicsItem() const
 * \brief Return the graphics item representing this element.
 *
 */

/**
 * \fn void WorksheetElement::setVisible(bool on)
 * \brief Show/hide the element.
 *
 */

/**
 * \fn bool WorksheetElement::isVisible() const
 * \brief Return whether the element is (at least) partially visible.
 *
 */

/**
 * \brief Return whether the element is fully visible (i.e., including all child elements).
 *
 * The standard implementation returns isVisible().
 */
bool WorksheetElement::isFullyVisible() const {
	return isVisible();
}

/**
 * \fn void WorksheetElement::setPrinting(bool on)
 * \brief Switches the printing mode on/off
 *
 */
void WorksheetElement::setPrinting(bool printing) {
	m_printing = printing;
}

bool WorksheetElement::isPrinting() const {
	return m_printing;
}

void WorksheetElement::setZValue(qreal value) {
	graphicsItem()->setZValue(value);
}

STD_SWAP_METHOD_SETTER_CMD_IMPL_F(WorksheetElement, SetVisible, bool, swapVisible, update)
void WorksheetElement::setVisible(bool on) {
	Q_D(WorksheetElement);
	exec(new WorksheetElementSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool WorksheetElementPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();

	//When making a graphics item invisible, it gets deselected in the scene.
	//In this case we don't want to deselect the item in the project explorer.
	//We need to supress the deselection in the view.
	auto* worksheet = static_cast<Worksheet*>(q->parent(AspectType::Worksheet));
	if (worksheet) {
		worksheet->suppressSelectionChangedEvent(true);
		setVisible(on);
		worksheet->suppressSelectionChangedEvent(false);
	} else
		setVisible(on);

	Q_EMIT q->changed();
	Q_EMIT q->visibleChanged(on);
	return oldValue;
}

bool WorksheetElement::isVisible() const {
	Q_D(const WorksheetElement);
	return d->isVisible();
}

/**
    This does exactly what Qt internally does to creates a shape from a painter path.
*/
QPainterPath WorksheetElement::shapeFromPath(const QPainterPath &path, const QPen &pen) {
	if (path == QPainterPath())
		return path;

// 	PERFTRACE("WorksheetElement::shapeFromPath()");

	// TODO: We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
	// if we pass a value of 0.0 to QPainterPathStroker::setWidth()
	const qreal penWidthZero = qreal(1.e-8);

	QPainterPathStroker ps;
	ps.setCapStyle(pen.capStyle());
	if (pen.widthF() <= 0.0)
		ps.setWidth(penWidthZero);
	else
		ps.setWidth(pen.widthF());
	ps.setJoinStyle(pen.joinStyle());
	ps.setMiterLimit(pen.miterLimit());

	QPainterPath p = ps.createStroke(path);
	p.addPath(path);

	return p;
}

QMenu* WorksheetElement::createContextMenu() {
	QMenu* menu = AbstractAspect::createContextMenu();

	//add the sub-menu for the drawing order

	//don't add the drawing order menu for axes and legends, they're always drawn on top of each other elements
	if (type() == AspectType::Axis || type() == AspectType::CartesianPlotLegend)
		return menu;

	//for plots in a worksheet with an active layout the Z-factor is not relevant but we still
	//want to use the "Drawing order" menu to be able to change the position/order of the plot in the layout.
	//Since the order of the child in the list of children is opposite to the Z-factor, we change
	//the names of the menus to adapt to the reversed logic.
	if (dynamic_cast<AbstractPlot*>(this) ) {
		const Worksheet* w = dynamic_cast<const Worksheet*>(this->parentAspect());
		if (!w)
			return menu;

		if (w->layout() != Worksheet::Layout::NoLayout) {
			m_moveBehindMenu->setTitle(i18n("Move in &front of"));
			m_moveBehindMenu->setIcon(QIcon::fromTheme("draw-arrow-up"));
			m_moveInFrontOfMenu->setTitle(i18n("Move &behind"));
			m_moveInFrontOfMenu->setIcon(QIcon::fromTheme("draw-arrow-down"));
		} else {
			m_moveBehindMenu->setTitle(i18n("Move &behind"));
			m_moveBehindMenu->setIcon(QIcon::fromTheme("draw-arrow-down"));
			m_moveInFrontOfMenu->setTitle(i18n("Move in &front of"));
			m_moveInFrontOfMenu->setIcon(QIcon::fromTheme("draw-arrow-up"));
		}
	}

	//don't add the drawing order menu if the parent element has no other children
	int children = 0;
	for (auto* child : parentAspect()->children<WorksheetElement>()) {
		if (child->type() != AspectType::Axis && child->type() != AspectType::CartesianPlotLegend)
			children++;
	}

	if (children > 1) {
		menu->addSeparator();
		menu->addMenu(m_drawingOrderMenu);
	}

	return menu;
}

void WorksheetElement::prepareDrawingOrderMenu() {
	const AbstractAspect* parent = parentAspect();
	const int index = parent->indexOfChild<WorksheetElement>(this);
	const auto& children = parent->children<WorksheetElement>();

	//"move behind" sub-menu
	m_moveBehindMenu->clear();
	for (int i = 0; i < index; ++i) {
		const auto* elem = children.at(i);
		//axes and legends are always drawn on top of other elements, don't add them to the menu
		if (elem->type() != AspectType::Axis && elem->type() != AspectType::CartesianPlotLegend) {
			auto* action = m_moveBehindMenu->addAction(elem->icon(), elem->name());
			action->setData(i);
		}
	}

	//"move in front of" sub-menu
	m_moveInFrontOfMenu->clear();
	for (int i = index + 1; i < children.size(); ++i) {
		const auto* elem = children.at(i);
		//axes and legends are always drawn on top of other elements, don't add them to the menu
		if (elem->type() != AspectType::Axis && elem->type() != AspectType::CartesianPlotLegend) {
			auto* action = m_moveInFrontOfMenu->addAction(elem->icon(), elem->name());
			action->setData(i);
		}
	}

	//hide the sub-menus if they don't have any entries
	m_moveInFrontOfMenu->menuAction()->setVisible(!m_moveInFrontOfMenu->isEmpty());
	m_moveBehindMenu->menuAction()->setVisible(!m_moveBehindMenu->isEmpty());
}

void WorksheetElement::execMoveInFrontOf(QAction* action) {
	Q_EMIT moveBegin();
	AbstractAspect* parent = parentAspect();
	int index = action->data().toInt();
	AbstractAspect* sibling1 = parent->child<WorksheetElement>(index);
	AbstractAspect* sibling2 = parent->child<WorksheetElement>(index + 1);
	beginMacro(i18n("%1: move behind %2.", name(), sibling1->name()));
	setMoved(true);
	remove();
	parent->insertChildBefore(this, sibling2);
	setMoved(false);
	endMacro();
	Q_EMIT moveEnd();
}

void WorksheetElement::execMoveBehind(QAction* action) {
	Q_EMIT moveBegin();
	AbstractAspect* parent = parentAspect();
	int index = action->data().toInt();
	AbstractAspect* sibling = parent->child<WorksheetElement>(index);
	beginMacro(i18n("%1: move in front of %2.", name(), sibling->name()));
	setMoved(true);
	remove();
	parent->insertChildBefore(this, sibling);
	setMoved(false);
	endMacro();
	Q_EMIT moveEnd();
}

QPointF WorksheetElement::align(QPointF pos, QRectF rect, HorizontalAlignment horAlign, VerticalAlignment vertAlign, bool positive) const
{
	// positive is right
	double xAlign;
	switch (horAlign) {
	case WorksheetElement::HorizontalAlignment::Left:
		xAlign = rect.width()/2;
		break;
	case WorksheetElement::HorizontalAlignment::Right:
		xAlign = - rect.width()/2;
		break;
	case WorksheetElement::HorizontalAlignment::Center:
		// Fall through
	default:
		xAlign = 0;
	break;
	}

	// positive is to top
	double yAlign;
	switch (vertAlign) {
	case WorksheetElement::VerticalAlignment::Bottom:
		yAlign = - rect.height()/2;
		break;
	case WorksheetElement::VerticalAlignment::Top:
		yAlign = rect.height()/2;
		break;
	case WorksheetElement::VerticalAlignment::Center:
		// Fall through
	default:
		yAlign = 0;
	break;
	}

	// For yAlign it must be two times plus.
	if (positive)
		return {pos.x() + xAlign, pos.y() + yAlign};
	else
		return {pos.x() - xAlign, pos.y() + yAlign};
}

QRectF WorksheetElement::parentRect() const {
	QRectF rect;
	auto parent = parentAspect();
	if (parent && parent->type() == AspectType::CartesianPlot && plot()) {
		if (type() != AspectType::Axis)
			rect = plot()->graphicsItem()->mapRectFromScene(plot()->rect());
		else
			rect = plot()->dataRect(); //axes are positioned relative to the data rect and not to the whole plot rect
	} else {
		const auto* parent = graphicsItem()->parentItem();
		if (parent) {
			rect = parent->boundingRect();
		} else {
			if (graphicsItem()->scene())
				rect = graphicsItem()->scene()->sceneRect();
		}
	}

	return rect;
}

/*!
	* \brief parentPosToRelativePos
	* Converts the absolute position of the element in parent coordinates into the distance between the
	* alignement point of the parent and the element
	* \param parentPos Element position in parent coordinates
	* \param parentRect Parent data rect
	* \param rect element's rect
	* \param position contains the alignement of the element to the parent
	* \return distance between the parent position to the element
	*/
QPointF WorksheetElement::parentPosToRelativePos(QPointF parentPos, QRectF rect, PositionWrapper position,
												 HorizontalAlignment horAlign, VerticalAlignment vertAlign) const {
	QPointF relPos;
	QRectF parentRect = this->parentRect();

	if (position.horizontalPosition == HorizontalPosition::Left)
		relPos.setX(parentPos.x() - (parentRect.x()));
	else if (position.horizontalPosition == HorizontalPosition::Center || position.horizontalPosition == HorizontalPosition::Custom)
		relPos.setX(parentPos.x() - (parentRect.x() + parentRect.width()/2));
	else  //position.horizontalPosition == WorksheetElement::HorizontalPosition::Right // default
		relPos.setX(parentPos.x() - (parentRect.x() + parentRect.width()));

	if (position.verticalPosition == VerticalPosition::Center|| position.verticalPosition == VerticalPosition::Custom)
		relPos.setY(parentRect.y() + parentRect.height()/2 - parentPos.y());
	else if (position.verticalPosition == VerticalPosition::Bottom)
		relPos.setY(parentRect.y() + parentRect.height() - parentPos.y());
	else // position.verticalPosition == VerticalPosition::Top // default
		relPos.setY(parentRect.y() - parentPos.y());

	return align(relPos, rect, horAlign, vertAlign, false);
}

/*!
* \brief relativePosToParentPos
* \param parentRect
* \param rect element's rect
* \param position contains the alignement of the element to the parent
* \return parent position
*/
QPointF WorksheetElement::relativePosToParentPos(QRectF rect, PositionWrapper position,
												 HorizontalAlignment horAlign, VerticalAlignment vertAlign) const {
	QPointF parentPos;
	QRectF parentRect = this->parentRect();

	if (position.horizontalPosition == HorizontalPosition::Left)
		parentPos.setX(parentRect.x() + position.point.x());
	else if (position.horizontalPosition == HorizontalPosition::Center || position.horizontalPosition == HorizontalPosition::Custom)
		parentPos.setX(parentRect.x() + parentRect.width()/2 + position.point.x());
	else  //position.horizontalPosition == WorksheetElement::HorizontalPosition::Right // default
		parentPos.setX(parentRect.x() + parentRect.width() + position.point.x());

	if (position.verticalPosition == VerticalPosition::Center || position.verticalPosition == VerticalPosition::Custom)
		parentPos.setY(parentRect.y() + parentRect.height()/2 - position.point.y());
	else if (position.verticalPosition == VerticalPosition::Bottom)
		parentPos.setY(parentRect.y() + parentRect.height() - position.point.y());
	else // position.verticalPosition == WorksheetElement::VerticalPosition::Top // default
		parentPos.setY(parentRect.y() - position.point.y());

	return align(parentPos, rect, horAlign, vertAlign, true);
}

void WorksheetElement::save(QXmlStreamWriter* writer) const {
	Q_D(const WorksheetElement);
	writer->writeAttribute( "x", QString::number(d->position.point.x()) );
	writer->writeAttribute( "y", QString::number(d->position.point.y()) );
	writer->writeAttribute( "horizontalPosition", QString::number(static_cast<int>(d->position.horizontalPosition)) );
	writer->writeAttribute( "verticalPosition", QString::number(static_cast<int>(d->position.verticalPosition)) );
	writer->writeAttribute( "horizontalAlignment", QString::number(static_cast<int>(d->horizontalAlignment)) );
	writer->writeAttribute( "verticalAlignment", QString::number(static_cast<int>(d->verticalAlignment)) );
	writer->writeAttribute( "rotationAngle", QString::number(d->rotationAngle) );
	writer->writeAttribute( "plotRangeIndex", QString::number(m_cSystemIndex) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeAttribute( "coordinateBinding", QString::number(d->coordinateBindingEnabled));
	writer->writeAttribute( "logicalPosX", QString::number(d->positionLogical.x()));
	writer->writeAttribute( "logicalPosY", QString::number(d->positionLogical.y()));
}

bool WorksheetElement::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	Q_D(WorksheetElement);
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	auto attribs = reader->attributes();

	auto str = attribs.value("x").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("x").toString());
	else
		d->position.point.setX(str.toDouble());

	str = attribs.value("y").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("y").toString());
	else
		d->position.point.setY(str.toDouble());

	READ_INT_VALUE("horizontalPosition", position.horizontalPosition, WorksheetElement::HorizontalPosition);
	READ_INT_VALUE("verticalPosition", position.verticalPosition, WorksheetElement::VerticalPosition);
	if ( Project::xmlVersion() < 1) {
		// Before 2.9.0 the position.point is only used when horizontalPosition or
		// vertical position was set to custom, otherwise the label was attached to the
		// "position" and it was not possible to arrange relative to this alignpoint
		// From 2.9.0, the horizontalPosition and verticalPosition indicate the anchor
		// point and position.point indicates the distance to them
		// Custom is the same as Center, so rename it, because Custom is legacy
		if (d->position.horizontalPosition != WorksheetElement::HorizontalPosition::Custom) {
			d->position.point.setX(0);
			if (d->position.horizontalPosition == WorksheetElement::HorizontalPosition::Left)
				d->horizontalAlignment = WorksheetElement::HorizontalAlignment::Left;
			else if (d->position.horizontalPosition == WorksheetElement::HorizontalPosition::Right)
				d->horizontalAlignment = WorksheetElement::HorizontalAlignment::Right;
		} else
			d->position.horizontalPosition = WorksheetElement::HorizontalPosition::Center;

		if (d->position.verticalPosition != WorksheetElement::VerticalPosition::Custom) {
			d->position.point.setY(0);
			if (d->position.verticalPosition == WorksheetElement::VerticalPosition::Top)
				d->verticalAlignment = WorksheetElement::VerticalAlignment::Top;
			else if (d->position.verticalPosition == WorksheetElement::VerticalPosition::Bottom)
				d->verticalAlignment = WorksheetElement::VerticalAlignment::Bottom;
		} else
			d->position.verticalPosition = WorksheetElement::VerticalPosition::Center;

		//in the old format the order was reversed, multiple by -1 here
		d->position.point.setY(-d->position.point.y());
	} else {
		READ_INT_VALUE("horizontalAlignment", horizontalAlignment, WorksheetElement::HorizontalAlignment);
		READ_INT_VALUE("verticalAlignment", verticalAlignment, WorksheetElement::VerticalAlignment);
	}
	READ_DOUBLE_VALUE("rotationAngle", rotationAngle);
	READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);

	str = attribs.value("visible").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("visible").toString());
	else
		d->setVisible(str.toInt());

	READ_INT_VALUE("coordinateBinding", coordinateBindingEnabled, bool);

	str = attribs.value("logicalPosX").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("logicalPosX").toString());
	else
		d->positionLogical.setX(str.toDouble());

	str = attribs.value("logicalPosY").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("logicalPosY").toString());
	else
		d->positionLogical.setY(str.toDouble());

	return true;
}

void WorksheetElement::loadThemeConfig(const KConfig &) {
}

void WorksheetElement::saveThemeConfig(const KConfig &) {
}

// coordinate system

void  WorksheetElement::setCoordinateSystemIndex(int index) {
	m_cSystemIndex = index;
	if (m_plot)
		cSystem = dynamic_cast<const CartesianCoordinateSystem*>(m_plot->coordinateSystem(index));
	else
		DEBUG(Q_FUNC_INFO << ", WARNING: No plot found. Failed setting csystem index.")
}

int WorksheetElement::coordinateSystemCount() const {
	if (m_plot)
		return m_plot->coordinateSystemCount();
	DEBUG(Q_FUNC_INFO << ", WARNING: no plot set!")

	return 0;
}

QString WorksheetElement::coordinateSystemInfo(const int index) const {
	if (m_plot)
		return m_plot->coordinateSystem(index)->info();

	return QString();
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(WorksheetElement, WorksheetElement::PositionWrapper, position, position)
BASIC_SHARED_D_READER_IMPL(WorksheetElement, WorksheetElement::HorizontalAlignment, horizontalAlignment, horizontalAlignment)
BASIC_SHARED_D_READER_IMPL(WorksheetElement, WorksheetElement::VerticalAlignment, verticalAlignment, verticalAlignment)
BASIC_SHARED_D_READER_IMPL(WorksheetElement, QPointF, positionLogical, positionLogical)
BASIC_SHARED_D_READER_IMPL(WorksheetElement, qreal, rotationAngle, rotationAngle)
BASIC_SHARED_D_READER_IMPL(WorksheetElement, bool, coordinateBindingEnabled, coordinateBindingEnabled)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(WorksheetElement, SetPosition, WorksheetElement::PositionWrapper, position, retransform)
void WorksheetElement::setPosition(const PositionWrapper& pos) {
	Q_D(WorksheetElement);
	if (pos.point != d->position.point || pos.horizontalPosition != d->position.horizontalPosition || pos.verticalPosition != d->position.verticalPosition)
		exec(new WorksheetElementSetPositionCmd(d, pos, ki18n("%1: set position")));
}

STD_SETTER_CMD_IMPL_F_S(WorksheetElement, SetHorizontalAlignment, WorksheetElement::HorizontalAlignment, horizontalAlignment, retransform)
void WorksheetElement::setHorizontalAlignment(const WorksheetElement::HorizontalAlignment hAlign) {
	Q_D(WorksheetElement);
	if (hAlign != d->horizontalAlignment)
		exec(new WorksheetElementSetHorizontalAlignmentCmd(d, hAlign, ki18n("%1: set horizontal alignment")));
}

STD_SETTER_CMD_IMPL_F_S(WorksheetElement, SetVerticalAlignment, WorksheetElement::VerticalAlignment, verticalAlignment, retransform)
void WorksheetElement::setVerticalAlignment(const WorksheetElement::VerticalAlignment vAlign) {
	Q_D(WorksheetElement);
	if (vAlign != d->verticalAlignment)
		exec(new WorksheetElementSetVerticalAlignmentCmd(d, vAlign, ki18n("%1: set vertical alignment")));
}

STD_SETTER_CMD_IMPL_F_S(WorksheetElement, SetCoordinateBindingEnabled, bool, coordinateBindingEnabled, retransform)
void WorksheetElement::setCoordinateBindingEnabled(bool on) {
	Q_D(WorksheetElement);
	if (on != d->coordinateBindingEnabled)
		exec(new WorksheetElementSetCoordinateBindingEnabledCmd(d, on, on ? ki18n("%1: use logical coordinates") : ki18n("%1: set invisible")));
}

STD_SETTER_CMD_IMPL_F_S(WorksheetElement, SetPositionLogical, QPointF, positionLogical, retransform)
void WorksheetElement::setPositionLogical(QPointF pos) {
	Q_D(WorksheetElement);
	if (pos != d->positionLogical)
		exec(new WorksheetElementSetPositionLogicalCmd(d, pos, ki18n("%1: set logical position")));
}

/*!
 * \brief WorksheetElement::setPosition
 * sets the position without undo/redo-stuff
 * \param point point in scene coordinates
 */
void WorksheetElement::setPosition(QPointF point) {
	Q_D(WorksheetElement);
	if (point != d->position.point) {
		d->position.point = point;
		retransform();
	}
}

/*!
 * position is set to invalid if the parent item is not drawn on the scene
 * (e.g. axis is not drawn because it's outside plot ranges -> don't draw axis' title label)
 */
void WorksheetElement::setPositionInvalid(bool invalid) {
	Q_D(WorksheetElement);
	if (invalid != d->positionInvalid)
		d->positionInvalid = invalid;
}

STD_SETTER_CMD_IMPL_F_S(WorksheetElement, SetRotationAngle, qreal, rotationAngle, recalcShapeAndBoundingRect)
void WorksheetElement::setRotationAngle(qreal angle) {
	Q_D(WorksheetElement);
	if (angle != d->rotationAngle)
		exec(new WorksheetElementSetRotationAngleCmd(d, angle, ki18n("%1: set rotation angle")));
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
WorksheetElementPrivate::WorksheetElementPrivate(WorksheetElement *owner): q(owner) {

}

QString WorksheetElementPrivate::name() const {
	return q->name();
}

QRectF WorksheetElementPrivate::boundingRect() const {
	return boundingRectangle;
}

void WorksheetElementPrivate::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {

}

void WorksheetElementPrivate::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right
			|| event->key() == Qt::Key_Up ||event->key() == Qt::Key_Down) {
		const int delta = 5; // always in scene coordinates

		WorksheetElement::PositionWrapper tempPosition = position;
		if(coordinateBindingEnabled && q->cSystem) {
			//the position in logical coordinates was changed, calculate the position in scene coordinates
			bool visible;
			QPointF p = q->cSystem->mapLogicalToScene(positionLogical, visible, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			if (event->key() == Qt::Key_Left) {
				p.setX(p.x() - delta);
			} else if (event->key() == Qt::Key_Right) {
				p.setX(p.x() + delta);
			} else if (event->key() == Qt::Key_Up) {
				p.setY(p.y() - delta); // Don't understand why I need a negative here and below a positive delta
			} else if (event->key() == Qt::Key_Down) {
				p.setY(p.y() + delta);
			}
			auto pLogic = q->cSystem->mapSceneToLogical(p, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			q->setPositionLogical(pLogic);
		} else {
			QPointF point = q->parentPosToRelativePos(pos(), boundingRectangle, position,
													horizontalAlignment, verticalAlignment);

			if (event->key() == Qt::Key_Left) {
				point.setX(point.x() - delta);
			} else if (event->key() == Qt::Key_Right) {
				point.setX(point.x() + delta);
			} else if (event->key() == Qt::Key_Up) {
				point.setY(point.y() + delta);
			} else if (event->key() == Qt::Key_Down) {
				point.setY(point.y() - delta);
			}
			tempPosition.point = point;
			q->setPosition(tempPosition);
		}
		event->accept();
	} else {
		QGraphicsItem::keyPressEvent(event);
	}
}

void WorksheetElementPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	//convert position of the item in parent coordinates to label's position
	const QPointF point = q->parentPosToRelativePos(pos(),
													boundingRect(), position,
													horizontalAlignment, verticalAlignment);
	if (point != position.point) {
		//position was changed -> set the position related member variables
		suppressRetransform = true;
		WorksheetElement::PositionWrapper tempPosition = position;
		tempPosition.point = point;
		q->setPosition(tempPosition);
		suppressRetransform = false;
	}

	QGraphicsItem::mouseReleaseEvent(event);
}

QVariant WorksheetElementPrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (suppressItemChangeEvent)
		return value;

	if (change == QGraphicsItem::ItemPositionChange) {
		// don't use setPosition here, because then all small changes are on the undo stack
		if(coordinateBindingEnabled) {
			QPointF pos = q->align(value.toPointF(), boundingRectangle, horizontalAlignment, verticalAlignment, false);

			positionLogical = q->cSystem->mapSceneToLogical(pos, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			Q_EMIT q->positionLogicalChanged(positionLogical);
		} else {
			//convert item's center point in parent's coordinates
			WorksheetElement::PositionWrapper tempPosition = position;
			tempPosition.point = q->parentPosToRelativePos(value.toPointF(), boundingRectangle, position,
														horizontalAlignment, verticalAlignment);

			//Q_EMIT the signals in order to notify the UI.
			Q_EMIT q->positionChanged(tempPosition);
		}
	}

	return QGraphicsItem::itemChange(change, value);
}

/*!
 * \brief TextLabelPrivate::mapParentToPlotArea
 * Mapping a point from parent coordinates to plotArea coordinates
 * Needed because in some cases the parent is not the PlotArea, but a child of it (Marker/InfoElement)
 * IMPORTANT: function is also used in Custompoint, so when changing anything, change it also there
 * \param point point in parent coordinates
 * \return point in PlotArea coordinates
 */
QPointF WorksheetElementPrivate::mapParentToPlotArea(QPointF point) {
	AbstractAspect* parent = q->parent(AspectType::CartesianPlot);
	if (parent) {
		auto* plot = static_cast<CartesianPlot*>(parent);
		// mapping from parent to item coordinates and them to plot area
		return mapToItem(plot->plotArea()->graphicsItem(), mapFromParent(point));
	}

	return point; // don't map if no parent set. Then it's during load
}

/*!
 * \brief TextLabelPrivate::mapPlotAreaToParent
 * Mapping a point from the PlotArea (CartesianPlot::plotArea) coordinates to the parent
 * coordinates of this item
 * Needed because in some cases the parent is not the PlotArea, but a child of it (Marker/InfoElement)
 * IMPORTANT: function is also used in Custompoint, so when changing anything, change it also there
 * \param point point in plotArea coordinates
 * \return point in parent coordinates
 */
QPointF WorksheetElementPrivate::mapPlotAreaToParent(QPointF point) {
	AbstractAspect* parent = q->parent(AspectType::CartesianPlot);

	if (parent) {
		auto* plot = static_cast<CartesianPlot*>(parent);
		// first mapping to item coordinates and from there back to parent
		// WorksheetinfoElement: parentItem()->parentItem() == plot->graphicsItem()
		// plot->graphicsItem().pos() == plot->plotArea()->graphicsItem().pos()
		return mapToParent(mapFromItem(plot->plotArea()->graphicsItem(), point));
	}

	return point; // don't map if no parent set. Then it's during load
}

