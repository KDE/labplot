/***************************************************************************
    File                 : ShortcutsDialogModel.cpp
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

#include "ShortcutsDialogModel.h"

ShortcutsDialogModelItem::ShortcutsDialogModelItem(ActionManager * manager, int depth, int row, ShortcutsDialogModelItem *parent)
{
	m_action_manager = manager;
    m_parent_item = parent;
    m_depth = depth;
	m_row = row;
	if (parent)
		parent->addChild(this);
}

ShortcutsDialogModelItem::~ShortcutsDialogModelItem()
{
    qDeleteAll(m_child_items);
}

ShortcutsDialogModelItem *ShortcutsDialogModelItem::child(int number)
{
    return m_child_items.value(number);
}

int ShortcutsDialogModelItem::childCount() const
{
    return m_child_items.count();
}

void ShortcutsDialogModelItem::addChild(ShortcutsDialogModelItem * child)
{
	m_child_items.append(child);
}

ShortcutsDialogModelItem *ShortcutsDialogModelItem::parent()
{
    return m_parent_item;
}

ShortcutsDialogModel::ShortcutsDialogModel(QList<ActionManager *> action_managers, QObject * parent)
	: QAbstractItemModel(parent)
{
	m_root_item = new ShortcutsDialogModelItem(0, 0, 0, 0);

	int row = 0;
	foreach(ActionManager *manager, action_managers)
	{
		ShortcutsDialogModelItem * first_level_item = new ShortcutsDialogModelItem(manager, 1, row++, m_root_item);
		first_level_item->setText(manager->title());

		QList<QString> list = manager->internalNames();
		int row2 = 0;
		foreach(QString str, list)
		{
			ShortcutsDialogModelItem * second_level_item = new ShortcutsDialogModelItem(manager, 2, row2++, first_level_item);
			second_level_item->setActionName(str);
			second_level_item->setText(manager->actionText(str));
		}
	}
}

ShortcutsDialogModel::~ShortcutsDialogModel()
{
	delete m_root_item;
}

ShortcutsDialogModelItem * ShortcutsDialogModel::getItem(const QModelIndex & index) const
{
    if (index.isValid()) 
	{
        ShortcutsDialogModelItem * item = static_cast<ShortcutsDialogModelItem *>(index.internalPointer());
        if (item) 
			return item;
    }
    return m_root_item;
}

QModelIndex ShortcutsDialogModel::index(int row, int column, const QModelIndex & parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    ShortcutsDialogModelItem * parent_item = getItem(parent);
	if (!parent_item)
		return QModelIndex();

    ShortcutsDialogModelItem * child_item = parent_item->child(row);
    if (child_item)
        return createIndex(row, column, child_item);
        
	return QModelIndex();
}

QModelIndex ShortcutsDialogModel::parent(const QModelIndex & index) const
{
    if (!index.isValid())
        return QModelIndex();

    ShortcutsDialogModelItem * child_item = getItem(index);
    ShortcutsDialogModelItem * parent_item = child_item->parent();

    if (parent_item == m_root_item)
        return QModelIndex();

    return createIndex(parent_item->row(), 0, parent_item);
}

int ShortcutsDialogModel::rowCount(const QModelIndex & parent) const
{
    ShortcutsDialogModelItem * parent_item = getItem(parent);

    return parent_item->childCount();
}

int ShortcutsDialogModel::columnCount(const QModelIndex & parent) const
{
	Q_UNUSED(parent);
	return 3;
}

QVariant ShortcutsDialogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal)
		switch(role) 
		{
			case Qt::DisplayRole:
				switch(section) 
				{
					case 0: return tr("Action");
					case 1: return tr("Shortcut");
					case 2: return tr("Alternative", "alternative shortcut");
				}
		}
	return QVariant();
}

QVariant ShortcutsDialogModel::data(const QModelIndex & index, int role) const
{
	if (index.isValid())
		switch(role) 
		{
			case Qt::DisplayRole:
			case Qt::EditRole:
			case Qt::ToolTipRole:
				{
					ShortcutsDialogModelItem * item = getItem(index);
					switch(item->depth()) 
					{
						case 1: 
							if (index.column() == 0)
								return QVariant(item->text());
							break;
						case 2: 
							switch(index.column())
							{
								case 0:
									return QVariant(item->text());
								case 1:
								case 2:
									{
										QString action_name = item->actionName();
										ActionManager * manager = item->actionManager();
										QList<QKeySequence> sequences = manager->shortcuts(action_name);
										return sequences.value(index.column()-1);
									}
							}
					}
				}
		}

	return QVariant();
}

Qt::ItemFlags ShortcutsDialogModel::flags(const QModelIndex & index) const
{
	Qt::ItemFlags result = Qt::ItemIsEnabled;
	if (index.isValid())
	{
		ShortcutsDialogModelItem * item = getItem(index);
		switch(item->depth()) 
		{
			case 1: 
				break;
			case 2: 
				if (index.column() == 1 || index.column() == 2)
					result |= Qt::ItemIsSelectable | Qt::ItemIsEditable;
		}
	}

	return result;
}

bool ShortcutsDialogModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	if (!index.isValid() || role != Qt::EditRole) return false;


	ShortcutsDialogModelItem * item = getItem(index);
	if (item->depth() == 2)  
	{
		QString action_name = item->actionName();
		ActionManager * manager = item->actionManager();
		QList<QKeySequence> sequences = manager->shortcuts(action_name);

		switch(index.column())
		{
			case 1: 
				if (sequences.size() > 0)
					sequences.replace(0, value.value<QKeySequence>());
				else
					sequences.append(value.value<QKeySequence>());
				break;
			case 2: 
				if (sequences.size() > 1)
					sequences.replace(1, value.value<QKeySequence>());
				else if (sequences.size() > 0)
					sequences.append(value.value<QKeySequence>());
				else 
				{
					sequences.append(QKeySequence());
					sequences.append(value.value<QKeySequence>());
				}
				break;
			default:
				return false;
		}
		manager->setShortcuts(action_name, sequences);
		emit dataChanged(index, index);
		return true;
	}

	return false;
}

