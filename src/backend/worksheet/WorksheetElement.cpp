/***************************************************************************
    File                 : WorksheetElement.cpp
    Project              : LabPlot
    Description          : Base class for all Worksheet children.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2017 by Alexander Semke (alexander.semke@web.de)

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

#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/WorksheetElement.h"
#include "plots/AbstractPlot.h"
#include "plots/cartesian/CartesianCoordinateSystem.h"

#include <QGraphicsItem>
#include <QMenu>
#include <QPen>
#include <KLocalizedString>

/**
 * \class WorksheetElement
 * \brief Base class for all Worksheet children.
 *
 */
WorksheetElement::WorksheetElement(const QString &name, AspectType type)
	: AbstractAspect(name, type) {

	m_drawingOrderMenu = new QMenu(i18n("Drawing &order"));
	m_drawingOrderMenu->setIcon(QIcon::fromTheme("layer-bottom"));
	m_moveBehindMenu = new QMenu(i18n("Move &behind"));
	m_moveBehindMenu->setIcon(QIcon::fromTheme("draw-arrow-down"));
	m_moveInFrontOfMenu = new QMenu(i18n("Move in &front of"));
	m_moveInFrontOfMenu->setIcon(QIcon::fromTheme("draw-arrow-up"));
	m_drawingOrderMenu->addMenu(m_moveBehindMenu);
	m_drawingOrderMenu->addMenu(m_moveInFrontOfMenu);

	connect(m_moveBehindMenu, &QMenu::aboutToShow, this, &WorksheetElement::prepareMoveBehindMenu);
	connect(m_moveInFrontOfMenu, &QMenu::aboutToShow, this, &WorksheetElement::prepareMoveInFrontOfMenu);
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
	m_plot = dynamic_cast<CartesianPlot*>(parentAspect());
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

void WorksheetElement::prepareMoveBehindMenu() {
	m_moveBehindMenu->clear();
	AbstractAspect* parent = parentAspect();
	int index = parent->indexOfChild<WorksheetElement>(this);
	const QVector<WorksheetElement*>& children = parent->children<WorksheetElement>();

	for (int i = 0; i < index; ++i) {
		const WorksheetElement* elem = children.at(i);
		//axes and legends are always drawn on top of other elements, don't add them to the menu
		if (elem->type() != AspectType::Axis && elem->type() != AspectType::CartesianPlotLegend) {
			QAction* action = m_moveBehindMenu->addAction(elem->icon(), elem->name());
			action->setData(i);
		}
	}

	//TODO: doesn't always work properly
	//hide the "move behind" menu if it doesn't have any entries, show if not shown yet otherwise
	//m_moveBehindMenu->menuAction()->setVisible(!m_moveBehindMenu->isEmpty());
}

void WorksheetElement::prepareMoveInFrontOfMenu() {
	m_moveInFrontOfMenu->clear();
	AbstractAspect* parent = parentAspect();
	int index = parent->indexOfChild<WorksheetElement>(this);
	const QVector<WorksheetElement*>& children = parent->children<WorksheetElement>();

	for (int i = index + 1; i < children.size(); ++i) {
		const WorksheetElement* elem = children.at(i);
		//axes and legends are always drawn on top of other elements, don't add them to the menu
		if (elem->type() != AspectType::Axis && elem->type() != AspectType::CartesianPlotLegend) {
			QAction* action = m_moveInFrontOfMenu->addAction(elem->icon(), elem->name());
			action->setData(i);
		}
	}

	//TODO: doesn't alway work properly
	//hide the "move in front" menu if it doesn't have any entries, show if not shown yet otherwise
	//m_moveInFrontOfMenu->menuAction()->setVisible(!m_moveInFrontOfMenu->isEmpty());
}

void WorksheetElement::execMoveInFrontOf(QAction* action) {
	emit moveBegin();
	AbstractAspect* parent = parentAspect();
	int index = action->data().toInt();
	AbstractAspect* sibling1 = parent->child<WorksheetElement>(index);
	AbstractAspect* sibling2 = parent->child<WorksheetElement>(index + 1);
	beginMacro(i18n("%1: move behind %2.", name(), sibling1->name()));
	remove();
	parent->insertChildBefore(this, sibling2);
	endMacro();
	emit moveEnd();
}

void WorksheetElement::execMoveBehind(QAction* action) {
	emit moveBegin();
	AbstractAspect* parent = parentAspect();
	int index = action->data().toInt();
	AbstractAspect* sibling = parent->child<WorksheetElement>(index);
	beginMacro(i18n("%1: move in front of %2.", name(), sibling->name()));
	remove();
	parent->insertChildBefore(this, sibling);
	endMacro();
	emit moveEnd();
}

QPointF WorksheetElement::parentPosToRelativePos(QPointF parentPos, QRectF parentRect, QRectF worksheetElementRect, PositionWrapper position, WorksheetElement::HorizontalAlignment horAlign, WorksheetElement::VerticalAlignment vertAlign) const {

    QPointF relPos;
    // positive is right
	double xAlign;
	switch (horAlign) {
	case WorksheetElement::HorizontalAlignment::Left:
		xAlign = worksheetElementRect.width()/2;
		break;
	case WorksheetElement::HorizontalAlignment::Right:
		xAlign = - worksheetElementRect.width()/2;
		break;
	case WorksheetElement::HorizontalAlignment::Center:
		// Fall through
	default:
		xAlign = 0;
	break;
	}

	if (position.horizontalPosition == HorizontalPosition::Left)
		relPos.setX(parentPos.x() - (parentRect.x() + xAlign));
	else if (position.horizontalPosition == HorizontalPosition::Center || position.horizontalPosition == HorizontalPosition::Custom)
		relPos.setX(parentPos.x() - (parentRect.x() + parentRect.width()/2 + xAlign));
	else  //position.horizontalPosition == WorksheetElement::HorizontalPosition::Right // default
		relPos.setX(parentPos.x() - (parentRect.x() + parentRect.width() - xAlign));

	// positive is to top
	double yAlign;
	switch (vertAlign) {
	case WorksheetElement::VerticalAlignment::Bottom:
		yAlign = - worksheetElementRect.height()/2;
		break;
	case WorksheetElement::VerticalAlignment::Top:
		yAlign = worksheetElementRect.height()/2;
		break;
	case WorksheetElement::VerticalAlignment::Center:
		// Fall through
	default:
		yAlign = 0;
	break;
	}

	if (position.verticalPosition == VerticalPosition::Center|| position.verticalPosition == VerticalPosition::Custom)
		relPos.setY(parentRect.y() + parentRect.height()/2 - parentPos.y() + yAlign);
	else if (position.verticalPosition == VerticalPosition::Bottom)
		relPos.setY(parentRect.y() + parentRect.height() + yAlign - parentPos.y());
	else // position.verticalPosition == VerticalPosition::Top // default
		relPos.setY(parentRect.y() + yAlign - parentPos.y());
	return relPos;
}

QPointF WorksheetElement::relativePosToParentPos(QRectF parentRect, QRectF worksheetElementRect, PositionWrapper position, WorksheetElement::HorizontalAlignment horAlign, WorksheetElement::VerticalAlignment vertAlign) const {
    QPointF parentPos;

	// positive is right of the anchor point
	double xAlign;
	switch (horAlign) {
	case WorksheetElement::HorizontalAlignment::Left:
		xAlign = worksheetElementRect.width()/2;
		break;
	case WorksheetElement::HorizontalAlignment::Right:
		xAlign = - worksheetElementRect.width()/2;
		break;
	case WorksheetElement::HorizontalAlignment::Center:
		// Fall through
	default:
		xAlign = 0;
	break;
	}

	if (position.horizontalPosition == HorizontalPosition::Left)
		parentPos.setX(parentRect.x() + position.point.x() + xAlign);
	else if (position.horizontalPosition == HorizontalPosition::Center || position.horizontalPosition == HorizontalPosition::Custom)
		parentPos.setX(parentRect.x() + parentRect.width()/2 + position.point.x() + xAlign);
	else  //position.horizontalPosition == WorksheetElement::HorizontalPosition::Right // default
		parentPos.setX(parentRect.x() + parentRect.width() + position.point.x() + xAlign);


	// positive is above the anchor point
	double yAlign;
	switch (vertAlign) {
	case WorksheetElement::VerticalAlignment::Bottom:
		yAlign = - worksheetElementRect.height()/2;
		break;
	case WorksheetElement::VerticalAlignment::Top:
		yAlign = worksheetElementRect.height()/2;
		break;
	case WorksheetElement::VerticalAlignment::Center:
		// Fall through
	default:
		yAlign = 0;
	break;
	}
	if (position.verticalPosition == VerticalPosition::Center || position.verticalPosition == VerticalPosition::Custom)
		parentPos.setY(parentRect.y() + parentRect.height()/2 - position.point.y() + yAlign);
	else if (position.verticalPosition == VerticalPosition::Bottom)
		parentPos.setY(parentRect.y() + parentRect.height() +  yAlign - position.point.y());
	else // position.verticalPosition == WorksheetElement::VerticalPosition::Top // default
		parentPos.setY(parentRect.y() + yAlign- position.point.y());

	return parentPos;
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
