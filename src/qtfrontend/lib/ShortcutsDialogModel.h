/***************************************************************************
    File                 : ShortcutsDialogModel.h
    Project              : SciDAVis
    Description          : Model for ShortcutsDialog
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
                           
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

#ifndef SHORTCUTSDIALOGMODEL_H
#define SHORTCUTSDIALOGMODEL_H

#include <QAbstractItemModel>
#include "ActionManager.h"

//! Items that describe an entry in ShortcutsDialogModel
/**
 * Pointers to these items are stored in the internal pointer of
 * the corresponding QModelIndex.
 * The roles of the items are characterized by their depth value:
 * 0: root item, 1: action manager: 2: an action key (internal name 
 * in ActionManager).
 */
class ShortcutsDialogModelItem
{
	public:
		//! Ctor
		/** 
		 * \param manager the corresponding ActionManager
		 * \param depth 0: root item, 1: action manager: 2: an action key (internal name in ActionManager)
		 * \param row row in the list of children of the parent item
		 * \param parent parten item; if parent != 0, the new item will automatically added to parent's children
		 */
		 ShortcutsDialogModelItem(ActionManager * manager, int depth, int row, ShortcutsDialogModelItem *parent = 0);
		~ShortcutsDialogModelItem();

		ShortcutsDialogModelItem * parent();
		ShortcutsDialogModelItem * child(int number);
		int childCount() const;
		int depth() const { return m_depth; };
		int row() const { return m_row; };
		ActionManager * actionManager() const { return m_action_manager; }
		CLASS_ACCESSOR(QString, m_text, text, Text);
		CLASS_ACCESSOR(QString, m_action_name, actionName, ActionName);
		void addChild(ShortcutsDialogModelItem *child);

	private:
		//! List of child items
		QList<ShortcutsDialogModelItem*> m_child_items;
		//! Depth value
		/**
		 * 0: root item, 1: action manager: 2: an action key (internal name in ActionManager)
		 */
		int m_depth;
		//! row in the list of children of the parent item
		int m_row;
		//! The text displayed in column 0 of the model
		/** 
		 * The meaning of this depends on depth()
		 * 0: no meaning
		 * 1: ActionManager title
		 * 2: action text/description
		 */
		QString m_text;
		//! The key/internal name of the corresponding actioin (only valid if depth() == 2)
		QString m_action_name;
		//! The parent item
		ShortcutsDialogModelItem * m_parent_item;
		//! The corresponding ActionManager (only valid for depth() >= 1)
		ActionManager * m_action_manager;
};

//! Model for ShortcutsDialog
/**
 * This model contains three columns:
 * action text, (primarty) shortcut, alternative shortcut. The actions are 
 * displayed as children of their managers (represented by their titles).
 *
 */
class ShortcutsDialogModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		ShortcutsDialogModel(QList<ActionManager *> action_managers, QObject * parent = 0);
		~ShortcutsDialogModel();

		QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & index) const;
		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
		bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
		Qt::ItemFlags flags(const QModelIndex & index) const;

	private:
		ShortcutsDialogModelItem * m_root_item;
		ShortcutsDialogModelItem * getItem(const QModelIndex & index) const;
};

#endif // SHORTCUTSDIALOGMODEL_H

