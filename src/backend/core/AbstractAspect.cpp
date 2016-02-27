/***************************************************************************
    File                 : AbstractAspect.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007-2010 by Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2011-2015 by Alexander Semke (alexander.semke@web.de)
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
#include "backend/core/aspectcommands.h"
#include "backend/core/Project.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/SignallingUndoCommand.h"
#include "backend/lib/PropertyChangeCommand.h"

#include <QIcon>
#include <QMenu>
#include <QStyle>
#include <QApplication>
#include <QXmlStreamWriter>

#include <KAction>
#include <KStandardAction>
#include <QMenu>

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
 *
 * Optionally,
 * you can supply an icon() to be used by different views (including the ProjectExplorer)
 * and/or reimplement createContextMenu() for a custom context menu of views.
 *
 * The private data of AbstractAspect is contained in a separate class AbstractAspect::Private.
 * The write access to AbstractAspect::Private should always be done using aspect commands
 * to allow undo/redo.
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
 * \fn template < class T > QList<T*> AbstractAspect::children(const ChildIndexFlags &flags=0) const
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
 * \fn protected virtual void childSelected(const AbstractAspect*){}
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

AbstractAspect::AbstractAspect(const QString &name)
	: m_aspect_private(new Private(this, name)), m_undoAware(true)
{
}

AbstractAspect::~AbstractAspect() {
	delete m_aspect_private;
}

QString AbstractAspect::name() const {
	return m_aspect_private->m_name;
}

void AbstractAspect::setName(const QString &value) {
	if (value.isEmpty()) {
		setName("1");
		return;
	}

	if (value == m_aspect_private->m_name)
		return;

	QString new_name;
	if (m_aspect_private->m_parent) {
		new_name = m_aspect_private->m_parent->uniqueNameFor(value);
		if (new_name != value)
			info(i18n("Intended name \"%1\" was changed to \"%2\" in order to avoid name collision.", value, new_name));
	} else {
		new_name = value;
	}

	exec(new PropertyChangeCommand<QString>(i18n("%1: rename to %2", m_aspect_private->m_name, new_name),
				&m_aspect_private->m_name, new_name),
			"aspectDescriptionAboutToChange", "aspectDescriptionChanged", Q_ARG(const AbstractAspect*,this));
}

QString AbstractAspect::comment() const {
	return m_aspect_private->m_comment;
}

void AbstractAspect::setComment(const QString& value) {
	if (value == m_aspect_private->m_comment) return;
	exec(new PropertyChangeCommand<QString>(i18n("%1: change comment", m_aspect_private->m_name),
				&m_aspect_private->m_comment, value),
			"aspectDescriptionAboutToChange", "aspectDescriptionChanged", Q_ARG(const AbstractAspect*,this));
}

/**
 * \brief Set the creation time
 *
 * The creation time will automatically be set when the aspect object
 * is created. This function is usually only needed when the aspect
 * is loaded from a file.
 */
void AbstractAspect::setCreationTime(const QDateTime& time) {
	if (time == m_aspect_private->m_creation_time) return;
	exec(new PropertyChangeCommand<QDateTime>(i18n("%1: set creation time", m_aspect_private->m_name),
				&m_aspect_private->m_creation_time, time));
}

QDateTime AbstractAspect::creationTime() const {
	return m_aspect_private->m_creation_time;
}

bool AbstractAspect::hidden() const {
    return m_aspect_private->m_hidden;
}

/**
 * \brief Set "hidden" property, i.e. whether to exclude this aspect from being shown in the explorer.
 */
void AbstractAspect::setHidden(bool value) {
    if (value == m_aspect_private->m_hidden) return;
    exec(new PropertyChangeCommand<bool>(i18n("%1: change hidden status", m_aspect_private->m_name),
				 &m_aspect_private->m_hidden, value),
			"aspectHiddenAboutToChange", "aspectHiddenChanged", Q_ARG(const AbstractAspect*,this));
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
// 	menu->addAction(KStandardAction::copy(this));
// 	menu->addAction(KStandardAction::paste(this));
// 	menu->addSeparator();
    menu->addAction(QIcon::fromTheme("edit-rename"), i18n("Rename"), this, SIGNAL(renameRequested()));

	//don't allow to delete data spreadsheets in the datapicker curves
    if ( !(dynamic_cast<const Spreadsheet*>(this) && dynamic_cast<const DatapickerCurve*>(this->parentAspect())) )
		menu->addAction(QIcon::fromTheme("edit-delete"), i18n("Delete"), this, SLOT(remove()));

	return menu;
}

/**
 * \brief Return my parent Aspect or 0 if I currently don't have one.
 */
AbstractAspect * AbstractAspect::parentAspect() const {
	return m_aspect_private->m_parent;
}

/**
 * \brief Return the folder the Aspect is contained in or 0 if there is none.
 *
 * The returned folder may be the aspect itself if it inherits Folder.
 */
Folder* AbstractAspect::folder() {
	if(inherits("Folder")) return static_cast<Folder*>(this);
	AbstractAspect* parent_aspect = parentAspect();
	while(parent_aspect && !parent_aspect->inherits("Folder"))
		parent_aspect = parent_aspect->parentAspect();
	return static_cast<Folder*>(parent_aspect);
}

/**
 * \brief Return whether the there is a path upwards to the given aspect
 *
 * This also returns true if other==this.
 */
bool AbstractAspect::isDescendantOf(AbstractAspect* other) {
	if(other == this) return true;
	AbstractAspect* parent_aspect = parentAspect();
	while(parent_aspect) 	{
		if(parent_aspect == other) return true;
		parent_aspect = parent_aspect->parentAspect();
	}
	return false;
}

/**
 * \brief Return the Project this Aspect belongs to, or 0 if it is currently not part of one.
 */
Project* AbstractAspect::project() {
	return parentAspect() ? parentAspect()->project() : 0;
}

/**
 * \brief Return the path that leads from the top-most Aspect (usually a Project) to me.
 */
QString AbstractAspect::path() const {
	return parentAspect() ? parentAspect()->path() + '/' + name() : "";
}

/**
 * \brief Add the given Aspect to my list of children.
 */
void AbstractAspect::addChild(AbstractAspect* child) {
	Q_CHECK_PTR(child);

	QString new_name = m_aspect_private->uniqueNameFor(child->name());
	beginMacro(i18n("%1: add %2.", name(), new_name));
	if (new_name != child->name()) {
		info(i18n("Renaming \"%1\" to \"%2\" in order to avoid name collision.", child->name(), new_name));
		child->setName(new_name);
	}

	exec(new AspectChildAddCmd(m_aspect_private, child, m_aspect_private->m_children.count()));
	endMacro();
}

/**
 * \brief Add the given Aspect to my list of children without any checks and without putting this step onto the undo-stack
 */
void AbstractAspect::addChildFast(AbstractAspect* child) {
	m_aspect_private->insertChild(m_aspect_private->m_children.count(), child);
}

/**
 * \brief Insert the given Aspect at a specific position in my list of children.
 */
void AbstractAspect::insertChildBefore(AbstractAspect* child, AbstractAspect* before) {
	Q_CHECK_PTR(child);

	QString new_name = m_aspect_private->uniqueNameFor(child->name());
	beginMacro(i18n("%1: insert %2 before %3.", name(), new_name, before ? before->name() : "end"));
	if (new_name != child->name()) {
		info(i18n("Renaming \"%1\" to \"%2\" in order to avoid name collision.", child->name(), new_name));
		child->setName(new_name);
	}
	int index = m_aspect_private->indexOfChild(before);
	if (index == -1)
		index = m_aspect_private->m_children.count();

	exec(new AspectChildAddCmd(m_aspect_private, child, index));
	endMacro();
}

/**
 * \brief Insert the given Aspect at a specific position in my list of children.without any checks and without putting this step onto the undo-stack
 */
void AbstractAspect::insertChildBeforeFast(AbstractAspect* child, AbstractAspect* before) {
	connect(child, SIGNAL(selected(const AbstractAspect*)), this, SLOT(childSelected(const AbstractAspect*)));
	connect(child, SIGNAL(deselected(const AbstractAspect*)), this, SLOT(childDeselected(const AbstractAspect*)));

	int index = m_aspect_private->indexOfChild(before);
	if (index == -1)
		index = m_aspect_private->m_children.count();
	m_aspect_private->insertChild(index, child);
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
	beginMacro(i18n("%1: remove %2.", name(), child->name()));
	exec(new AspectChildRemoveCmd(m_aspect_private, child));
	endMacro();
}

/**
 * \brief Remove all child Aspects.
 */
void AbstractAspect::removeAllChildren() {
	beginMacro(i18n("%1: remove all children.", name()));

	QList<AbstractAspect*> children = rawChildren();
	QList<AbstractAspect*>::const_iterator i = children.constBegin();
	AbstractAspect *current = 0, *nextSibling = 0;
	if (i != children.end()) {
		current = *i;
		if (++i != children.end())
			nextSibling = *i;
	}

	while (current) {
		emit aspectAboutToBeRemoved(current);
		exec(new AspectChildRemoveCmd(m_aspect_private, current));
		emit aspectRemoved(this, nextSibling, current);

		current = nextSibling;
		if (i != children.end() && ++i != children.end())
			nextSibling = *i;
		else
			nextSibling = 0;
	}

	endMacro();
}

/**
 * \brief Move a child to another parent aspect and transfer ownership.
 */
void AbstractAspect::reparent(AbstractAspect* newParent, int newIndex) {
	Q_ASSERT(parentAspect() != NULL);
	Q_ASSERT(newParent != NULL);
	int max_index = newParent->childCount<AbstractAspect>(IncludeHidden);
	if (newIndex == -1)
		newIndex = max_index;
	Q_ASSERT(newIndex >= 0 && newIndex <= max_index);

	AbstractAspect* old_parent = parentAspect();
	int old_index = old_parent->indexOfChild<AbstractAspect>(this, IncludeHidden);
	AbstractAspect* old_sibling = old_parent->child<AbstractAspect>(old_index+1, IncludeHidden);
	AbstractAspect* new_sibling = newParent->child<AbstractAspect>(newIndex, IncludeHidden);

	//TODO check/test this!
	emit aspectAboutToBeRemoved(this);
	emit newParent->aspectAboutToBeAdded(newParent, new_sibling, this);
	exec(new AspectChildReparentCmd(parentAspect()->m_aspect_private, newParent->m_aspect_private, this, newIndex));
	emit old_parent->aspectRemoved(old_parent, old_sibling, this);
	emit aspectAdded(this);

	endMacro();
}

QList<AbstractAspect*> AbstractAspect::children(const char* className, const ChildIndexFlags &flags) {
	QList<AbstractAspect*> result;
	foreach (AbstractAspect * child, rawChildren()) {
		if (flags & IncludeHidden || !child->hidden()) {
			if ( child->inherits(className) || !(flags & Compress)) {
				result << child;
				if (flags & Recursive){
					result << child->children(className, flags);
				}
			}
		}
	}
	return result;
}

const QList<AbstractAspect*> AbstractAspect::rawChildren() const {
	return m_aspect_private->m_children;
}

/**
 * \brief Remove me from my parent's list of children.
 */
void AbstractAspect::remove() {
	if(parentAspect())
		parentAspect()->removeChild(this);
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
 * warnings (non-critial errors) occur, this function must return
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
	writer->writeStartElement("comment");
	writer->writeCharacters(comment());
	writer->writeEndElement();
}

/**
 * \brief Load comment from an XML element
 */
bool AbstractAspect::readCommentElement(XmlStreamReader * reader){
	setComment(reader->readElementText());
	return true;
}

/**
 * \brief Save name and creation time to XML
 */
void AbstractAspect::writeBasicAttributes(QXmlStreamWriter* writer) const {
	writer->writeAttribute("creation_time" , creationTime().toString("yyyy-dd-MM hh:mm:ss:zzz"));
	writer->writeAttribute("name", name());
}

/**
 * \brief Load name and creation time from XML
 *
 * \return false on error
 */
bool AbstractAspect::readBasicAttributes(XmlStreamReader* reader){
	QXmlStreamAttributes attribs = reader->attributes();

	// name
	QString str = attribs.value("name").toString();
	if(str.isEmpty())
		reader->raiseWarning(i18n("Attribute 'name' is missing or empty."));

	setName(str);

	// creation time
	str = attribs.value("creation_time").toString();
	if(str.isEmpty()) {
		reader->raiseWarning(i18n("Invalid creation time for '%1'. Using current time.", name()));
		setCreationTime(QDateTime::currentDateTime());
	} else {
		QDateTime creation_time = QDateTime::fromString(str, "yyyy-dd-MM hh:mm:ss:zzz");
		if (creation_time.isValid())
			setCreationTime(creation_time);
		else
			setCreationTime(QDateTime::currentDateTime());
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
	m_undoAware = b;
}

/**
 * \brief Return the undo stack of the Project, or 0 if this Aspect is not part of a Project.
 *
 * It's also possible to construct undo-enabled Aspect trees without Project.
 * The only requirement is that the root Aspect reimplements undoStack() to get the
 * undo stack from somewhere (the default implementation just delegates to parentAspect()).
 */
QUndoStack* AbstractAspect::undoStack() const {
	return parentAspect() ? parentAspect()->undoStack() : 0;
}

/**
 * \brief Execute the given command, pushing it on the undoStack() if available.
 */
void AbstractAspect::exec(QUndoCommand* cmd) {
	Q_CHECK_PTR(cmd);
	if (m_undoAware) {
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
	exec(new SignallingUndoCommand("change signal", this,
				preChangeSignal, postChangeSignal, val0, val1, val2, val3));
	exec(command);
	exec(new SignallingUndoCommand("change signal", this,
				postChangeSignal, preChangeSignal, val0, val1, val2, val3));
	endMacro();
}

/**
 * \brief Begin an undo stack macro (series of commands)
 */
void AbstractAspect::beginMacro(const QString& text) {
	if (!m_undoAware)
		return;

	QUndoStack* stack = undoStack();
	if (stack)
		stack->beginMacro(text);
}

/**
 * \brief End the current undo stack macro
 */
void AbstractAspect::endMacro() {
	if (!m_undoAware)
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
void AbstractAspect::setSelected(bool s){
  if (s)
	emit selected(this);
  else
	emit deselected(this);
}

void AbstractAspect::childSelected(const AbstractAspect* aspect) {
	//forward the signal to the highest possible level in the parent-child hierarchy
	//e.g. axis of a plot was selected. Don't include parent aspects here that do not
	//need to react on the selection of children: e.g. Folder or XYFitCurve with
	//the child column for calculated residuals
	if (aspect->parentAspect() != 0
		&& !aspect->parentAspect()->inherits("Folder")
		&& !aspect->parentAspect()->inherits("XYFitCurve"))
		emit aspect->parentAspect()->selected(aspect);
}

void AbstractAspect::childDeselected(const AbstractAspect* aspect) {
	//forward the signal to the highest possible level in the parent-child hierarchy
	//e.g. axis of a plot was selected. Don't include parent aspects here that do not
	//need to react on the deselection of children: e.g. Folder or XYFitCurve with
	//the child column for calculated residuals
	if (aspect->parentAspect() != 0
		&& !aspect->parentAspect()->inherits("Folder")
		&& !aspect->parentAspect()->inherits("XYFitCurve"))
		emit aspect->parentAspect()->deselected(aspect);
}

/**
 * \brief Make the specified name unique among my children by incrementing a trailing number.
 */
QString AbstractAspect::uniqueNameFor(const QString& current_name) const {
	return m_aspect_private->uniqueNameFor(current_name);
}
