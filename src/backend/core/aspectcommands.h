/***************************************************************************
    File                 : aspectcommands.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 by Knut Franke, Tilman Benkert
                           (C) 2010 by Knut Franke
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

class AspectChildRemoveCmd : public QUndoCommand
{
	public:
		AspectChildRemoveCmd(AbstractAspect::Private * target, AbstractAspect* child)
			: m_target(target), m_child(child), m_index(-1), m_removed(false) {
				setText(QObject::tr("%1: remove %2").arg(m_target->m_name).arg(m_child->name()));
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
			Q_ASSERT(m_index != -1); // m_child must be a child of m_target->m_owner
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
				setText(QObject::tr("%1: add %2").arg(m_target->m_name).arg(m_child->name()));
				m_index = index;
				m_removed = true;
			}

		virtual void redo() { AspectChildRemoveCmd::undo(); }

		virtual void undo() { AspectChildRemoveCmd::redo(); }
};

class AspectChildReparentCmd : public QUndoCommand
{
	public:
		AspectChildReparentCmd(AbstractAspect::Private * target, AbstractAspect::Private * new_parent, 
				AbstractAspect* child, int new_index)
			: m_target(target), m_new_parent(new_parent), m_child(child), m_index(-1), m_new_index(new_index)
		{
			setText(QObject::tr("%1: move %2 to %3.").arg(m_target->m_name).arg(m_child->name()).arg(m_new_parent->m_name));
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

