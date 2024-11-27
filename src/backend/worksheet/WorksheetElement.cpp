/*
	File                 : WorksheetElement.cpp
	Project              : LabPlot
	Description          : Base class for all Worksheet children.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2012-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/worksheet/WorksheetElement.h"
#include "backend/core/AspectPrivate.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/WorksheetElementPrivate.h"
#include "plots/AbstractPlot.h"
#include "plots/PlotArea.h"
#include "plots/cartesian/CartesianCoordinateSystem.h"
#include "plots/cartesian/Plot.h"

#include <KLocalizedString>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPen>

/**
 * \class WorksheetElement
 * \brief Base class for all Worksheet children.
 *
 */
// WorksheetElement::WorksheetElement(const QString& name, AspectType type)
//: AbstractAspect(name, type), d_ptr(new WorksheetElementPrivate(this)) {
//	init();
//}

WorksheetElement::WorksheetElement(const QString& name, WorksheetElementPrivate* dd, AspectType type)
	: AbstractAspect(name, type)
	, d_ptr(dd) {
	init();
}

void WorksheetElement::init() {
	Q_D(WorksheetElement);
	d->setData(0, static_cast<quint64>(type()));
}

WorksheetElement::~WorksheetElement() {
	delete m_moveBehindMenu;
	delete m_moveInFrontOfMenu;
	delete m_drawingOrderMenu;
}

void WorksheetElement::finalizeAdd() {
	DEBUG(Q_FUNC_INFO)
	Q_D(WorksheetElement);
	if (!d->m_plot) {
		// determine the plot parent which is not neccessarily the parent aspect like for
		// * child CustomPoint in InfoeElement
		// * child XYCurves in QQPlot
		// * etc.
		d->m_plot = dynamic_cast<CartesianPlot*>(parent(AspectType::CartesianPlot));
	}

	if (d->m_plot) {
		cSystem = dynamic_cast<const CartesianCoordinateSystem*>(d->m_plot->coordinateSystem(m_cSystemIndex));
		Q_EMIT plotRangeListChanged();
	} else
		DEBUG(Q_FUNC_INFO << ", WARNING: no plot available.")
}

/**
 * \fn QGraphicsItem *WorksheetElement::graphicsItem() const
 * \brief Return the graphics item representing this element.
 *
 */
QGraphicsItem* WorksheetElement::graphicsItem() const {
	return d_ptr;
}

/*!
 * \brief WorksheetElement::setParentGraphicsItem
 * Sets the parent graphicsitem, needed for binding to coord
 * \param item parent graphicsitem
 */
void WorksheetElement::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(WorksheetElement);
	d->setParentItem(item);
}

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

void WorksheetElement::setSuppressRetransform(bool value) {
	Q_D(WorksheetElement);
	d->suppressRetransform = value;
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

void WorksheetElement::changeVisibility() {
	Q_D(const WorksheetElement);
	this->setVisible(!d->isVisible());
}

void WorksheetElement::changeLocking() {
	this->setLock(!isLocked());
}

STD_SETTER_CMD_IMPL_S(WorksheetElement, SetLock, bool, lock)
void WorksheetElement::setLock(bool lock) {
	Q_D(WorksheetElement);
	if (lock != d->lock) {
		if (!lock && isHovered())
			setHover(false);
		exec(new WorksheetElementSetLockCmd(d, lock, lock ? ki18n("%1: lock") : ki18n("%1: unlock")));
	}
}

STD_SWAP_METHOD_SETTER_CMD_IMPL_F(WorksheetElement, SetVisible, bool, swapVisible, update)
void WorksheetElement::setVisible(bool on) {
	Q_D(WorksheetElement);
	exec(new WorksheetElementSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool WorksheetElementPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();

	// When making a graphics item invisible, it gets deselected in the scene.
	// In this case we don't want to deselect the item in the project explorer.
	// We need to supress the deselection in the view.
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
QPainterPath WorksheetElement::shapeFromPath(const QPainterPath& path, const QPen& pen) {
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

QAction* WorksheetElement::visibilityAction() {
	if (!m_visibilityAction) {
		m_visibilityAction = new QAction(QIcon::fromTheme(QStringLiteral("view-visible")), i18n("Visible"), this);
		m_visibilityAction->setCheckable(true);
		connect(m_visibilityAction, &QAction::triggered, this, &WorksheetElement::changeVisibility);
	}
	return m_visibilityAction;
}

QAction* WorksheetElement::lockingAction() {
	if (!m_lockingAction) {
		m_lockingAction = new QAction(QIcon::fromTheme(QStringLiteral("hidemouse")), i18n("Lock"), this);
		m_lockingAction->setCheckable(true);
		connect(m_lockingAction, &QAction::triggered, this, &WorksheetElement::changeLocking);
	}
	return m_lockingAction;
}

QMenu* WorksheetElement::createContextMenu() {
	if (!m_drawingOrderMenu) {
		m_drawingOrderMenu = new QMenu(i18n("Drawing &order"));
		m_drawingOrderMenu->setIcon(QIcon::fromTheme(QStringLiteral("layer-bottom")));

		m_moveBehindMenu = new QMenu(i18n("Move &behind"));
		m_moveBehindMenu->setIcon(QIcon::fromTheme(QStringLiteral("draw-arrow-down")));
		m_drawingOrderMenu->addMenu(m_moveBehindMenu);

		m_moveInFrontOfMenu = new QMenu(i18n("Move in &front of"));
		m_moveInFrontOfMenu->setIcon(QIcon::fromTheme(QStringLiteral("draw-arrow-up")));
		m_drawingOrderMenu->addMenu(m_moveInFrontOfMenu);

		connect(m_drawingOrderMenu, &QMenu::aboutToShow, this, &WorksheetElement::prepareDrawingOrderMenu);
		connect(m_moveBehindMenu, &QMenu::triggered, this, &WorksheetElement::execMoveBehind);
		connect(m_moveInFrontOfMenu, &QMenu::triggered, this, &WorksheetElement::execMoveInFrontOf);
	}

	QMenu* menu = AbstractAspect::createContextMenu();
	QAction* firstAction = menu->actions().at(1); // skip the first action because of the "title-action"

	auto* visibilityAction = this->visibilityAction();
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);
	menu->insertSeparator(firstAction);

	// don't add the lock action for elements which cannot be freely moved on the worksheet (like axis and curves/plots)
	if (!dynamic_cast<Axis*>(this) && !dynamic_cast<Plot*>(this)) {
		auto* lockingAction = this->lockingAction();
		lockingAction->setChecked(isLocked());
		menu->insertAction(firstAction, lockingAction);
		menu->insertSeparator(firstAction);
	}

	// add the sub-menu for the drawing order
	// don't add the drawing order menu for axes and legends, they're always drawn on top of each other elements
	if (type() == AspectType::Axis || type() == AspectType::CartesianPlotLegend)
		return menu;

	// for plots in a worksheet with an active layout the Z-factor is not relevant but we still
	// want to use the "Drawing order" menu to be able to change the position/order of the plot in the layout.
	// Since the order of the child in the list of children is opposite to the Z-factor, we change
	// the names of the menus to adapt to the reversed logic.
	if (dynamic_cast<AbstractPlot*>(this)) {
		const auto* w = dynamic_cast<const Worksheet*>(this->parentAspect());
		if (!w)
			return menu;

		if (w->layout() != Worksheet::Layout::NoLayout) {
			m_moveBehindMenu->setTitle(i18n("Move in &front of"));
			m_moveBehindMenu->setIcon(QIcon::fromTheme(QStringLiteral("draw-arrow-up")));
			m_moveInFrontOfMenu->setTitle(i18n("Move &behind"));
			m_moveInFrontOfMenu->setIcon(QIcon::fromTheme(QStringLiteral("draw-arrow-down")));
		} else {
			m_moveBehindMenu->setTitle(i18n("Move &behind"));
			m_moveBehindMenu->setIcon(QIcon::fromTheme(QStringLiteral("draw-arrow-down")));
			m_moveInFrontOfMenu->setTitle(i18n("Move in &front of"));
			m_moveInFrontOfMenu->setIcon(QIcon::fromTheme(QStringLiteral("draw-arrow-up")));
		}
	}

	// don't add the drawing order menu if the parent element has no other children
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
	const auto* parent = parentAspect();
	const int index = parent->indexOfChild<AbstractAspect>(this, ChildIndexFlag::IncludeHidden);
	const auto& children = parent->children<AbstractAspect>(ChildIndexFlag::IncludeHidden);

	//"move behind" sub-menu
	m_moveBehindMenu->clear();
	for (int i = 0; i < index; ++i) {
		const auto* elem = children.at(i);
		if (elem->isHidden())
			continue;
		// axes and legends are always drawn on top of other elements, don't add them to the menu
		if (elem->type() != AspectType::Axis && elem->type() != AspectType::CartesianPlotLegend) {
			auto* action = m_moveBehindMenu->addAction(elem->icon(), elem->name());
			action->setData(i);
		}
	}

	//"move in front of" sub-menu
	m_moveInFrontOfMenu->clear();
	for (int i = index + 1; i < children.size(); ++i) {
		const auto* elem = children.at(i);
		if (elem->isHidden())
			continue;
		// axes and legends are always drawn on top of other elements, don't add them to the menu
		if (elem->type() != AspectType::Axis && elem->type() != AspectType::CartesianPlotLegend) {
			auto* action = m_moveInFrontOfMenu->addAction(elem->icon(), elem->name());
			action->setData(i);
		}
	}

	// hide the sub-menus if they don't have any entries
	m_moveInFrontOfMenu->menuAction()->setVisible(!m_moveInFrontOfMenu->isEmpty());
	m_moveBehindMenu->menuAction()->setVisible(!m_moveBehindMenu->isEmpty());
}

void WorksheetElement::execMoveInFrontOf(QAction* action) {
	auto* parent = parentAspect();
	const int newIndex = action->data().toInt();
	const int currIndex = parent->indexOfChild<AbstractAspect>(this, ChildIndexFlag::IncludeHidden);
	parent->moveChild(this, newIndex - currIndex);
}

void WorksheetElement::execMoveBehind(QAction* action) {
	auto* parent = parentAspect();
	const int newIndex = action->data().toInt();
	const int currIndex = parent->indexOfChild<AbstractAspect>(this, ChildIndexFlag::IncludeHidden);
	parent->moveChild(this, newIndex - currIndex);
}

QPointF WorksheetElement::align(QPointF pos, QRectF rect, HorizontalAlignment horAlign, VerticalAlignment vertAlign, bool positive) const {
	// positive is right
	double xAlign = 0.;
	switch (horAlign) {
	case HorizontalAlignment::Left:
		xAlign = rect.width() / 2.;
		break;
	case HorizontalAlignment::Right:
		xAlign = -rect.width() / 2.;
		break;
	case HorizontalAlignment::Center:
		break;
	}

	// positive is to top
	double yAlign = 0.;
	switch (vertAlign) {
	case VerticalAlignment::Bottom:
		yAlign = -rect.height() / 2.;
		break;
	case VerticalAlignment::Top:
		yAlign = rect.height() / 2.;
		break;
	case VerticalAlignment::Center:
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
	auto* parent = parentAspect();
	if (parent && parent->type() == AspectType::CartesianPlot && plot()) {
		if (type() != AspectType::Axis)
			rect = plot()->graphicsItem()->mapRectFromScene(plot()->rect());
		else
			rect = plot()->dataRect(); // axes are positioned relative to the data rect and not to the whole plot rect
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
 * alignment point of the parent and the element
 * \param parentPos Element position in parent coordinates
 * \param parentRect Parent data rect
 * \param rect element's rect
 * \param position contains the alignement of the element to the parent
 * \return distance between the parent position to the element
 */
QPointF WorksheetElement::parentPosToRelativePos(QPointF parentPos, PositionWrapper position) const {
	// increasing relative pos hor --> right
	// increasing relative pos vert --> top
	// increasing parent pos hor --> right
	// increasing parent pos vert --> bottom

	QRectF parentRect = this->parentRect();
	QPointF relPos;

	double percentage = 0.;
	switch (position.horizontalPosition) {
	case HorizontalPosition::Left:
		break;
	case HorizontalPosition::Center:
		percentage = 0.5;
		break;
	case HorizontalPosition::Right:
		percentage = 1.0;
		break;
	case HorizontalPosition::Relative:
		percentage = position.point.x();
	}

	relPos.setX(parentPos.x() - (parentRect.x() + parentRect.width() * percentage));

	switch (position.verticalPosition) {
	case VerticalPosition::Top:
		percentage = 0.;
		break;
	case VerticalPosition::Center:
		percentage = 0.5;
		break;
	case VerticalPosition::Bottom:
		percentage = 1.0;
		break;
	case VerticalPosition::Relative:
		percentage = position.point.y();
	}

	relPos.setY(parentRect.y() + parentRect.height() * percentage - parentPos.y());

	return relPos;
}

/*!
 * \brief relativePosToParentPos
 * \param parentRect
 * \param rect element's rect
 * \param position contains the alignment of the element to the parent
 * \return parent position
 */
QPointF WorksheetElement::relativePosToParentPos(PositionWrapper position) const {
	// increasing relative pos hor --> right
	// increasing relative pos vert --> top
	// increasing parent pos hor --> right
	// increasing parent pos vert --> bottom

	QRectF parentRect = this->parentRect();
	QPointF parentPos;

	double percentage = 0.;
	switch (position.horizontalPosition) {
	case HorizontalPosition::Left:
	case HorizontalPosition::Relative:
		break;
	case HorizontalPosition::Center:
		percentage = 0.5;
		break;
	case HorizontalPosition::Right:
		percentage = 1.0;
		break;
	}

	if (position.horizontalPosition == HorizontalPosition::Relative)
		parentPos.setX(parentRect.x() + parentRect.width() * position.point.x());
	else
		parentPos.setX(parentRect.x() + parentRect.width() * percentage + position.point.x());

	switch (position.verticalPosition) {
	case VerticalPosition::Top:
		percentage = 0.;
		break;
	case VerticalPosition::Center:
		percentage = 0.5;
		break;
	case VerticalPosition::Bottom:
		percentage = 1.0;
		break;
	case VerticalPosition::Relative:
		break;
	}

	if (position.verticalPosition == VerticalPosition::Relative)
		parentPos.setY(parentRect.y() + parentRect.height() * position.point.y());
	else
		parentPos.setY(parentRect.y() + parentRect.height() * percentage - position.point.y());

	return parentPos;
}

/*!
 * \brief handleAspectUpdated
 * in some cases one aspect can depend on another, like a XYCurve on Column
 * or InfoElement on XYCurve. This is a generic function called for
 * all Elements when a new aspect will be added even it is not a child of the
 * current element
 *
 * Path is explicit specified, so it must not be recalculated every time when iterating over multiple
 * WorksheetElements. The path is the same as aspect->path()
 * \param path
 */
void WorksheetElement::handleAspectUpdated(const QString& path, const AbstractAspect* aspect) {
	Q_UNUSED(path);
	Q_UNUSED(aspect);
}

void WorksheetElement::save(QXmlStreamWriter* writer) const {
	Q_D(const WorksheetElement);
	writer->writeAttribute(QStringLiteral("x"), QString::number(d->position.point.x()));
	writer->writeAttribute(QStringLiteral("y"), QString::number(d->position.point.y()));
	writer->writeAttribute(QStringLiteral("horizontalPosition"), QString::number(static_cast<int>(d->position.horizontalPosition)));
	writer->writeAttribute(QStringLiteral("verticalPosition"), QString::number(static_cast<int>(d->position.verticalPosition)));
	writer->writeAttribute(QStringLiteral("horizontalAlignment"), QString::number(static_cast<int>(d->horizontalAlignment)));
	writer->writeAttribute(QStringLiteral("verticalAlignment"), QString::number(static_cast<int>(d->verticalAlignment)));
	writer->writeAttribute(QStringLiteral("rotationAngle"), QString::number(d->rotation()));
	writer->writeAttribute(QStringLiteral("plotRangeIndex"), QString::number(m_cSystemIndex));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeAttribute(QStringLiteral("coordinateBinding"), QString::number(d->coordinateBindingEnabled));
	writer->writeAttribute(QStringLiteral("logicalPosX"), QString::number(d->positionLogical.x()));
	writer->writeAttribute(QStringLiteral("logicalPosY"), QString::number(d->positionLogical.y()));
	writer->writeAttribute(QStringLiteral("locked"), QString::number(d->lock));
}

bool WorksheetElement::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	Q_D(WorksheetElement);
	auto attribs = reader->attributes();

	auto str = attribs.value(QStringLiteral("x")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("x"));
	else
		d->position.point.setX(str.toDouble());

	str = attribs.value(QStringLiteral("y")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("y"));
	else
		d->position.point.setY(str.toDouble());

	READ_INT_VALUE("horizontalPosition", position.horizontalPosition, HorizontalPosition);
	READ_INT_VALUE("verticalPosition", position.verticalPosition, VerticalPosition);
	if (Project::xmlVersion() < 11) {
		// In earlier versions 3 was custom which is now center. But now 3 is relative
		if ((int)d->position.horizontalPosition == 3) {
			d->position.horizontalPosition = HorizontalPosition::Center;
		}
		if ((int)d->position.verticalPosition == 3) {
			d->position.verticalPosition = VerticalPosition::Center;
		}
	}
	if (Project::xmlVersion() < 1) {
		// Before 2.9.0 the position.point is only used when horizontalPosition or
		// vertical position was set to Custom, otherwise the label was attached to the
		// "position" and it was not possible to arrange relative to this anchor point
		// From 2.9.0, the horizontalPosition and verticalPosition indicate the anchor
		// point and position.point indicates the distance to them
		if (d->position.horizontalPosition != HorizontalPosition::Relative) {
			d->position.point.setX(0);
			if (d->position.horizontalPosition == HorizontalPosition::Left)
				d->horizontalAlignment = HorizontalAlignment::Left;
			else if (d->position.horizontalPosition == HorizontalPosition::Right)
				d->horizontalAlignment = HorizontalAlignment::Right;
		} else // TODO
			d->position.horizontalPosition = HorizontalPosition::Center;

		if (d->position.verticalPosition != VerticalPosition::Relative) {
			d->position.point.setY(0);
			if (d->position.verticalPosition == VerticalPosition::Top)
				d->verticalAlignment = VerticalAlignment::Top;
			else if (d->position.verticalPosition == VerticalPosition::Bottom)
				d->verticalAlignment = VerticalAlignment::Bottom;
		} else // TODO
			d->position.verticalPosition = VerticalPosition::Center;

		// in the old format the order was reversed, multiply by -1 here
		d->position.point.setY(-d->position.point.y());
	} else {
		READ_INT_VALUE("horizontalAlignment", horizontalAlignment, HorizontalAlignment);
		READ_INT_VALUE("verticalAlignment", verticalAlignment, VerticalAlignment);
	}
	if (project()->xmlVersion() >= 8) {
		QGRAPHICSITEM_READ_DOUBLE_VALUE("rotationAngle", Rotation);
	} else {
		str = attribs.value(QStringLiteral("rotationAngle")).toString();
		if (str.isEmpty())
			reader->raiseMissingAttributeWarning(QStringLiteral("rotationAngle"));
		else
			d->setRotation(-1 * str.toDouble());
	}
	READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);

	str = attribs.value(QStringLiteral("visible")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("visible"));
	else
		d->setVisible(str.toInt());

	READ_INT_VALUE("coordinateBinding", coordinateBindingEnabled, bool);

	str = attribs.value(QStringLiteral("logicalPosX")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("logicalPosX"));
	else
		d->positionLogical.setX(str.toDouble());

	str = attribs.value(QStringLiteral("logicalPosY")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("logicalPosY"));
	else
		d->positionLogical.setY(str.toDouble());

	str = attribs.value(QStringLiteral("locked")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("locked"));
	else
		d->lock = static_cast<bool>(str.toInt());

	return true;
}

void WorksheetElement::loadThemeConfig(const KConfig&) {
}

void WorksheetElement::saveThemeConfig(const KConfig&) {
}

// coordinate system

class SetCoordinateSystemIndexCmd : public QUndoCommand {
public:
	SetCoordinateSystemIndexCmd(WorksheetElement* element, int index, QUndoCommand* parent = nullptr)
		: QUndoCommand(parent)
		, m_element(element)
		, m_index(index) {
	}

	virtual void redo() override {
		const auto oldIndex = m_element->m_cSystemIndex;
		m_element->m_cSystemIndex = m_index;
		if (m_element->plot())
			m_element->cSystem = dynamic_cast<const CartesianCoordinateSystem*>(m_element->plot()->coordinateSystem(m_index));
		else
			DEBUG(Q_FUNC_INFO << ", WARNING: No plot found. Failed setting csystem index.")

		m_index = oldIndex;
		m_element->retransform();
		Q_EMIT m_element->coordinateSystemIndexChanged(m_element->m_cSystemIndex);
	}

	virtual void undo() override {
		redo();
	}

private:
	WorksheetElement* m_element;
	int m_index;
};

void WorksheetElement::setCoordinateSystemIndex(int index, QUndoCommand* parent) {
	if (index != m_cSystemIndex) {
		auto* command = new SetCoordinateSystemIndexCmd(this, index, parent);
		if (!parent)
			exec(command);
	} else if (!cSystem) {
		// during load the index will be set,
		// but the element might not have yet a plot assigned
		if (plot())
			cSystem = dynamic_cast<const CartesianCoordinateSystem*>(plot()->coordinateSystem(index));
		retransform();
	}
}

int WorksheetElement::coordinateSystemCount() const {
	Q_D(const WorksheetElement);
	if (d->m_plot)
		return d->m_plot->coordinateSystemCount();
	DEBUG(Q_FUNC_INFO << ", WARNING: no plot set!")

	return 0;
}

QString WorksheetElement::coordinateSystemInfo(const int index) const {
	Q_D(const WorksheetElement);
	if (d->m_plot)
		return d->m_plot->coordinateSystem(index)->info();

	return {};
}

bool WorksheetElement::isHovered() const {
	Q_D(const WorksheetElement);
	return d->isHovered();
}

void WorksheetElement::setHover(bool on) {
	Q_D(WorksheetElement);
	d->setHover(on);
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(WorksheetElement, WorksheetElement::PositionWrapper, position, position)
BASIC_SHARED_D_READER_IMPL(WorksheetElement, WorksheetElement::HorizontalAlignment, horizontalAlignment, horizontalAlignment)
BASIC_SHARED_D_READER_IMPL(WorksheetElement, WorksheetElement::VerticalAlignment, verticalAlignment, verticalAlignment)
BASIC_SHARED_D_READER_IMPL(WorksheetElement, QPointF, positionLogical, positionLogical)
BASIC_SHARED_D_READER_IMPL(WorksheetElement,
						   qreal,
						   rotationAngle,
						   rotation() * -1) // the rotation is in qgraphicsitem different to the convention used in labplot
BASIC_SHARED_D_READER_IMPL(WorksheetElement, bool, coordinateBindingEnabled, coordinateBindingEnabled)
BASIC_SHARED_D_READER_IMPL(WorksheetElement, qreal, scale, scale())
BASIC_SHARED_D_READER_IMPL(WorksheetElement, bool, isLocked, lock)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S_SC(WorksheetElement, SetPosition, WorksheetElement::PositionWrapper, position, updatePosition, objectPositionChanged)
void WorksheetElement::setPosition(const PositionWrapper& pos) {
	Q_D(WorksheetElement);
	if (pos.point != d->position.point || pos.horizontalPosition != d->position.horizontalPosition || pos.verticalPosition != d->position.verticalPosition
		|| pos.positionLimit != d->position.positionLimit)
		exec(new WorksheetElementSetPositionCmd(d, pos, ki18n("%1: set position")));
}

STD_SETTER_CMD_IMPL_F_S_SC(WorksheetElement,
						   SetHorizontalAlignment,
						   WorksheetElement::HorizontalAlignment,
						   horizontalAlignment,
						   updatePosition,
						   objectPositionChanged)
void WorksheetElement::setHorizontalAlignment(const WorksheetElement::HorizontalAlignment hAlign) {
	Q_D(WorksheetElement);
	if (hAlign != d->horizontalAlignment)
		exec(new WorksheetElementSetHorizontalAlignmentCmd(d, hAlign, ki18n("%1: set horizontal alignment")));
}

STD_SETTER_CMD_IMPL_F_S_SC(WorksheetElement,
						   SetVerticalAlignment,
						   WorksheetElement::VerticalAlignment,
						   verticalAlignment,
						   updatePosition,
						   objectPositionChanged)
void WorksheetElement::setVerticalAlignment(const WorksheetElement::VerticalAlignment vAlign) {
	Q_D(WorksheetElement);
	if (vAlign != d->verticalAlignment)
		exec(new WorksheetElementSetVerticalAlignmentCmd(d, vAlign, ki18n("%1: set vertical alignment")));
}

STD_SETTER_CMD_IMPL_S(WorksheetElement, SetCoordinateBindingEnabled, bool, coordinateBindingEnabled) // do I need a final method?
bool WorksheetElement::setCoordinateBindingEnabled(bool on) {
	Q_D(WorksheetElement);
	if (on && !cSystem)
		return false;
	if (on != d->coordinateBindingEnabled) {
		// Must not be in the Undo Command,
		// because if done once, logical and
		// scene pos are synched and therefore
		// when changing it does not have any visual effect
		d->updatePosition();
		exec(new WorksheetElementSetCoordinateBindingEnabledCmd(d, on, on ? ki18n("%1: use logical coordinates") : ki18n("%1: set invisible")));
		return true;
	}
	return true;
}

STD_SETTER_CMD_IMPL_F_S_SC(WorksheetElement, SetPositionLogical, QPointF, positionLogical, updatePosition, objectPositionChanged)
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

GRAPHICSITEM_SETTER_CMD_IMPL_F_S(WorksheetElement, SetRotationAngle, qreal, rotation, setRotation, recalcShapeAndBoundingRect)
void WorksheetElement::setRotationAngle(qreal angle) {
	const qreal angle_graphicsItem = -angle;
	Q_D(WorksheetElement);
	if (angle_graphicsItem != d->rotation())
		exec(new WorksheetElementSetRotationAngleCmd(d, angle_graphicsItem, ki18n("%1: set rotation angle")));
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
WorksheetElementPrivate::WorksheetElementPrivate(WorksheetElement* owner)
	: q(owner) {
}

QString WorksheetElementPrivate::name() const {
	return q->name();
}

QRectF WorksheetElementPrivate::boundingRect() const {
	return m_boundingRectangle;
}

QPainterPath WorksheetElementPrivate::shape() const {
	return m_shape;
}

void WorksheetElementPrivate::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {
}

void WorksheetElementPrivate::updatePosition() {
	QPointF p;
	if (coordinateBindingEnabled && q->cSystem) {
		// the position in logical coordinates was changed, calculate the position in scene coordinates
		// insidePlot will get false if the point lies outside of the datarect
		p = q->cSystem->mapLogicalToScene(positionLogical, insidePlot, AbstractCoordinateSystem::MappingFlag::SuppressPageClippingVisible);
		position.point = q->parentPosToRelativePos(mapPlotAreaToParent(p), position);
		Q_EMIT q->positionChanged(position);
	} else {
		insidePlot = true; // not important if within the datarect or not
		p = q->relativePosToParentPos(position);

		// the position in scene coordinates was changed, calculate the position in logical coordinates
		if (q->cSystem && q->cSystem->isValid()) {
			positionLogical = q->cSystem->mapSceneToLogical(mapParentToPlotArea(p), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			Q_EMIT q->positionLogicalChanged(positionLogical);
		}
	}

	p = q->align(p, boundingRect(), horizontalAlignment, verticalAlignment, true);

	suppressItemChangeEvent = true;
	setPos(p);
	suppressItemChangeEvent = false;

	Q_EMIT q->changed();
}

bool WorksheetElementPrivate::sceneEvent(QEvent* event) {
	// don't allow to move the element with the mouse or with the cursor keys if it's locked
	// or if its not movable (ItemIsMovable blocks mouse moves only, we also need to block key press events),
	// react on all other events
	if ((lock && (event->type() == QEvent::GraphicsSceneMouseMove || event->type() == QEvent::KeyPress))
		|| (!flags().testFlag(QGraphicsItem::ItemIsMovable) && event->type() == QEvent::KeyPress)) {
		event->ignore();
		return true;
	}

	return QGraphicsItem::sceneEvent(event);
}

void WorksheetElementPrivate::keyPressEvent(QKeyEvent* event) {
	const bool keyVertical = event->key() == Qt::Key_Up || event->key() == Qt::Key_Down;
	const bool keyHorizontal = event->key() == Qt::Key_Left || event->key() == Qt::Key_Right;
	if ((keyHorizontal && position.positionLimit != WorksheetElement::PositionLimit::Y)
		|| (keyVertical && position.positionLimit != WorksheetElement::PositionLimit::X)) {
		const int delta = 5; // always in scene coordinates

		WorksheetElement::PositionWrapper tempPosition = position;
		if (coordinateBindingEnabled && q->cSystem) {
			if (!q->cSystem->isValid())
				return;
			// the position in logical coordinates was changed, calculate the position in scene coordinates
			bool visible;
			QPointF p = q->cSystem->mapLogicalToScene(positionLogical, visible, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			if (event->key() == Qt::Key_Left) {
				p.setX(p.x() - delta);
			} else if (event->key() == Qt::Key_Right) {
				p.setX(p.x() + delta);
			} else if (event->key() == Qt::Key_Up) {
				p.setY(p.y() - delta); // y-axis is reversed, change the sign here
			} else if (event->key() == Qt::Key_Down) {
				p.setY(p.y() + delta);
			}
			auto pLogic = q->cSystem->mapSceneToLogical(p, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			q->setPositionLogical(pLogic); // So it is undoable
		} else {
			QPointF point = q->parentPosToRelativePos(pos(), position);
			point = q->align(point, m_boundingRectangle, horizontalAlignment, verticalAlignment, false);

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
			q->setPosition(tempPosition); // So it is undoable
		}
		event->accept();
	} else
		QGraphicsItem::keyPressEvent(event);
}

void WorksheetElementPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	// when moving the element with the mouse (left button pressed), the move event doesn't have
	// the information about the pressed button anymore (NoButton) that is needed in mouseMoveEvent()
	// to decide if the element move was started or not. So, we check the pressed buttong here.
	if (event->button() == Qt::LeftButton)
		m_leftButtonPressed = true;

	QGraphicsItem::mousePressEvent(event);
}

void WorksheetElementPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	if (!m_moveStarted && m_leftButtonPressed)
		m_moveStarted = true;

	QGraphicsItem::mouseMoveEvent(event);
}

void WorksheetElementPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	m_leftButtonPressed = false;
	if (!m_moveStarted) {
		QGraphicsItem::mouseReleaseEvent(event);
		return;
	}

	// convert position of the item in parent coordinates to label's position
	QPointF point = q->parentPosToRelativePos(pos(), position);
	point = q->align(point, boundingRect(), horizontalAlignment, verticalAlignment, false);
	if (point != position.point) {
		// position was changed -> set the position related member variables
		suppressRetransform = true;
		auto tempPosition = position;
		tempPosition.point = point;
		q->setPosition(tempPosition);
		updatePosition(); // to update the logical position if available
		suppressRetransform = false;
	}

	m_moveStarted = false;
	QGraphicsItem::mouseReleaseEvent(event);
}

QVariant WorksheetElementPrivate::itemChange(GraphicsItemChange change, const QVariant& value) {
	if (suppressItemChangeEvent)
		return value;

	if (change == QGraphicsItem::ItemPositionChange) {
		auto currPos = pos();
		auto newPos = value.toPointF();
		switch (position.positionLimit) {
		case WorksheetElement::PositionLimit::X:
			newPos.setY(currPos.y());
			break;
		case WorksheetElement::PositionLimit::Y:
			newPos.setX(currPos.x());
			break;
		case WorksheetElement::PositionLimit::None:
		default:
			break;
		}

		// don't use setPosition here, because then all small changes are on the undo stack
		// setPosition is used then in mouseReleaseEvent
		if (coordinateBindingEnabled) {
			if (!q->cSystem->isValid())
				return QGraphicsItem::itemChange(change, value);
			QPointF pos = q->align(newPos, m_boundingRectangle, horizontalAlignment, verticalAlignment, false);

			positionLogical = q->cSystem->mapSceneToLogical(mapParentToPlotArea(pos), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			Q_EMIT q->positionLogicalChanged(positionLogical);
			Q_EMIT q->objectPositionChanged();
		} else {
			// convert item's center point in parent's coordinates
			auto tempPosition = position;
			tempPosition.point = q->parentPosToRelativePos(newPos, position);
			tempPosition.point = q->align(tempPosition.point, boundingRect(), horizontalAlignment, verticalAlignment, false);

			// Q_EMIT the signals in order to notify the UI.
			Q_EMIT q->positionChanged(tempPosition);
			Q_EMIT q->objectPositionChanged();
		}
		return QGraphicsItem::itemChange(change, newPos);
	}

	return QGraphicsItem::itemChange(change, value);
}

/*!
 * \brief WorksheetElementPrivate::mapParentToPlotArea
 * Mapping a point from parent coordinates to plotArea coordinates
 * Needed because in some cases the parent is not the PlotArea, but a child of it (Marker/InfoElement)
 * IMPORTANT: function is also used in Custompoint, so when changing anything, change it also there
 * \param point point in parent coordinates
 * \return point in PlotArea coordinates
 */
QPointF WorksheetElementPrivate::mapParentToPlotArea(QPointF point) const {
	auto* parent = q->parent(AspectType::CartesianPlot);
	if (parent) {
		auto* plot = static_cast<CartesianPlot*>(parent);
		// mapping from parent to item coordinates and them to plot area
		return mapToItem(plot->plotArea()->graphicsItem(), mapFromParent(point));
	}

	return point; // don't map if no parent set. Then it's during load
}

/*!
 * \brief WorksheetElementPrivate::mapPlotAreaToParent
 * Mapping a point from the PlotArea (CartesianPlot::plotArea) coordinates to the parent
 * coordinates of this item
 * Needed because in some cases the parent is not the PlotArea, but a child of it (Marker/InfoElement)
 * IMPORTANT: function is also used in Custompoint, so when changing anything, change it also there
 * \param point point in plotArea coordinates
 * \return point in parent coordinates
 */
QPointF WorksheetElementPrivate::mapPlotAreaToParent(QPointF point) const {
	auto* parent = q->parent(AspectType::CartesianPlot);
	if (parent) {
		auto* plot = static_cast<CartesianPlot*>(parent);
		// first mapping to item coordinates and from there back to parent
		// WorksheetinfoElement: parentItem()->parentItem() == plot->graphicsItem()
		// plot->graphicsItem().pos() == plot->plotArea()->graphicsItem().pos()
		return mapToParent(mapFromItem(plot->plotArea()->graphicsItem(), point));
	}

	return point; // don't map if no parent set. Then it's during load
}

void WorksheetElementPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected())
		setHover(true);
}

void WorksheetElementPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	setHover(false);
}

void WorksheetElementPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	// don't show any context menu if the element is hidden which is the case
	// for example for axis and plot title labels. For such objects the context menu
	// of their parents, i.e. of axis and plot, is used.
	if (!q->isHidden()) {
		auto* menu = q->createContextMenu();
		if (menu)
			menu->exec(event->screenPos());
	}
}

bool WorksheetElementPrivate::isHovered() const {
	return m_hovered;
}

void WorksheetElementPrivate::setHover(bool on) {
	if (on == m_hovered)
		return; // don't update if state not changed

	m_hovered = on;
	Q_EMIT q->hoveredChanged(on);
	update();
}

CartesianPlot* WorksheetElement::plot() const {
	Q_D(const WorksheetElement);
	return d->m_plot;
}
