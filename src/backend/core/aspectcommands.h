/*
    File                 : aspectcommands.h
    Project              : LabPlot
    Description          : Undo commands used by AbstractAspect.
    Only meant to be used within AbstractAspect.cpp
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007-2010 Knut Franke <knut.franke@gmx.de>
    SPDX-FileCopyrightText: 2007-2009 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2013-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASPECTCOMMANDS_H
#define ASPECTCOMMANDS_H

#include "AspectPrivate.h"
#include <QUndoCommand>
#include <KLocalizedString>

class AspectChildRemoveCmd : public QUndoCommand {
public:
	AspectChildRemoveCmd(AbstractAspectPrivate* target, AbstractAspect* child)
		: m_target(target), m_child(child), m_moved(child->isMoved()) {
		setText(i18n("%1: remove %2", m_target->m_name, m_child->name()));
	}

	~AspectChildRemoveCmd() override {
		//TODO: this makes labplot crashing on project close/save.
// 			if (m_removed)
// 				delete m_child;
	}

	// calling redo transfers ownership of m_child to the undo command
	void redo() override {
		QDEBUG(Q_FUNC_INFO << ", TARGET = " << m_target->q << " CHILD = " << m_child << ", PARENT = " << m_child->parentAspect())
		AbstractAspect* nextSibling;
		if (m_child == m_target->m_children.last())
			nextSibling = nullptr;
		else
			nextSibling = m_target->m_children.at(m_target->indexOfChild(m_child) + 1);

		//emit the "about to be removed" signal also for all children columns so the curves can react.
		//no need to notify when the parent is just being moved (move up, moved down in the project explorer),
		//(move = delete at the current position + insert at the new position)
		if (!m_moved) {
			const auto& columns = m_child->children<Column>(AbstractAspect::ChildIndexFlag::Recursive);
			for (auto* col : columns)
				emit col->parentAspect()->aspectAboutToBeRemoved(col);
		}

		emit m_target->q->aspectAboutToBeRemoved(m_child);
		m_index = m_target->removeChild(m_child);
		emit m_target->q->aspectRemoved(m_target->q, nextSibling, m_child);
		QDEBUG(Q_FUNC_INFO << ", DONE. CHILD = " << m_child)
//		m_removed = true;
	}

	// calling undo transfers ownership of m_child back to its parent aspect
	void undo() override {
		Q_ASSERT(m_index != -1); // m_child must be a child of m_target->q

		if (m_moved)
			m_child->setMoved(true);

		emit m_target->q->aspectAboutToBeAdded(m_target->q, nullptr, m_child);
		m_target->insertChild(m_index, m_child);
		m_child->finalizeAdd();
		emit m_target->q->aspectAdded(m_child);

		if (m_moved)
			m_child->setMoved(false);
// 		m_removed = false;
	}

protected:
	AbstractAspectPrivate* m_target{nullptr};
	AbstractAspect* m_child{nullptr};
	int m_index{-1};
	bool m_moved{false};
// 	bool m_removed{false};
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
		: m_target(target), m_new_parent(new_parent), m_child(child), m_new_index(new_index) {
		setText(i18n("%1: move %2 to %3.", m_target->m_name, m_child->name(), m_new_parent->m_name));
	}

	// calling redo transfers ownership of m_child to the new parent aspect
	void redo() override {
		emit m_child->aspectAboutToBeRemoved(m_child);
		m_index = m_target->removeChild(m_child);
		m_new_parent->insertChild(m_new_index, m_child);
		emit m_child->aspectAdded(m_child);
	}

	// calling undo transfers ownership of m_child back to its previous parent aspect
	void undo() override {
		Q_ASSERT(m_index != -1);
		emit m_child->aspectAboutToBeRemoved(m_child);
		m_new_parent->removeChild(m_child);
		m_target->insertChild(m_index, m_child);
		emit m_child->aspectAdded(m_child);
	}

protected:
	AbstractAspectPrivate* m_target;
	AbstractAspectPrivate* m_new_parent;
	AbstractAspect* m_child;
	int m_index{-1};
	int m_new_index;
};

#endif
