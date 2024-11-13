/*
	File                 : aspectcommands.h
	Project              : LabPlot
	Description          : Undo commands used by AbstractAspect.
	Only meant to be used within AbstractAspect.cpp
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007-2010 Knut Franke <knut.franke@gmx.de>
	SPDX-FileCopyrightText: 2007-2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2013-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASPECTCOMMANDS_H
#define ASPECTCOMMANDS_H

#include "AspectPrivate.h"
#include <KLocalizedString>
#include <QUndoCommand>

class AspectCommonCmd : public QUndoCommand {
public:
	AspectCommonCmd(QUndoCommand* parent = nullptr)
		: QUndoCommand(parent) {
	}
	int removeChild(AbstractAspectPrivate* parent, AbstractAspect* child) {
		int index = parent->indexOfChild(child);
		Q_ASSERT(index != -1);
		parent->m_children.removeAll(child);
		QObject::disconnect(child, nullptr, nullptr, nullptr);
		child->setParentAspect(nullptr);
		// QDEBUG(Q_FUNC_INFO << " DONE. CHILD = " << child)
		return index;
	}
};

class AspectChildMoveCmd : public QUndoCommand {
public:
	AspectChildMoveCmd(AbstractAspectPrivate* target, AbstractAspect* child, int steps, QUndoCommand* parent = nullptr)
		: QUndoCommand(parent)
		, m_target(target)
		, m_child(child) {
		setText(i18n("%1: move up", m_target->m_name));
		const int origIndex = m_target->indexOfChild(m_child);
		int newIndex = origIndex + steps;
		if (newIndex > m_target->m_children.count() - 1)
			newIndex = m_target->m_children.count() - 1;
		else if (newIndex < 0)
			newIndex = 0;
		m_index = newIndex;
	}

	virtual ~AspectChildMoveCmd() override {
	}

	virtual void redo() override {
		move(m_index); // move up
	}

	virtual void undo() override {
		move(m_index); // move down
	}

	void move(int newIndex) {
		// First child (index 0): Most behind child
		// Last child: Most front child
		const int origIndex = m_target->indexOfChild(m_child);
		if (newIndex != origIndex) {
			int nonHiddenIndex = 0;
			for (int i = 0; i < newIndex; i++) {
				if (!m_target->m_children.at(i)->isHidden())
					nonHiddenIndex++;
			}

			// According to qt documentation
			// https://doc.qt.io/qt-5/qabstractitemmodel.html#beginMoveRows
			if (newIndex > origIndex)
				nonHiddenIndex++;

			Q_EMIT m_target->q->childAspectAboutToBeMoved(m_child, nonHiddenIndex);

			m_target->m_children.removeAll(m_child);
			m_target->m_children.insert(newIndex, m_child);

			m_index = origIndex;

			Q_EMIT m_target->q->childAspectMoved();
		}
	}

protected:
	AbstractAspectPrivate* m_target{nullptr};
	AbstractAspect* m_child{nullptr};
	int m_index{-1};
};

class AspectChildRemoveCmd : public AspectCommonCmd {
public:
	AspectChildRemoveCmd(AbstractAspectPrivate* target, AbstractAspect* child, QUndoCommand* parent = nullptr)
		: AspectCommonCmd(parent)
		, m_target(target)
		, m_child(child) {
		Q_ASSERT(!child->isMoved());
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

		// emit the "about to be removed" signal also for all children columns so the plots can react.
		const auto& columns = m_child->children<Column>(AbstractAspect::ChildIndexFlag::Recursive);
		for (auto* col : columns) {
			Q_EMIT col->parentAspect()->childAspectAboutToBeRemoved(col);
			Q_EMIT col->aspectAboutToBeRemoved(col);
		}

		// no need to emit signals if the aspect is hidden, the only exceptions is it's a datapicker point
		// and we need to react on its removal in order to update the data spreadsheet.
		// TODO: the check for hidden was added originally to avoid crashes in the debug build of Qt because
		// of asserts for negative values in the model. It also helps wrt. the performance since we don't need
		// to react on such events in the model for hidden aspects. Adding here the exception for the datapicker
		// will most probably trigger again crashes in the debug build of Qt if the datapicker is involved but we
		// rather accept this "edge case" than having no undo/redo for position changes for datapicker points until
		// we have a better solution.
		if (!m_child->isHidden() || m_child->type() == AspectType::DatapickerPoint)
			Q_EMIT m_target->q->childAspectAboutToBeRemoved(m_child);
		Q_EMIT m_child->aspectAboutToBeRemoved(m_child);

		m_index = removeChild(m_target, m_child);

		if (!m_child->isHidden() || m_child->type() == AspectType::DatapickerPoint)
			Q_EMIT m_target->q->childAspectRemoved(m_target->q, nextSibling, m_child);
	}

	// calling undo transfers ownership of m_child back to its parent aspect
	void undo() override {
		Q_ASSERT(m_index != -1); // m_child must be a child of m_target->q

		Q_EMIT m_target->q->childAspectAboutToBeAdded(m_target->q, nullptr, m_child);
		Q_EMIT m_target->q->childAspectAboutToBeAdded(m_target->q, m_index, m_child);
		m_target->insertChild(m_index, m_child);
		m_child->finalizeAdd();
		Q_EMIT m_target->q->childAspectAdded(m_child);
	}

protected:
	AbstractAspectPrivate* m_target{nullptr};
	AbstractAspect* m_child{nullptr};
	int m_index{-1};
};

class AspectChildAddCmd : public AspectChildRemoveCmd {
public:
	AspectChildAddCmd(AbstractAspectPrivate* target, AbstractAspect* child, int index, QUndoCommand* parent)
		: AspectChildRemoveCmd(target, child, parent) {
		setText(i18n("%1: add %2", m_target->m_name, m_child->name()));
		m_index = index;
	}

	void redo() override {
		AspectChildRemoveCmd::undo();
	}

	void undo() override {
		AspectChildRemoveCmd::redo();
	}
};

class AspectChildReparentCmd : public AspectCommonCmd {
public:
	AspectChildReparentCmd(AbstractAspectPrivate* target,
						   AbstractAspectPrivate* new_parent,
						   AbstractAspect* child,
						   int new_index,
						   QUndoCommand* parent = nullptr)
		: AspectCommonCmd(parent)
		, m_target(target)
		, m_new_parent(new_parent)
		, m_child(child)
		, m_new_index(new_index) {
		setText(i18n("%1: move %2 to %3.", m_target->m_name, m_child->name(), m_new_parent->m_name));
	}

	// calling redo transfers ownership of m_child to the new parent aspect
	void redo() override {
		Q_EMIT m_child->childAspectAboutToBeRemoved(m_child);
		m_index = removeChild(m_target, m_child);
		m_new_parent->insertChild(m_new_index, m_child);
		Q_EMIT m_child->childAspectAdded(m_child);
	}

	// calling undo transfers ownership of m_child back to its previous parent aspect
	void undo() override {
		Q_ASSERT(m_index != -1);
		Q_EMIT m_child->childAspectAboutToBeRemoved(m_child);
		removeChild(m_new_parent, m_child);
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

class AspectNameChangeCmd : public AspectCommonCmd {
public:
	AspectNameChangeCmd(AbstractAspectPrivate* aspect, const QString& newName, QUndoCommand* parent = nullptr)
		: AspectCommonCmd(parent)
		, m_aspect(aspect)
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
