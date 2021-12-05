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
#include "plots/AbstractPlot.h"
#include "plots/cartesian/CartesianCoordinateSystem.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMenu>
#include <QPen>
#include <KLocalizedString>

/**
 * \class WorksheetElement
 * \brief Base class for all Worksheet children.
 *
 */
WorksheetElement::WorksheetElement(const QString& name, AspectType type)
	: AbstractAspect(name, type) {

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
		emit plotRangeListChanged();
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

/**
 * \fn void WorksheetElement::retransform()
 * \brief Tell the element to newly transform its graphics item into its coordinate system.
 *
 * This method must not change the undo-aware data of the element, only
 * the graphics item which represents the item is to be updated.
 */

void WorksheetElement::setZValue(qreal value) {
	graphicsItem()->setZValue(value);
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
	emit moveBegin();
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
	emit moveEnd();
}

void WorksheetElement::execMoveBehind(QAction* action) {
	emit moveBegin();
	AbstractAspect* parent = parentAspect();
	int index = action->data().toInt();
	AbstractAspect* sibling = parent->child<WorksheetElement>(index);
	beginMacro(i18n("%1: move in front of %2.", name(), sibling->name()));
	setMoved(true);
	remove();
	parent->insertChildBefore(this, sibling);
	setMoved(false);
	endMacro();
	emit moveEnd();
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
		return QPointF(pos.x() + xAlign, pos.y() + yAlign);
	else
		return QPointF(pos.x() - xAlign, pos.y() + yAlign);
}

QRectF WorksheetElement::parentRect() const {
	QRectF rect;
	if (plot()) {
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
