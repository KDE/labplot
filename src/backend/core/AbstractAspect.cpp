/***************************************************************************
    File                 : AbstractAspect.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
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

AbstractAspect::AbstractAspect(const QString &name)
	: m_aspect_private(new Private(this, name))
{
}

AbstractAspect::~AbstractAspect()
{
	delete m_aspect_private;
}

void AbstractAspect::writeCommentElement(QXmlStreamWriter * writer) const
{
	writer->writeStartElement("comment");
	QString temp = comment();
	temp.replace("\n", "\\n");
	writer->writeCDATA(temp);
	writer->writeEndElement();
}

bool AbstractAspect::readCommentElement(XmlStreamReader * reader)
{
	Q_ASSERT(reader->isStartElement() && reader->name() == "comment");
	QString temp = reader->readElementText();
	temp.replace("\\n", "\n");
	setComment(temp);
	return true;
}

void AbstractAspect::writeBasicAttributes(QXmlStreamWriter * writer) const
{
	writer->writeAttribute("creation_time" , creationTime().toString("yyyy-dd-MM hh:mm:ss:zzz"));
	writer->writeAttribute("caption_spec", captionSpec());
	writer->writeAttribute("name", name());
}

bool AbstractAspect::readBasicAttributes(XmlStreamReader * reader)
{
	QString prefix(tr("XML read error: ","prefix for XML error messages"));
	QString postfix(tr(" (loading failed)", "postfix for XML error messages"));

	QXmlStreamAttributes attribs = reader->attributes();
	QString str;

	// read name
	str = attribs.value(reader->namespaceUri().toString(), "name").toString();
	if(str.isEmpty())
	{
		reader->raiseError(prefix+tr("aspect name missing")+postfix);
		return false;
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

AbstractAspect * AbstractAspect::parentAspect() const
{
	return m_aspect_private->parent();
}

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
	completeAspectInsertion(child, m_aspect_private->childCount()-1);
	endMacro();
}

void AbstractAspect::insertChild(AbstractAspect* child, int index)
{
	Q_CHECK_PTR(child);
	QString new_name = m_aspect_private->uniqueNameFor(child->name());
	beginMacro(tr("%1: insert %2 at position %3.").arg(name()).arg(new_name).arg(index+1));
	if (new_name != child->name()) {
		info(tr("Renaming \"%1\" to \"%2\" in order to avoid name collision.").arg(child->name()).arg(new_name));
		child->setName(new_name);
	}
	exec(new AspectChildAddCmd(m_aspect_private, child, index));
	completeAspectInsertion(child, index);
	endMacro();
}

void AbstractAspect::removeChild(AbstractAspect* child)
{
	Q_ASSERT(indexOfChild(child) != -1);
	beginMacro(tr("%1: remove %2.").arg(name()).arg(child->name()));
	prepareAspectRemoval(child);
	exec(new AspectChildRemoveCmd(m_aspect_private, child));
	endMacro();
}

void AbstractAspect::reparentChild(AbstractAspect *new_parent, AbstractAspect *child)
{
	Q_ASSERT(new_parent != NULL);
	reparentChild(new_parent, child, new_parent->childCount());
}

void AbstractAspect::reparentChild(AbstractAspect *new_parent, AbstractAspect *child, int new_index)
{
	Q_ASSERT(indexOfChild(child) != -1);
	Q_ASSERT(new_index > 0 && new_index <= new_parent->childCount());
	Q_ASSERT(new_parent != NULL);
	beginMacro(tr("%1: move %2 to %3.").arg(name()).arg(child->name()).arg(new_parent->name()));
	prepareAspectRemoval(child);
	exec(new AspectChildReparentCmd(m_aspect_private, new_parent->m_aspect_private, child, new_index));
	new_parent->completeAspectInsertion(child, new_index);
	endMacro();
}

void AbstractAspect::removeChild(int index)
{
	Q_ASSERT(index >= 0 && index <= childCount());
	removeChild(m_aspect_private->child(index));
}

AbstractAspect* AbstractAspect::child(int index) const
{
	Q_ASSERT(index >= 0 && index <= childCount());
	return m_aspect_private->child(index);
}

int AbstractAspect::childCount() const
{
	return m_aspect_private->childCount();
}

int AbstractAspect::indexOfChild(const AbstractAspect *child) const
{
	return m_aspect_private->indexOfChild(child);
}

void AbstractAspect::moveChild(int from, int to)
{
	Q_ASSERT(0 <= from && from < m_aspect_private->childCount());
	Q_ASSERT(0 <= to   && to   < m_aspect_private->childCount());
	exec(new AspectChildMoveCmd(m_aspect_private, from, to));
}

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

void AbstractAspect::beginMacro(const QString& text)
{
	QUndoStack *stack = undoStack();
	if (stack)
		stack->beginMacro(text);
}

void AbstractAspect::endMacro()
{
	QUndoStack *stack = undoStack();
	if (stack)
		stack->endMacro();
}

QString AbstractAspect::name() const
{
	return m_aspect_private->name();
}

void AbstractAspect::setName(const QString &value)
{
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

QString AbstractAspect::captionSpec() const
{
	return m_aspect_private->captionSpec();
}

void AbstractAspect::setCaptionSpec(const QString &value)
{
	if (value == m_aspect_private->captionSpec()) return;
	exec(new AspectCaptionSpecChangeCmd(m_aspect_private, value));
}

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

QIcon AbstractAspect::icon() const
{
	return QIcon();
}

QMenu *AbstractAspect::createContextMenu()
{
	QMenu * menu = new QMenu();
    
	const QStyle *widget_style = qApp->style();
	QAction *action_temp;

	action_temp = menu->addAction(QObject::tr("&Remove"), this, SLOT(remove()));
	action_temp->setIcon(widget_style->standardIcon(QStyle::SP_TrashIcon));

	return menu;
}

Folder * AbstractAspect::folder()
{
	if(inherits("Folder")) return static_cast<Folder *>(this);
	AbstractAspect * parent_aspect = parentAspect();
	while(parent_aspect && !parent_aspect->inherits("Folder")) 
		parent_aspect = parent_aspect->parentAspect();
	return static_cast<Folder *>(parent_aspect);	
}

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

QString AbstractAspect::uniqueNameFor(const QString &current_name) const
{
	return m_aspect_private->uniqueNameFor(current_name);
}

QList<AbstractAspect *> AbstractAspect::descendantsThatInherit(const char * class_name)
{
	QList<AbstractAspect *> list;
	if (inherits(class_name))
		list << this;
	for (int i=0; i<childCount(); i++)
		list << child(i)->descendantsThatInherit(class_name);
	return list;
}

void AbstractAspect::removeAllChildAspects()
{
	beginMacro(tr("%1: remove all children.").arg(name()));
	for (int i=childCount()-1; i >= 0; i--) 
		removeChild(i);
	endMacro();
}

QVariant AbstractAspect::global(const QString &key)
{
	QString qualified_key = QString(staticMetaObject.className()) + "/" + key;
	QVariant result = Private::g_settings->value(qualified_key);
	if (result.isValid())
		return result;
	else
		return Private::g_defaults[qualified_key];
}

void AbstractAspect::setGlobal(const QString &key, const QVariant &value)
{
	Private::g_settings->setValue(QString(staticMetaObject.className()) + "/" + key, value);
}

void AbstractAspect::setGlobalDefault(const QString &key, const QVariant &value)
{
	Private::g_defaults[QString(staticMetaObject.className()) + "/" + key] = value;
}

