/***************************************************************************
    File                 : AbstractAspect.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007-2010 by Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2011-2016 by Alexander Semke (alexander.semke@web.de)
    Description          : Base class for all objects in a Project.
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

#include "backend/core/AbstractAspect.h"
#include "backend/core/AspectPrivate.h"
#include "backend/core/AspectFactory.h"
#include "backend/core/aspectcommands.h"
#include "backend/core/Project.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/SignallingUndoCommand.h"
#include "backend/lib/PropertyChangeCommand.h"
#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif

#include <QClipboard>
#include <QMenu>
#include <QMimeData>
#include <KStandardAction>

/**
 * \class AbstractAspect
 * \brief Base class of all persistent objects in a Project.
 *
 * Before going into the details, it's useful to understand the ideas behind the
 * \ref aspect "Aspect Framework".
 *
 * Aspects organize themselves into trees, where a parent takes ownership of its children. Usually,
 * though not necessarily, a Project instance will sit at the root of the tree (without a Project
 * ancestor, project() will return 0 and undo does not work). Children are organized using
 * addChild(), removeChild(), child(), indexOfChild() and childCount() on the parent's side as well
 * as the equivalent convenience methods index() and remove() on the child's side.
 * In contrast to the similar feature of QObject, Aspect trees are fully undo/redo aware and provide
 * signals around object adding/removal.
 *
 * AbstractAspect manages for every Aspect the properties #name, #comment, #captionSpec and
 * #creationTime. All of these translate into the caption() as described in the documentation
 * of setCaptionSpec().
 *
 * If an undoStack() can be found (usually it is managed by Project), changes to the properties
 * as well as adding/removing children support multi-level undo/redo. In order to support undo/redo
 * for problem-specific data in derived classes, make sure that all changes to your data are done
 * by handing appropriate commands to exec().
 */

/**
 * \enum AbstractAspect::ChildIndexFlag
 * \brief Flags which control numbering scheme of children.
 */
/**
 * \var AbstractAspect::IncludeHidden
 * \brief Include aspects marked as "hidden" in numbering or listing children.
 */
/**
 * \var AbstractAspect::Recursive
 * \brief Recursively handle all descendents, not just immediate children.
 */
/**
 * \var AbstractAspect::Compress
 * \brief Remove all null pointers from the result list.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////
// documentation of template and inline methods
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \fn template < class T > T *AbstractAspect::ancestor() const
 * \brief Return the closest ancestor of class T (or NULL if none found).
 */

/**
 * \fn template < class T > QVector<T*> AbstractAspect::children(const ChildIndexFlags &flags=0) const
 * \brief Return list of children inheriting from class T.
 *
 * Use AbstractAspect for T in order to get all children.
 */

/**
 * \fn template < class T > T *AbstractAspect::child(int index, const ChildIndexFlags &flags=0) const
 * \brief Return child identified by (0 based) index and class.
 *
 * Identifying objects by an index is inherently error-prone and confusing,
 * given that the index can be based on different criteria (viz, counting
 * only instances of specific classes and including/excluding hidden
 * aspects). Therefore, it is recommended to avoid indices wherever possible
 * and instead refer to aspects using AbstractAspect pointers.
 */

/**
 * \fn template < class T > T *AbstractAspect::child(const QString &name) const
 * \brief Get child by name and class.
 */

/**
 * \fn template < class T > int AbstractAspect::childCount(const ChildIndexFlags &flags=0) const
 * \brief Return the number of child Aspects inheriting from given class.
 */

/**
 * \fn template < class T > int AbstractAspect::indexOfChild(const AbstractAspect * child, const ChildIndexFlags &flags=0) const
 * \brief Return (0 based) index of child in the list of children inheriting from class T.
 */

/**
 * \fn void AbstractAspect::aspectDescriptionAboutToChange(const AbstractAspect *aspect)
 * \brief Emitted before the name, comment or caption spec is changed
 */

/**
 * \fn void AbstractAspect::aspectDescriptionChanged(const AbstractAspect *aspect)
 * \brief Emitted after the name, comment or caption spec have changed
 */

/**
 * \fn void AbstractAspect::aspectAboutToBeAdded(const AbstractAspect *parent, const AbstractAspect *before, const AbstractAspect * child)
 * \brief Emitted before a new child is inserted
 */

/**
 * \fn void AbstractAspect::aspectAdded(const AbstractAspect *aspect)
 * \brief Emitted after a new Aspect has been added to the tree
 */

/**
 * \fn void AbstractAspect::aspectAboutToBeRemoved(const AbstractAspect *aspect)
 * \brief Emitted before an aspect is removed from its parent
 */

/**
 * \fn void AbstractAspect::aspectRemoved(const AbstractAspect *parent, const AbstractAspect * before, const AbstractAspect * child)
 * \brief Emitted from the parent after removing a child
 */

/**
 * \fn void AbstractAspect::aspectHiddenAboutToChange(const AbstractAspect *aspect)
 * \brief Emitted before the hidden attribute is changed
 */

/**
 * \fn void AbstractAspect::aspectHiddenChanged(const AbstractAspect *aspect)
 * \brief Emitted after the hidden attribute has changed
 */

/**
 * \fn void AbstractAspect::statusInfo(const QString &text)
 * \brief Emitted whenever some aspect in the tree wants to give status information to the user
 * \sa info(const QString&)
 */

/**
 * \fn protected void AbstractAspect::info(const QString &text)
 * \brief Implementations should call this whenever status information should be given to the user.
 *
 * This will cause statusInfo() to be emitted. Typically, this will cause the specified string
 * to be displayed in a status bar, a log window or some similar non-blocking way so as not to
 * disturb the workflow.
 */

/**
 * \fn protected virtual void childSelected(const AbstractAspect*) {}
 * \brief called when a child's child aspect was selected in the model
 */

/**
 * \fn protected virtual void childDeselected()
 * \brief called when a child aspect was deselected in the model
 */

/**
 * \fn protected virtual void childDeselected(const AbstractAspect*)
 * \brief called when a child's child aspect was deselected in the model
 */

////////////////////////////////////////////////////////////////////////////////////////////////////
// start of AbstractAspect implementation
////////////////////////////////////////////////////////////////////////////////////////////////////

AbstractAspect::AbstractAspect(const QString &name, AspectType type)
	: m_type(type), d(new AbstractAspectPrivate(this, name)) {
}

AbstractAspect::~AbstractAspect() {
	delete d;
}

QString AbstractAspect::name() const {
	return d->m_name;
}

/*!
 * \brief AbstractAspect::setName
 * sets the name of the abstract aspect
 * \param value
 * \param autoUnique
 * \return returns, if the new name is valid or not
 */
bool AbstractAspect::setName(const QString &value, bool autoUnique) {
	if (value.isEmpty())
		return setName(QLatin1String("1"), autoUnique);

	if (value == d->m_name)
		return true; // name not changed, but the name is valid

	QString new_name;
	if (d->m_parent) {
		new_name = d->m_parent->uniqueNameFor(value);

		if (!autoUnique && new_name.compare(value) != 0) // value is not unique, so don't change name
			return false; // this value is used in the dock to check if the name is valid


		if (new_name != value)
			info(i18n(R"(Intended name "%1" was changed to "%2" in order to avoid name collision.)", value, new_name));
	} else
		new_name = value;

	exec(new PropertyChangeCommand<QString>(i18n("%1: rename to %2", d->m_name, new_name),
				&d->m_name, new_name),
			"aspectDescriptionAboutToChange", "aspectDescriptionChanged", Q_ARG(const AbstractAspect*,this));
	return true;
}

QString AbstractAspect::comment() const {
	return d->m_comment;
}

void AbstractAspect::setComment(const QString& value) {
	if (value == d->m_comment) return;
	exec(new PropertyChangeCommand<QString>(i18n("%1: change comment", d->m_name),
				&d->m_comment, value),
			"aspectDescriptionAboutToChange", "aspectDescriptionChanged", Q_ARG(const AbstractAspect*,this));
}

void AbstractAspect::setCreationTime(const QDateTime& time) {
	d->m_creation_time = time;
}

QDateTime AbstractAspect::creationTime() const {
	return d->m_creation_time;
}

bool AbstractAspect::hidden() const {
	return d->m_hidden;
}

/**
 * \brief Set "hidden" property, i.e. whether to exclude this aspect from being shown in the explorer.
 */
void AbstractAspect::setHidden(bool value) {
	if (value == d->m_hidden)
		return;
	d->m_hidden = value;
}

void AbstractAspect::setIsLoading(bool load) {
	d->m_isLoading = load;
}

bool AbstractAspect::isLoading() const {
	return d->m_isLoading;
}

/**
 * \brief Return an icon to be used for decorating my views.
 */
QIcon AbstractAspect::icon() const {
	return QIcon();
}

/**
 * \brief Return a new context menu.
 *
 * The caller takes ownership of the menu.
 */
QMenu* AbstractAspect::createContextMenu() {
	QMenu* menu = new QMenu();
	menu->addSection(this->name());

	//TODO: activate this again when the functionality is implemented
// 	menu->addAction( KStandardAction::cut(this) );

	if (this != project()) {
		auto* action = KStandardAction::copy(this);
		connect(action, &QAction::triggered, this, &AbstractAspect::copy);
		menu->addAction(action);

		if (m_type != AspectType::CartesianPlotLegend) {
			auto* actionDuplicate = new QAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("Duplicate Here"), this);
			actionDuplicate->setShortcut(Qt::CTRL + Qt::Key_D);
			connect(actionDuplicate, &QAction::triggered, this, &AbstractAspect::duplicate);
			menu->addAction(actionDuplicate);
		}
	}

	//determine the aspect type of the content available in the clipboard
	//and enable the paste entry if the content is labplot specific
	//and if it can be pasted into the current aspect
	QString name;
	auto t = clipboardAspectType(name);
	if (t != AspectType::AbstractAspect && pasteTypes().indexOf(t) != -1) {
		auto* action = KStandardAction::paste(this);
		action->setText(i18n("Paste '%1'", name));
		menu->addAction(action);
		connect(action, &QAction::triggered, this, &AbstractAspect::paste);
	}
	menu->addSeparator();

	//don't allow to rename and delete
	// - data spreadsheets of datapicker curves
	// - columns in data spreadsheets of datapicker curves
	// - columns in live-data source
	// - Mqtt subscriptions
	// - Mqtt topics
	// - Columns in Mqtt topics
	bool disabled = (type() == AspectType::Spreadsheet && parentAspect()->type() == AspectType::DatapickerCurve)
		|| (type() == AspectType::Column && parentAspect()->parentAspect() && parentAspect()->parentAspect()->type() == AspectType::DatapickerCurve)
		|| (type() == AspectType::Column && parentAspect()->type() == AspectType::LiveDataSource)
#ifdef HAVE_MQTT
		|| (type() == AspectType::MQTTSubscription)
		|| (type() == AspectType::MQTTTopic)
		| (type() == AspectType::Column && parentAspect()->type() == AspectType::MQTTTopic)
#endif
		|| (type() == AspectType::CustomPoint && parentAspect()->type() == AspectType::InfoElement)
		;

	if(!disabled) {
		menu->addAction(QIcon::fromTheme(QLatin1String("edit-rename")), i18n("Rename"), this, SIGNAL(renameRequested()));
		if (type() != AspectType::Project)
			menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete"), this, SLOT(remove()));
	}

	return menu;
}

AspectType AbstractAspect::type() const {
	return m_type;
}

bool AbstractAspect::inherits(AspectType type) const {
	return (static_cast<quint64>(m_type) & static_cast<quint64>(type)) == static_cast<quint64>(type);
}

/**
 * \brief In the parent-child hierarchy, return the first parent of type \param type or null pointer if there is none.
 */
AbstractAspect* AbstractAspect::parent(AspectType type) const {
	AbstractAspect* parent = parentAspect();
	if (!parent)
		return nullptr;

	if (parent->inherits(type))
		return parent;

	return parent->parent(type);
}

/**
 * \brief Return my parent Aspect or 0 if I currently don't have one.
 */
AbstractAspect* AbstractAspect::parentAspect() const {
	return d->m_parent;
}

void AbstractAspect::setParentAspect(AbstractAspect* parent) {
	d->m_parent = parent;
}

/**
 * \brief Return the folder the Aspect is contained in or 0 if there is none.
 *
 * The returned folder may be the aspect itself if it inherits Folder.
 */
Folder* AbstractAspect::folder() {
	if (inherits(AspectType::Folder)) return static_cast<class Folder*>(this);
	AbstractAspect* parent_aspect = parentAspect();
	while (parent_aspect && !parent_aspect->inherits(AspectType::Folder))
		parent_aspect = parent_aspect->parentAspect();
	return static_cast<class Folder*>(parent_aspect);
}

/**
 * \brief Return whether the there is a path upwards to the given aspect
 *
 * This also returns true if other==this.
 */
bool AbstractAspect::isDescendantOf(AbstractAspect* other) {
	if (other == this) return true;
	AbstractAspect* parent_aspect = parentAspect();
	while (parent_aspect) {
		if (parent_aspect == other) return true;
		parent_aspect = parent_aspect->parentAspect();
	}
	return false;
}

/**
 * \brief Return the Project this Aspect belongs to, or 0 if it is currently not part of one.
 */
Project* AbstractAspect::project() {
	return parentAspect() ? parentAspect()->project() : nullptr;
}

/**
 * \brief Return the path that leads from the top-most Aspect (usually a Project) to me.
 */
QString AbstractAspect::path() const {
	return parentAspect() ? parentAspect()->path() + QLatin1Char('/') + name() : QString();
}

/**
 * \brief Add the given Aspect to my list of children.
 */
void AbstractAspect::addChild(AbstractAspect* child) {
	Q_CHECK_PTR(child);

	QString new_name = uniqueNameFor(child->name());
	beginMacro(i18n("%1: add %2", name(), new_name));
	if (new_name != child->name()) {
		info(i18n(R"(Renaming "%1" to "%2" in order to avoid name collision.)", child->name(), new_name));
		child->setName(new_name);
	}

	exec(new AspectChildAddCmd(d, child, d->m_children.count()));
	child->finalizeAdd();
	endMacro();
}

/**
 * \brief Add the given Aspect to my list of children without any checks and without putting this step onto the undo-stack
 */
void AbstractAspect::addChildFast(AbstractAspect* child) {
	emit aspectAboutToBeAdded(this, nullptr, child); //TODO: before-pointer is 0 here, also in the commands classes. why?
	d->insertChild(d->m_children.count(), child);
	child->finalizeAdd();
	emit aspectAdded(child);
}

/**
 * \brief Insert the given Aspect at a specific position in my list of children.
 */
void AbstractAspect::insertChildBefore(AbstractAspect* child, AbstractAspect* before) {
	Q_CHECK_PTR(child);

	QString new_name = uniqueNameFor(child->name());
	beginMacro(before ? i18n("%1: insert %2 before %3", name(), new_name, before->name()) : i18n("%1: insert %2 before end", name(), new_name));
	if (new_name != child->name()) {
		info(i18n(R"(Renaming "%1" to "%2" in order to avoid name collision.)", child->name(), new_name));
		child->setName(new_name);
	}
	int index = d->indexOfChild(before);
	if (index == -1)
		index = d->m_children.count();

	exec(new AspectChildAddCmd(d, child, index));
	endMacro();
}

/**
 * \brief Insert the given Aspect at a specific position in my list of children.without any checks and without putting this step onto the undo-stack
 */
void AbstractAspect::insertChildBeforeFast(AbstractAspect* child, AbstractAspect* before) {
	connect(child, &AbstractAspect::selected, this, &AbstractAspect::childSelected);
	connect(child, &AbstractAspect::deselected, this, &AbstractAspect::childDeselected);

	int index = d->indexOfChild(before);
	if (index == -1)
		index = d->m_children.count();

	emit aspectAboutToBeAdded(this, nullptr, child);
	d->insertChild(index, child);
	emit aspectAdded(child);
}

/**
 * \brief Remove the given Aspect from my list of children.
 *
 * The ownership of the child is transferred to the undo command,
 * i.e., the aspect is deleted by the undo command.
 * \sa reparent()
 */
void AbstractAspect::removeChild(AbstractAspect* child) {
	Q_ASSERT(child->parentAspect() == this);

	//when the child being removed is a LiveDataSource or a MQTT client,
	//stop reading from the source before removing the child from the project
	if (child->type() == AspectType::LiveDataSource)
		static_cast<LiveDataSource*>(child)->pauseReading();
#ifdef HAVE_MQTT
	else if (child->type() == AspectType::MQTTClient)
		static_cast<MQTTClient*>(child)->pauseReading();
#endif

	beginMacro(i18n("%1: remove %2", name(), child->name()));
	exec(new AspectChildRemoveCmd(d, child));
	endMacro();
}

/**
 * \brief Remove all child Aspects.
 */
void AbstractAspect::removeAllChildren() {
	beginMacro(i18n("%1: remove all children", name()));

	QVector<AbstractAspect*> children_list = children();
	QVector<AbstractAspect*>::const_iterator i = children_list.constBegin();
	AbstractAspect *current = nullptr, *nextSibling = nullptr;
	if (i != children_list.constEnd()) {
		current = *i;
		if (++i != children_list.constEnd())
			nextSibling = *i;
	}

	while (current) {
		emit aspectAboutToBeRemoved(current);
		exec(new AspectChildRemoveCmd(d, current));
		emit aspectRemoved(this, nextSibling, current);

		current = nextSibling;
		if (i != children_list.constEnd() && ++i != children_list.constEnd())
			nextSibling = *i;
		else
			nextSibling = nullptr;
	}

	endMacro();
}

/**
 * \brief Move a child to another parent aspect and transfer ownership.
 */
void AbstractAspect::reparent(AbstractAspect* newParent, int newIndex) {
	Q_ASSERT(parentAspect());
	Q_ASSERT(newParent);
	int max_index = newParent->childCount<AbstractAspect>(ChildIndexFlag::IncludeHidden);
	if (newIndex == -1)
		newIndex = max_index;
	Q_ASSERT(newIndex >= 0 && newIndex <= max_index);

//	AbstractAspect* old_parent = parentAspect();
// 	int old_index = old_parent->indexOfChild<AbstractAspect>(this, IncludeHidden);
// 	auto* old_sibling = old_parent->child<AbstractAspect>(old_index+1, IncludeHidden);
// 	auto* new_sibling = newParent->child<AbstractAspect>(newIndex, IncludeHidden);

// 	emit newParent->aspectAboutToBeAdded(newParent, new_sibling, this);
	exec(new AspectChildReparentCmd(parentAspect()->d, newParent->d, this, newIndex));
// 	emit old_parent->aspectRemoved(old_parent, old_sibling, this);
}

QVector<AbstractAspect*> AbstractAspect::children(AspectType type, ChildIndexFlags flags) const {
	QVector<AbstractAspect*> result;
	for (auto* child : children()) {
		if (flags & ChildIndexFlag::IncludeHidden || !child->hidden()) {
			if (child->inherits(type) || !(flags & ChildIndexFlag::Compress)) {
				result << child;
				if (flags & ChildIndexFlag::Recursive) {
					result << child->children(type, flags);
				}
			}
		}
	}
	return result;
}

const QVector<AbstractAspect*>& AbstractAspect::children() const {
	Q_ASSERT(d);
	return d->m_children;
}

/**
 * \brief Remove me from my parent's list of children.
 */
void AbstractAspect::remove() {
	if (parentAspect())
		parentAspect()->removeChild(this);
}

/*!
 * returns the list of all parent aspects (folders and sub-folders)
 */
QVector<AbstractAspect*> AbstractAspect::dependsOn() const {
	QVector<AbstractAspect*> aspects;
	if (parentAspect())
		aspects << parentAspect() << parentAspect()->dependsOn();

	return aspects;
}

/*!
 * return the list of all aspect types that can be copy&pasted into the current aspect.
 * returns an empty list on default, needs to be re-implemented in all derived classes
 * that want to allow other aspects to be pasted into.
 */
QVector<AspectType> AbstractAspect::pasteTypes() const {
	return QVector<AspectType>();
}

/*!
 * copies the aspect to the clipboard. The standard XML-serialization
 * via AbstractAspect::load() is used.
 */
void AbstractAspect::copy() const {
	QString output;
	QXmlStreamWriter writer(&output);
	writer.writeStartDocument();

	//add LabPlot's copy&paste "identifier"
	writer.writeDTD(QLatin1String("<!DOCTYPE LabPlotCopyPasteXML>"));
	writer.writeStartElement("copy_content"); //root element

	//write the type of the copied aspect
	writer.writeStartElement(QLatin1String("type"));
	writer.writeAttribute(QLatin1String("value"), QString::number(static_cast<int>(m_type)));
	writer.writeEndElement();

	//write the aspect itself
	save(&writer);

	writer.writeEndElement(); //end the root-element
	writer.writeEndDocument();
	QApplication::clipboard()->setText(output);
}

void AbstractAspect::duplicate() {
	copy();
	parentAspect()->paste(true);
}

/*!
 * in case the clipboard containts a LabPlot's specific copy&paste content,
 * this function deserializes the XML string and adds the created aspect as
 * a child to the current aspect ("paste").
 */
void AbstractAspect::paste(bool duplicate) {
	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	if (!mimeData->hasText())
		return;

	const QString& xml = clipboard->text();
	if (!xml.startsWith(QLatin1String("<?xml version=\"1.0\"?><!DOCTYPE LabPlotCopyPasteXML>")))
		return;

	WAIT_CURSOR;
	AbstractAspect* aspect = nullptr;
	XmlStreamReader reader(xml);
	while (!reader.atEnd()) {
		reader.readNext();

		if (!reader.isStartElement())
			continue;

		if (reader.name() == QLatin1String("type")) {
			auto attribs = reader.attributes();
			auto type = static_cast<AspectType>(attribs.value(QLatin1String("value")).toInt());
			if (type != AspectType::AbstractAspect)
				aspect = AspectFactory::createAspect(type, this);
		} else {
			if (aspect) {
				aspect->load(&reader, false);
				break;
			}
		}
	}

	if (aspect) {
		if (!duplicate)
			beginMacro(i18n("%1: pasted '%2'", name(), aspect->name()));
		else {
			beginMacro(i18n("%1: duplicated '%2'", name(), aspect->name()));
			aspect->setName(i18n("Copy of '%1'", aspect->name()));
		}

		if (aspect->type() != AspectType::CartesianPlotLegend)
			addChild(aspect);
		else {
			//spectial handling for the legend since only one single
			//legend object is allowed per plot
			auto* plot = static_cast<CartesianPlot*>(this);
			auto* legend = static_cast<CartesianPlotLegend*>(aspect);
			plot->addLegend(legend);
		}

		project()->restorePointers(aspect);
		endMacro();
	}
	RESET_CURSOR;
}

/*!
 * helper function determening whether the current content of the clipboard
 * contants the labplot specific copy&paste XML content. In case a valid content
 * is available, the aspect type of the object to be pasted is returned.
 * AspectType::AbstractAspect is returned otherwise.
 */
AspectType AbstractAspect::clipboardAspectType(QString& name) {
	AspectType type = AspectType::AbstractAspect;
	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	if (!mimeData->hasText())
		return type;

	const QString& xml = clipboard->text();
	if (!xml.startsWith(QLatin1String("<?xml version=\"1.0\"?><!DOCTYPE LabPlotCopyPasteXML>")))
		return type;

	XmlStreamReader reader(xml);
	bool typeFound = false;
	while (!reader.atEnd()) {
		reader.readNext();
		if (reader.isStartElement()) {
			auto attribs = reader.attributes();
			if (reader.name() == QLatin1String("type")) {
				type = static_cast<AspectType>(attribs.value(QLatin1String("value")).toInt());
				typeFound = true;
			} else {
				name = attribs.value(QLatin1String("name")).toString();
				if (typeFound)
					break;
			}
		}
	}

	return type;
}

bool AbstractAspect::isDraggable() const {
	return false;
}

QVector<AspectType> AbstractAspect::dropableOn() const {
	return QVector<AspectType>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! \name serialize/deserialize
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \fn virtual void AbstractAspect::save(QXmlStreamWriter *) const
 * \brief Save as XML
 */

/**
 * \fn virtual bool AbstractAspect::load(XmlStreamReader *)
 * \brief Load from XML
 *
 * XmlStreamReader supports errors as well as warnings. If only
 * warnings (non-critical errors) occur, this function must return
 * the reader at the end element corresponding to the current
 * element at the time the function was called.
 *
 * This function is normally intended to be called directly
 * after the ctor. If you want to call load on an aspect that
 * has been altered, you must make sure beforehand that
 * it is in the same state as after creation, e.g., remove
 * all its child aspects.
 *
 * \return false on error
 */

/**
 * \brief Save the comment to XML
 */
void AbstractAspect::writeCommentElement(QXmlStreamWriter * writer) const{
	writer->writeStartElement(QLatin1String("comment"));
	writer->writeCharacters(comment());
	writer->writeEndElement();
}

/**
 * \brief Load comment from an XML element
 */
bool AbstractAspect::readCommentElement(XmlStreamReader * reader) {
	setComment(reader->readElementText());
	return true;
}

/**
 * \brief Save name and creation time to XML
 */
void AbstractAspect::writeBasicAttributes(QXmlStreamWriter* writer) const {
	writer->writeAttribute(QLatin1String("creation_time") , creationTime().toString(QLatin1String("yyyy-dd-MM hh:mm:ss:zzz")));
	writer->writeAttribute(QLatin1String("name"), name());
}

/**
 * \brief Load name and creation time from XML
 *
 * \return false on error
 */
bool AbstractAspect::readBasicAttributes(XmlStreamReader* reader) {
	const QXmlStreamAttributes& attribs = reader->attributes();

	// name
	QString str = attribs.value(QLatin1String("name")).toString();
	if (str.isEmpty())
		reader->raiseWarning(i18n("Attribute 'name' is missing or empty."));

	d->m_name = str;

	// creation time
	str = attribs.value(QLatin1String("creation_time")).toString();
	if (str.isEmpty()) {
		reader->raiseWarning(i18n("Invalid creation time for '%1'. Using current time.", name()));
		d->m_creation_time = QDateTime::currentDateTime();
	} else {
		QDateTime creation_time = QDateTime::fromString(str, QLatin1String("yyyy-dd-MM hh:mm:ss:zzz"));
		if (creation_time.isValid())
			d->m_creation_time = creation_time;
		else
			d->m_creation_time = QDateTime::currentDateTime();
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
//! \name undo related
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractAspect::setUndoAware(bool b) {
	d->m_undoAware = b;
}

/**
 * \brief Return the undo stack of the Project, or 0 if this Aspect is not part of a Project.
 *
 * It's also possible to construct undo-enabled Aspect trees without Project.
 * The only requirement is that the root Aspect reimplements undoStack() to get the
 * undo stack from somewhere (the default implementation just delegates to parentAspect()).
 */
QUndoStack* AbstractAspect::undoStack() const {
	return parentAspect() ? parentAspect()->undoStack() : nullptr;
}

/**
 * \brief Execute the given command, pushing it on the undoStack() if available.
 */
void AbstractAspect::exec(QUndoCommand* cmd) {
	Q_CHECK_PTR(cmd);
	if (d->m_undoAware) {
		QUndoStack *stack = undoStack();
		if (stack)
			stack->push(cmd);
		else {
			cmd->redo();
			delete cmd;
		}

		if (project())
			project()->setChanged(true);
	} else {
		cmd->redo();
		delete cmd;
	}
}

/**
 * \brief Execute command and arrange for signals to be sent before/after it is redone or undone.
 *
 * \arg \c command The command to be executed.
 * \arg \c preChangeSignal The name of the signal to be triggered before re-/undoing the command.
 * \arg \c postChangeSignal The name of the signal to be triggered after re-/undoing the command.
 * \arg <tt>val0,val1,val2,val3</tt> Arguments to the signals; to be given using Q_ARG().
 *
 * Signal arguments are given using the macro Q_ARG(typename, const value&). Since
 * the variable given as "value" will likely be out of scope when the signals are emitted, a copy
 * needs to be created. This uses QMetaType, which means that (non-trivial) argument types need to
 * be registered using qRegisterMetaType() before giving them to exec() (in particular, this also
 * goes for pointers to custom data types).
 *
 * \sa SignallingUndoCommand
 */
void AbstractAspect::exec(QUndoCommand* command,
		const char* preChangeSignal, const char* postChangeSignal,
		QGenericArgument val0, QGenericArgument val1, QGenericArgument val2, QGenericArgument val3) {
	beginMacro(command->text());
	exec(new SignallingUndoCommand(QLatin1String("change signal"), this,
				preChangeSignal, postChangeSignal, val0, val1, val2, val3));
	exec(command);
	exec(new SignallingUndoCommand(QLatin1String("change signal"), this,
				postChangeSignal, preChangeSignal, val0, val1, val2, val3));
	endMacro();
}

/**
 * \brief Begin an undo stack macro (series of commands)
 */
void AbstractAspect::beginMacro(const QString& text) {
	if (!d->m_undoAware)
		return;

	QUndoStack* stack = undoStack();
	if (stack)
		stack->beginMacro(text);
}

/**
 * \brief End the current undo stack macro
 */
void AbstractAspect::endMacro() {
	if (!d->m_undoAware)
		return;

	QUndoStack* stack = undoStack();
	if (stack)
		stack->endMacro();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////////////////////////


/*!
 * this function is called when the selection in ProjectExplorer was changed.
 * forwards the selection/deselection to the parent aspect via emitting a signal.
 */
void AbstractAspect::setSelected(bool s) {
	if (s)
		emit selected(this);
	else
		emit deselected(this);
}

void AbstractAspect::childSelected(const AbstractAspect* aspect) {
	//forward the signal to the highest possible level in the parent-child hierarchy
	//e.g. axis of a plot was selected. Don't include parent aspects here that do not
	//need to react on the selection of children:
	//* Folder
	//* XYFitCurve with the child column for calculated residuals
	//* XYSmouthCurve with the child column for calculated rough values
	//* CantorWorksheet with the child columns for CAS variables
    AbstractAspect* parent = this->parentAspect();
	if (parent
		&& !parent->inherits(AspectType::Folder)
		&& !parent->inherits(AspectType::XYFitCurve)
		&& !parent->inherits(AspectType::XYSmoothCurve)
		&& !parent->inherits(AspectType::CantorWorksheet))
		emit this->selected(aspect);
}

void AbstractAspect::childDeselected(const AbstractAspect* aspect) {
	//forward the signal to the highest possible level in the parent-child hierarchy
	//e.g. axis of a plot was selected. Don't include parent aspects here that do not
	//need to react on the deselection of children:
	//* Folder
	//* XYFitCurve with the child column for calculated residuals
	//* XYSmouthCurve with the child column for calculated rough values
	//* CantorWorksheet with the child columns for CAS variables
    AbstractAspect* parent = this->parentAspect();
	if (parent
		&& !parent->inherits(AspectType::Folder)
		&& !parent->inherits(AspectType::XYFitCurve)
		&& !parent->inherits(AspectType::XYSmoothCurve)
		&& !parent->inherits(AspectType::CantorWorksheet))
		emit this->deselected(aspect);
}

/**
 * \brief Make the specified name unique among my children by incrementing a trailing number.
 */
QString AbstractAspect::uniqueNameFor(const QString& current_name) const {
	QStringList child_names;
	for (auto* child : children())
		child_names << child->name();

	if (!child_names.contains(current_name))
		return current_name;

	QString base = current_name;
	int last_non_digit;
	for (last_non_digit = base.size() - 1; last_non_digit >= 0; --last_non_digit) {
		if (base[last_non_digit].category() == QChar::Number_DecimalDigit) {
			base.chop(1);
		} else {
			if (base[last_non_digit].category() == QChar::Separator_Space)
				break;
			else {
				//non-digit character is found and it's not the separator,
				//the string either doesn't have any digits at all or is of
				//the form "data_2020.06". In this case we don't use anything
				//from the original name to increment the number
				last_non_digit = 0;
				base = current_name;
				break;
			}
		}
	}

	if (last_non_digit >=0 && base[last_non_digit].category() != QChar::Separator_Space)
		base.append(" ");

	int new_nr = current_name.rightRef(current_name.size() - base.size()).toInt();
	QString new_name;
	do
		new_name = base + QString::number(++new_nr);
	while (child_names.contains(new_name));

	return new_name;
}

void AbstractAspect::connectChild(AbstractAspect* child) {
	connect(child, &AbstractAspect::aspectDescriptionAboutToChange, this, &AbstractAspect::aspectDescriptionAboutToChange);
	connect(child, &AbstractAspect::aspectDescriptionChanged, this, &AbstractAspect::aspectDescriptionChanged);
	connect(child, &AbstractAspect::aspectAboutToBeAdded, this, &AbstractAspect::aspectAboutToBeAdded);
	connect(child, &AbstractAspect::aspectAdded, this, &AbstractAspect::aspectAdded);
	connect(child, &AbstractAspect::aspectAboutToBeRemoved, this, &AbstractAspect::aspectAboutToBeRemoved);
	connect(child, &AbstractAspect::aspectRemoved, this, &AbstractAspect::aspectRemoved);
	connect(child, &AbstractAspect::aspectHiddenAboutToChange, this, &AbstractAspect::aspectHiddenAboutToChange);
	connect(child, &AbstractAspect::aspectHiddenChanged, this, &AbstractAspect::aspectHiddenChanged);
	connect(child, &AbstractAspect::statusInfo, this, &AbstractAspect::statusInfo);

	connect(child, &AbstractAspect::selected, this, &AbstractAspect::childSelected);
	connect(child, &AbstractAspect::deselected, this, &AbstractAspect::childDeselected);
}

//##############################################################################
//######################  Private implementation ###############################
//##############################################################################
AbstractAspectPrivate::AbstractAspectPrivate(AbstractAspect* owner, const QString& name)
	: m_name(name.isEmpty() ? QLatin1String("1") : name), q(owner) {
	m_creation_time = QDateTime::currentDateTime();
}

AbstractAspectPrivate::~AbstractAspectPrivate() {
	for (auto* child : m_children)
		delete child;
}

void AbstractAspectPrivate::insertChild(int index, AbstractAspect* child) {
	m_children.insert(index, child);

	// Always remove from any previous parent before adding to a new one!
	// Can't handle this case here since two undo commands have to be created.
	Q_ASSERT(child->parentAspect() == nullptr);
	child->setParentAspect(q);
	q->connectChild(child);
}

int AbstractAspectPrivate::indexOfChild(const AbstractAspect* child) const {
	for (int i = 0; i < m_children.size(); ++i)
		if (m_children.at(i) == child) return i;

	return -1;
}

int AbstractAspectPrivate::removeChild(AbstractAspect* child) {
	int index = indexOfChild(child);
	Q_ASSERT(index != -1);
	m_children.removeAll(child);
	QObject::disconnect(child, nullptr, q, nullptr);
	child->setParentAspect(nullptr);
	return index;
}
