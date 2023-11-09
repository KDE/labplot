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
#include <KLocalizedString>
#include <QUndoCommand>

class AspectChildRemoveCmd : public QUndoCommand {
public:
	AspectChildRemoveCmd(AbstractAspectPrivate* target, AbstractAspect* child, QUndoCommand* parent = nullptr)
		: QUndoCommand(parent)
		, m_target(target)
		, m_child(child)
		, m_moved(child->isMoved()) {
		setText(i18n("%1: remove %2", m_target->m_name, m_child->name()));
	}

	~AspectChildRemoveCmd() override {
		// TODO: this makes labplot crashing on project close/save.
		// 			if (m_removed)
		// 				delete m_child;
	}

	// calling redo transfers ownership of m_child to the undo command
	void redo() override {
		// QDEBUG(Q_FUNC_INFO << ", TARGET = " << m_target->q << " CHILD = " << m_child << ", PARENT = " << m_child->parentAspect())
		AbstractAspect* nextSibling;
		if (m_child == m_target->m_children.last())
			nextSibling = nullptr;
		else
			nextSibling = m_target->m_children.at(m_target->indexOfChild(m_child) + 1);

		// emit the "about to be removed" signal also for all children columns so the curves can react.
		// no need to notify when the parent is just being moved (move up, moved down in the project explorer),
		//(move = delete at the current position + insert at the new position)
		if (!m_moved) {
			const auto& columns = m_child->children<Column>(AbstractAspect::ChildIndexFlag::Recursive);
			for (auto* col : columns)
				Q_EMIT col->parentAspect()->childAspectAboutToBeRemoved(col);
		}

		// no need to emit signals if the aspect is hidden, the only exceptions is it's a datapicker point
		// and we need to react on its removal in order to update the data spreadsheet.
		// TODO: the check for hidden was added originally to avoid crashes in the debug build of Qt because
		// of asserts for negative values in the model. It also helps wrt. the performance since we don't need
		// to react on such events in the model for hidden aspects. Adding here the exception for the datapicker
		// will most probably trigger again crashes in the debug build of Qt if the datapicker is involved but we
		// rather accept this "edge case" than having no undo/redo for position changes for datapicker points until
		// we have a better solution.
		if (!m_child->hidden() || m_child->type() == AspectType::DatapickerPoint)
			Q_EMIT m_target->q->childAspectAboutToBeRemoved(m_child);

		m_index = m_target->removeChild(m_child);

		if (!m_child->hidden() || m_child->type() == AspectType::DatapickerPoint)
			Q_EMIT m_target->q->childAspectRemoved(m_target->q, nextSibling, m_child);

		// QDEBUG(Q_FUNC_INFO << ", DONE. CHILD = " << m_child)
		//		m_removed = true;
	}

	// calling undo transfers ownership of m_child back to its parent aspect
	void undo() override {
		Q_ASSERT(m_index != -1); // m_child must be a child of m_target->q

		if (m_moved)
			m_child->setMoved(true);

		Q_EMIT m_target->q->childAspectAboutToBeAdded(m_target->q, nullptr, m_child);
		Q_EMIT m_target->q->childAspectAboutToBeAdded(m_target->q, m_index, m_child);
		m_target->insertChild(m_index, m_child);
		m_child->finalizeAdd();
		Q_EMIT m_target->q->childAspectAdded(m_child);

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
	AspectChildAddCmd(AbstractAspectPrivate* target, AbstractAspect* child, int index, QUndoCommand* parent)
		: AspectChildRemoveCmd(target, child, parent) {
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
	AspectChildReparentCmd(AbstractAspectPrivate* target, AbstractAspectPrivate* new_parent, AbstractAspect* child, int new_index)
		: m_target(target)
		, m_new_parent(new_parent)
		, m_child(child)
		, m_new_index(new_index) {
		setText(i18n("%1: move %2 to %3.", m_target->m_name, m_child->name(), m_new_parent->m_name));
	}

	// calling redo transfers ownership of m_child to the new parent aspect
	void redo() override {
		Q_EMIT m_child->childAspectAboutToBeRemoved(m_child);
		m_index = m_target->removeChild(m_child);
		m_new_parent->insertChild(m_new_index, m_child);
		Q_EMIT m_child->childAspectAdded(m_child);
	}

	// calling undo transfers ownership of m_child back to its previous parent aspect
	void undo() override {
		Q_ASSERT(m_index != -1);
		Q_EMIT m_child->childAspectAboutToBeRemoved(m_child);
		m_new_parent->removeChild(m_child);
		m_target->insertChild(m_index, m_child);
		Q_EMIT m_child->childAspectAdded(m_child);
	}

protected:
	AbstractAspectPrivate* m_target;
	AbstractAspectPrivate* m_new_parent;
	AbstractAspect* m_child;
	int m_index{-1};
	int m_new_index;
};

class AspectNameChangeCmd : public QUndoCommand {
public:
	AspectNameChangeCmd(AbstractAspectPrivate* aspect, const QString& newName)
		: m_aspect(aspect)
		, m_name(newName) {
		setText(i18n("%1: rename to %2", m_aspect->m_name, newName));
	}

	void redo() override {
		Q_EMIT m_aspect->q->aspectDescriptionAboutToChange(m_aspect->q);
		const QString name = m_aspect->m_name;
		m_aspect->m_name = m_name;
		m_name = name;
		Q_EMIT m_aspect->q->aspectDescriptionChanged(m_aspect->q);
	}

	void undo() override {
		redo();
	}

protected:
	AbstractAspectPrivate* m_aspect;
	QString m_name;
};

#endif
