/***************************************************************************
    File                 : AspectTreeModel.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Represents a tree of AbstractAspect objects as a
                           Qt item model.

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
#include "AspectTreeModel.h"
#include <QDateTime>
#include <QIcon>
#include <QMenu>

AspectTreeModel::AspectTreeModel(AbstractAspect* root, QObject *parent)
	: QAbstractItemModel(parent), m_root(root)
{
	connect(m_root, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)), 
		this, SLOT(aspectDescriptionChanged(const AbstractAspect *)));
	connect(m_root, SIGNAL(aspectAboutToBeAdded(const AbstractAspect *, int)), 
		this, SLOT(aspectAboutToBeAdded(const AbstractAspect *, int)));
	connect(m_root, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *, int)), 
		this, SLOT(aspectAboutToBeRemoved(const AbstractAspect *, int)));
	connect(m_root, SIGNAL(aspectAdded(const AbstractAspect *, int)), 
		this, SLOT(aspectAdded(const AbstractAspect *, int)));
	connect(m_root, SIGNAL(aspectRemoved(const AbstractAspect *, int)), 
		this, SLOT(aspectRemoved(const AbstractAspect *, int)));
}

AspectTreeModel::~AspectTreeModel()
{
	disconnect(m_root,0,this,0);
}

QModelIndex AspectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent)) return QModelIndex();
	if(!parent.isValid())
	{
		if(row != 0) return QModelIndex();
		return createIndex(row, column, m_root);
	}
	AbstractAspect *parent_aspect = static_cast<AbstractAspect*>(parent.internalPointer());
	AbstractAspect *child_aspect = parent_aspect->child(row);
	if (!child_aspect) return QModelIndex();
	return createIndex(row, column, child_aspect);
}

QModelIndex AspectTreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid()) return QModelIndex();
	AbstractAspect *parent_aspect = static_cast<AbstractAspect*>(index.internalPointer())->parentAspect();
	if (!parent_aspect) return QModelIndex();
	return modelIndexOfAspect(parent_aspect);
}

int AspectTreeModel::rowCount(const QModelIndex &parent) const
{
	if (!parent.isValid()) return 1;
	AbstractAspect *parent_aspect =  static_cast<AbstractAspect*>(parent.internalPointer());
	return parent_aspect->childCount();
}

int AspectTreeModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 4;
}

QVariant AspectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation != Qt::Horizontal) return QVariant();
	switch(role) {
		case Qt::DisplayRole:
			switch(section) {
				case 0: return tr("Name");
				case 1: return tr("Type");
				case 2: return tr("Created");
				case 3: return tr("Comment");
				default: return QVariant();
			}
		case Qt::SizeHintRole:
			switch(section) {
				case 0: return QSize(300,20);
				case 1: return QSize(80,20);
				case 2: return QSize(160,20);
				case 3: return QSize(400,20);
				default: return QVariant();
			}
		default:
			return QVariant();
	}
}

QVariant AspectTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();
	AbstractAspect *aspect = static_cast<AbstractAspect*>(index.internalPointer());
	switch(role) {
		case Qt::DisplayRole:
		case Qt::EditRole:
			switch(index.column()) {
				case 0: return aspect->name();
				case 1: return aspect->metaObject()->className();
				case 2: return aspect->creationTime().toString();
				case 3: return aspect->comment();
				default: return QVariant();
			}
		case Qt::ToolTipRole:
			return aspect->caption();
		case Qt::DecorationRole:
			return index.column() == 0 ? aspect->icon() : QIcon();
		case ContextMenuRole:
			return QVariant::fromValue(static_cast<QWidget*>(aspect->createContextMenu()));
		default:
			return QVariant();
	}
}

Qt::ItemFlags AspectTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid()) return 0;
	Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	if (index.column() == 0 || index.column() == 3)
		result |= Qt::ItemIsEditable;
	return result;
}

void AspectTreeModel::aspectDescriptionChanged(const AbstractAspect *aspect)
{
	emit dataChanged(modelIndexOfAspect(aspect), modelIndexOfAspect(aspect, 3));
}

void AspectTreeModel::aspectAboutToBeAdded(const AbstractAspect *parent, int index)
{
	beginInsertRows(modelIndexOfAspect(parent), index, index);
}

void AspectTreeModel::aspectAdded(const AbstractAspect *parent, int index)
{
	Q_UNUSED(index)
	endInsertRows();
	emit dataChanged(modelIndexOfAspect(parent), modelIndexOfAspect(parent, 3));
}

void AspectTreeModel::aspectAboutToBeRemoved(const AbstractAspect *parent, int index)
{
	beginRemoveRows(modelIndexOfAspect(parent), index, index);
}

void AspectTreeModel::aspectRemoved(const AbstractAspect *parent, int index)
{
	Q_UNUSED(parent); Q_UNUSED(index);
	endRemoveRows();
}

bool AspectTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid() || role != Qt::EditRole) return false;
	AbstractAspect *aspect = static_cast<AbstractAspect*>(index.internalPointer());
	switch (index.column()) {
		case 0:
			aspect->setName(value.toString());
			break;
		case 3:
			aspect->setComment(value.toString());
			break;
		default:
			return false;
	}
	emit dataChanged(index, index);
	return true;
}
