/***************************************************************************
    File                 : aspectcommands.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2010 by Knut Franke (knut.franke@gmx.de)
	Copyright            : (C) 2007-2009 Tilman Benkert(thzs@gmx.net)
	Copyright            : (C) 2013-2017 by Alexander Semke (alexander.semke@web.de)
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
#ifndef ASPECTCOMMANDS_H
#define ASPECTCOMMANDS_H

#include "AspectPrivate.h"
#include <QUndoCommand>
#include <KLocalizedString>

class AspectChildRemoveCmd : public QUndoCommand {
public:
	AspectChildRemoveCmd(AbstractAspectPrivate* target, AbstractAspect* child)
		: m_target(target), m_child(child), m_index(-1) {
// 			, m_removed(false) {
		setText(i18n("%1: remove %2", m_target->m_name, m_child->name()));
	}

	~AspectChildRemoveCmd() override {
		//TODO: this makes labplot crashing on project close/save.
// 			if (m_removed)
// 				delete m_child;
	}

	// calling redo transfers ownership of m_child to the undo command
	void redo() override {
		AbstractAspect* nextSibling;
		if (m_child == m_target->m_children.last())
			nextSibling = nullptr;
		else
			nextSibling = m_target->m_children.at(m_target->indexOfChild(m_child) + 1);

		emit m_target->q->aspectAboutToBeRemoved(m_child);
		m_index = m_target->removeChild(m_child);
		emit m_target->q->aspectRemoved(m_target->q, nextSibling, m_child);
//		m_removed = true;
	}

	// calling undo transfers ownership of m_child back to its parent aspect
	void undo() override {
		Q_ASSERT(m_index != -1); // m_child must be a child of m_target->q

		emit m_target->q->aspectAboutToBeAdded(m_target->q, nullptr, m_child);
		m_target->insertChild(m_index, m_child);
		emit m_target->q->aspectAdded(m_child);
// 		m_removed = false;
	}

protected:
	AbstractAspectPrivate* m_target;
	AbstractAspect* m_child;
	int m_index;
// 	bool m_removed;
};

class AspectChildAddCmd : public AspectChildRemoveCmd {
public:
	AspectChildAddCmd(AbstractAspectPrivate* target, AbstractAspect* child, int index)
		: AspectChildRemoveCmd(target, child) {
		setText(i18n("%1: add %2", m_target->m_name, m_child->name()));
		m_index = index;
// 		m_removed = true;
	}

	void redo() override {
		AspectChildRemoveCmd::undo();
	}

	void undo() override {
		AspectChildRemoveCmd::redo();
	}
};

class AspectChildReparentCmd : public QUndoCommand {
public:
	AspectChildReparentCmd(AbstractAspectPrivate* target, AbstractAspectPrivate* new_parent,
	                       AbstractAspect* child, int new_index)
		: m_target(target), m_new_parent(new_parent), m_child(child), m_index(-1), m_new_index(new_index) {
		setText(i18n("%1: move %2 to %3.", m_target->m_name, m_child->name(), m_new_parent->m_name));
	}

	// calling redo transfers ownership of m_child to the new parent aspect
	void redo() override {
		m_index = m_target->removeChild(m_child);
		m_new_parent->insertChild(m_new_index, m_child);
	}

	// calling undo transfers ownership of m_child back to its previous parent aspect
	void undo() override {
		Q_ASSERT(m_index != -1);
		m_new_parent->removeChild(m_child);
		m_target->insertChild(m_index, m_child);
	}

protected:
	AbstractAspectPrivate * m_target;
	AbstractAspectPrivate * m_new_parent;
	AbstractAspect* m_child;
	int m_index;
	int m_new_index;
};

#endif
