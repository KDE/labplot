/***************************************************************************
    File                 : AbstractAspect.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Base class for all persistent objects in a Project.

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
#include "core/AbstractAspect.h"
#include "core/AspectPrivate.h"
#include "core/aspectcommands.h"
#include "core/Folder.h"
#include "core/Project.h"
#include "lib/XmlStreamReader.h"

#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <QApplication>
#include <QXmlStreamWriter>

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
 * \fn virtual const Project *AbstractAspect::project() const
 * \brief Return the Project this Aspect belongs to, or 0 if it is currently not part of one.
 */

/**
 * \fn virtual Project *AbstractAspect::project()
 * \brief Return the Project this Aspect belongs to, or 0 if it is currently not part of one.
 */

/**
 * \fn virtual QString AbstractAspect::path() const 
 * \brief Return the path that leads from the top-most Aspect (usually a Project) to me.
 */

/**
 * \fn virtual void AbstractAspect::remove()
 * \brief Remove me from my parent's list of children.
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// start of AbstractAspect implementation
////////////////////////////////////////////////////////////////////////////////////////////////////

AbstractAspect::AbstractAspect(const QString &name)
	: m_aspect_private(new Private(this, name))
{
}

AbstractAspect::~AbstractAspect()
{
	delete m_aspect_private;
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
void AbstractAspect::writeCommentElement(QXmlStreamWriter * writer) const
{
	writer->writeStartElement("comment");
	QString temp = comment();
	temp.replace("\n", "\\n");
	writer->writeCDATA(temp);
	writer->writeEndElement();
}

/**
 * \brief Load comment from an XML element
 */
bool AbstractAspect::readCommentElement(XmlStreamReader * reader)
{
	Q_ASSERT(reader->isStartElement() && reader->name() == "comment");
	QString temp = reader->readElementText();
	temp.replace("\\n", "\n");
	setComment(temp);
	return true;
}

/**
 * \brief Save name, creation time and caption spec to XML
 */
void AbstractAspect::writeBasicAttributes(QXmlStreamWriter * writer) const
{
	writer->writeAttribute("creation_time" , creationTime().toString("yyyy-dd-MM hh:mm:ss:zzz"));
	writer->writeAttribute("caption_spec", captionSpec());
	writer->writeAttribute("name", name());
}

/**
 * \brief Load name, creation time and caption spec from XML
 *
 * \return false on error
 */
bool AbstractAspect::readBasicAttributes(XmlStreamReader * reader)
{
	QString prefix(tr("XML read error: ","prefix for XML error messages"));
	QString postfix(tr(" (non-critical)", "postfix for XML error messages"));

	QXmlStreamAttributes attribs = reader->attributes();
	QString str;

	// read name
	str = attribs.value(reader->namespaceUri().toString(), "name").toString();
	if(str.isEmpty())
	{
		reader->raiseWarning(prefix+tr("aspect name missing or empty")+postfix);
	}
	setName(str);
	// read creation time
	str = attribs.value(reader->namespaceUri().toString(), "creation_time").toString();
	QDateTime creation_time = QDateTime::fromString(str, "yyyy-dd-MM hh:mm:ss:zzz");
	if(str.isEmpty() || !creation_time.isValid())
	{
		reader->raiseWarning(tr("Invalid creation time for '%1'. Using current time.").arg(name()));
		setCreationTime(QDateTime::currentDateTime());
	}
	else
		setCreationTime(creation_time);
	// read caption spec
	str = attribs.value(reader->namespaceUri().toString(), "caption_spec").toString();
	setCaptionSpec(str);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return my parent Aspect or 0 if I currently don't have one.
 */
AbstractAspect * AbstractAspect::parentAspect() const
{
	return m_aspect_private->parent();
}

/**
 * \fn template < class T > T *AbstractAspect::ancestor() const
 * \brief Return the closest ancestor of class T (or NULL if none found).
 */

/**
 * \brief Add the given Aspect to my list of children.
 */
void AbstractAspect::addChild(AbstractAspect* child)
{
	Q_CHECK_PTR(child);
	QString new_name = m_aspect_private->uniqueNameFor(child->name());
	beginMacro(tr("%1: add %2.").arg(name()).arg(new_name));
	if (new_name != child->name()) {
		info(tr("Renaming \"%1\" to \"%2\" in order to avoid name collision.").arg(child->name()).arg(new_name));
		child->setName(new_name);
	}
	exec(new AspectChildAddCmd(m_aspect_private, child, m_aspect_private->childCount()));
	endMacro();
}

/**
 * \brief Insert the given Aspect at a specific position in my list of children.
 */
void AbstractAspect::insertChildBefore(AbstractAspect* child, AbstractAspect* before)
{
	Q_CHECK_PTR(child);
	QString new_name = m_aspect_private->uniqueNameFor(child->name());
	beginMacro(tr("%1: insert %2 before %3.").arg(name()).arg(new_name).arg(before ? before->name() : "end"));
	if (new_name != child->name()) {
		info(tr("Renaming \"%1\" to \"%2\" in order to avoid name collision.").arg(child->name()).arg(new_name));
		child->setName(new_name);
	}
	int index = m_aspect_private->indexOfChild(before);
	if (index == -1) index = m_aspect_private->childCount();
	exec(new AspectChildAddCmd(m_aspect_private, child, index));
	endMacro();
}

/**
 * \brief Remove the given Aspect from my list of children.
 *
 * The ownership of the child is transfered to the undo command,
 * i.e., the aspect is deleted by the undo command.
 * \sa reparent()
 */
void AbstractAspect::removeChild(AbstractAspect* child)
{
	Q_ASSERT(child->parentAspect() == this);
	beginMacro(tr("%1: remove %2.").arg(name()).arg(child->name()));
	exec(new AspectChildRemoveCmd(m_aspect_private, child));
	endMacro();
}

/**
 * \brief Move a child to another parent aspect and transfer ownership.
 */
void AbstractAspect::reparent(AbstractAspect * new_parent, int new_index)
{
	Q_ASSERT(parentAspect() != NULL);
	Q_ASSERT(new_parent != NULL);
	int max_index = new_parent->childCount<AbstractAspect>(IncludeHidden);
	if (new_index == -1)
		new_index = max_index;
	Q_ASSERT(new_index >= 0 && new_index <= max_index);
	beginMacro(tr("%1: move from %2 to %3").arg(name()).arg(parentAspect()->name()).arg(new_parent->name()));
	exec(new AspectChildReparentCmd(parentAspect()->m_aspect_private, new_parent->m_aspect_private, this, new_index));
	endMacro();
}

const QList< AbstractAspect* > AbstractAspect::rawChildren() const
{
    return m_aspect_private->children();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! \name undo related
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \fn virtual QUndoStack *AbstractAspect::undoStack() const
 * \brief Return the undo stack of the Project, or 0 if this Aspect is not part of a Project.
 *
 * It's also possible to construct undo-enabled Aspect trees without Project.
 * The only requirement is that the root Aspect reimplements undoStack() to get the
 * undo stack from somewhere (the default implementation just delegates to parentAspect()).
 */
// inlined

/**
 * \brief Execute the given command, pushing it on the undoStack() if available.
 */
void AbstractAspect::exec(QUndoCommand *cmd)
{
	Q_CHECK_PTR(cmd);
	QUndoStack *stack = undoStack();
	if (stack)
		stack->push(cmd);
	else {
		cmd->redo();
		delete cmd;
	}
	if (project())
		project()->setChanged(true);
}

/**
 * \brief Begin an undo stack macro (series of commands)
 */
void AbstractAspect::beginMacro(const QString& text)
{
	QUndoStack *stack = undoStack();
	if (stack)
		stack->beginMacro(text);
}

/**
 * \brief End the current undo stack macro
 */
void AbstractAspect::endMacro()
{
	QUndoStack *stack = undoStack();
	if (stack)
		stack->endMacro();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////////////////////////

QString AbstractAspect::name() const
{
	return m_aspect_private->name();
}

void AbstractAspect::setName(const QString &value)
{
	if (value.isEmpty()) {
		setName("1");
		return;
	}
	if (value == m_aspect_private->name()) return;
	if (m_aspect_private->parent()) {
		QString new_name = m_aspect_private->parent()->uniqueNameFor(value);
		if (new_name != value)
			info(tr("Intended name \"%1\" diverted to \"%2\" in order to avoid name collision.").arg(value).arg(new_name));
		exec(new AspectNameChangeCmd(m_aspect_private, new_name));
	} else
		exec(new AspectNameChangeCmd(m_aspect_private, value));
}

QString AbstractAspect::comment() const
{
	return m_aspect_private->comment();
}

void AbstractAspect::setComment(const QString &value)
{
	if (value == m_aspect_private->comment()) return;
	exec(new AspectCommentChangeCmd(m_aspect_private, value));
}

/**
 * \brief Return the specification string used for constructing the caption().
 *
 * See setCaptionSpec() for format.
 */
QString AbstractAspect::captionSpec() const
{
	return m_aspect_private->captionSpec();
}

/**
 * \brief Set the specification string used for constructing the caption().
 *
 * The caption is constructed by substituting some magic tokens in the specification:
 *
 * - \%n - name()
 * - \%c - comment()
 * - \%t - creationTime()
 *
 * Additionally, you can use the construct %C{&lt;text&gt;}. &lt;text&gt; is only shown in the caption
 * if comment() is not empty (name and creation time should never be empty).
 *
 * The default caption specification is "%n%C{ - }%c".
 */
void AbstractAspect::setCaptionSpec(const QString &value)
{
	if (value == m_aspect_private->captionSpec()) return;
	exec(new AspectCaptionSpecChangeCmd(m_aspect_private, value));
}

/**
 * \brief Set the creation time
 *
 * The creation time will automatically be set when the aspect object
 * is created. This function is usually only needed when the aspect
 * is loaded from a file.
 */
void AbstractAspect::setCreationTime(const QDateTime& time)
{
	if (time == m_aspect_private->creationTime()) return;
	exec(new AspectCreationTimeChangeCmd(m_aspect_private, time));
}

QDateTime AbstractAspect::creationTime() const
{
	return m_aspect_private->creationTime();
}

QString AbstractAspect::caption() const
{
	return m_aspect_private->caption();
}

bool AbstractAspect::hidden() const
{
    return m_aspect_private->hidden();
}

/**
 * \brief Set "hidden" property, i.e. whether to exclude this aspect from being shown in the explorer.
 */
void AbstractAspect::setHidden(bool value)
{
    if (value == m_aspect_private->hidden()) return;
    exec(new AspectHiddenChangeCmd(m_aspect_private, value));
}

/**
 * \brief Return an icon to be used for decorating my views.
 */
QIcon AbstractAspect::icon() const
{
	return QIcon();
}

/**
 * \brief Return a new context menu.
 *
 * The caller takes ownership of the menu.
 */
QMenu *AbstractAspect::createContextMenu()
{
	QMenu * menu = new QMenu();
    
	const QStyle *widget_style = qApp->style();
	QAction *action_temp;

	action_temp = menu->addAction(QObject::tr("&Remove"), this, SLOT(remove()));
	action_temp->setIcon(widget_style->standardIcon(QStyle::SP_TrashIcon));

	return menu;
}

/**
 * \brief Return the folder the Aspect is contained in or 0 if there is none.
 *
 * The returned folder may be the aspect itself if it inherits Folder.
 */
Folder * AbstractAspect::folder()
{
	if(inherits("Folder")) return static_cast<Folder *>(this);
	AbstractAspect * parent_aspect = parentAspect();
	while(parent_aspect && !parent_aspect->inherits("Folder")) 
		parent_aspect = parent_aspect->parentAspect();
	return static_cast<Folder *>(parent_aspect);	
}

/**
 * \brief Return whether the there is a path upwards to the given aspect
 *
 * This also returns true if other==this.
 */
bool AbstractAspect::isDescendantOf(AbstractAspect *other)
{
	if(other == this) return true;
	AbstractAspect * parent_aspect = parentAspect();
	while(parent_aspect)
	{
		if(parent_aspect == other) return true;
		parent_aspect = parent_aspect->parentAspect();
	}
	return false;
}

/**
 * \brief Make the specified name unique among my children by incrementing a trailing number.
 */
QString AbstractAspect::uniqueNameFor(const QString &current_name) const
{
	return m_aspect_private->uniqueNameFor(current_name);
}

/**
 * \brief Remove all child Aspects.
 */
void AbstractAspect::removeAllChildren()
{
	beginMacro(tr("%1: remove all children.").arg(name()));
	foreach(AbstractAspect * child, rawChildren())
		exec(new AspectChildRemoveCmd(m_aspect_private, child));
	endMacro();
}

/**
 * \brief Retrieve a global setting.
 */
QVariant AbstractAspect::global(const QString &key)
{
	QString qualified_key = QString(staticMetaObject.className()) + "/" + key;
	QVariant result = Private::g_settings->value(qualified_key);
	if (result.isValid())
		return result;
	else
		return Private::g_defaults[qualified_key];
}

/**
 * \brief Update a global setting.
 */
void AbstractAspect::setGlobal(const QString &key, const QVariant &value)
{
	Private::g_settings->setValue(QString(staticMetaObject.className()) + "/" + key, value);
}

/**
 * \brief Set default value for a global setting.
 */
void AbstractAspect::setGlobalDefault(const QString &key, const QVariant &value)
{
	Private::g_defaults[QString(staticMetaObject.className()) + "/" + key] = value;
}

