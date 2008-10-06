/***************************************************************************
    File                 : aspectcommands.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Undo commands used by AbstractAspect.
                           Only meant to be used within AbstractAspect.cpp

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

#include "AspectPrivate.h"

#include <QUndoCommand>

class AspectNameChangeCmd : public QUndoCommand
{
	public:
		AspectNameChangeCmd(AbstractAspect::Private *target, const QString &new_name)
			: m_target(target), m_other_name(new_name) {
				setText(QObject::tr("%1: rename to %2").arg(m_target->name()).arg(new_name));
			}

		virtual void redo() {
			QString tmp = m_target->name();
			m_target->setName(m_other_name);
			m_other_name = tmp;
		}

		virtual void undo() { redo(); }

	private:
		AbstractAspect::Private *m_target;
		QString m_other_name;
};

class AspectCommentChangeCmd : public QUndoCommand
{
	public:
		AspectCommentChangeCmd(AbstractAspect::Private *target, const QString &new_comment)
			: m_target(target), m_other_comment(new_comment) {
				setText(QObject::tr("%1: change comment").arg(m_target->name()));
			}

		virtual void redo() {
			QString tmp = m_target->comment();
			m_target->setComment(m_other_comment);
			m_other_comment = tmp;
		}

		virtual void undo() { redo(); }

	private:
		AbstractAspect::Private *m_target;
		QString m_other_comment;
};

class AspectCaptionSpecChangeCmd : public QUndoCommand
{
	public:
		AspectCaptionSpecChangeCmd(AbstractAspect::Private *target, const QString &new_caption_spec)
			: m_target(target), m_other_caption_spec(new_caption_spec) {
				setText(QObject::tr("%1: change caption").arg(m_target->name()));
			}

		virtual void redo() {
			QString tmp = m_target->captionSpec();
			m_target->setCaptionSpec(m_other_caption_spec);
			m_other_caption_spec = tmp;
		}

		virtual void undo() { redo(); }

	private:
		AbstractAspect::Private *m_target;
		QString m_other_caption_spec;
};


class AspectCreationTimeChangeCmd : public QUndoCommand
{
	public:
		AspectCreationTimeChangeCmd(AbstractAspect::Private *target, const QDateTime &new_creation_time)
			: m_target(target), m_other_creation_time(new_creation_time) {
				setText(QObject::tr("%1: set creation time").arg(m_target->name()));
			}

		virtual void redo() {
			QDateTime tmp = m_target->creationTime();
			m_target->setCreationTime(m_other_creation_time);
			m_other_creation_time = tmp;
		}

		virtual void undo() { redo(); }

	private:
		AbstractAspect::Private *m_target;
		QDateTime m_other_creation_time;
};


class AspectChildRemoveCmd : public QUndoCommand
{
	public:
		AspectChildRemoveCmd(AbstractAspect::Private * target, AbstractAspect* child)
			: m_target(target), m_child(child), m_index(-1), m_removed(false) {
				setText(QObject::tr("%1: remove %2").arg(m_target->name()).arg(m_child->name()));
			}
		~AspectChildRemoveCmd() {
			if (m_removed)			
				delete m_child;
		}

		// calling redo transfers ownership of m_child to the undo command
		virtual void redo() {
			m_index = m_target->removeChild(m_child);
			m_removed = true;
		}

		// calling undo transfers ownership of m_child back to its parent aspect
		virtual void undo() {
			Q_ASSERT(m_index != -1); // m_child must be a child of m_target->owner()
			m_target->insertChild(m_index, m_child);
			m_removed = false;
		}

	protected:
		AbstractAspect::Private * m_target;
		AbstractAspect* m_child;
		int m_index;
		bool m_removed;
};

class AspectChildAddCmd : public AspectChildRemoveCmd
{
	public:
		AspectChildAddCmd(AbstractAspect::Private * target, AbstractAspect* child, int index)
			: AspectChildRemoveCmd(target, child) {
				setText(QObject::tr("%1: add %2").arg(m_target->name()).arg(m_child->name()));
				m_index = index;
			}

		virtual void redo() { AspectChildRemoveCmd::undo(); }

		virtual void undo() { AspectChildRemoveCmd::redo(); }
};

class AspectChildMoveCmd : public QUndoCommand
{
	public:
		AspectChildMoveCmd(AbstractAspect::Private * target, int from, int to)
			: m_target(target), m_from(from), m_to(to) {
				setText(QObject::tr("%1: move child from position %2 to %3.").arg(m_target->name()).arg(m_from+1).arg(m_to+1));
			}

		virtual void redo() {
			// Moving in one go confuses QTreeView, so we would need another two signals
			// to be mapped to QAbstractItemModel::layoutAboutToBeChanged() and ::layoutChanged().
			AbstractAspect * child = m_target->child(m_from);
			m_target->removeChild(child);
			m_target->insertChild(m_to, child);
		}

		virtual void undo() {
			AbstractAspect * child = m_target->child(m_to);
			m_target->removeChild(child);
			m_target->insertChild(m_from, child);
		}

	private:
		AbstractAspect::Private *m_target;
		int m_from, m_to;
};

class AspectChildReparentCmd : public QUndoCommand
{
	public:
		AspectChildReparentCmd(AbstractAspect::Private * target, AbstractAspect::Private * new_parent, 
				AbstractAspect* child, int new_index)
			: m_target(target), m_new_parent(new_parent), m_child(child), m_index(-1), m_new_index(new_index)
		{
			setText(QObject::tr("%1: move %2 to %3.").arg(m_target->name()).arg(m_child->name()).arg(m_new_parent->name()));
		}
		~AspectChildReparentCmd() {}

		// calling redo transfers ownership of m_child to the new parent aspect
		virtual void redo() 
		{
			m_index = m_target->removeChild(m_child);
			m_new_parent->insertChild(m_new_index, m_child);
		}

		// calling undo transfers ownership of m_child back to its previous parent aspect
		virtual void undo() 
		{
			Q_ASSERT(m_index != -1); 
			m_new_parent->removeChild(m_child);
			m_target->insertChild(m_index, m_child);
		}

	protected:
		AbstractAspect::Private * m_target;
		AbstractAspect::Private * m_new_parent;
		AbstractAspect* m_child;
		int m_index;
		int m_new_index;
};

