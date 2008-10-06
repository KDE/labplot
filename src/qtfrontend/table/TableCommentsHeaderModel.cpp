/***************************************************************************
    File                 : TableCommentsHeaderModel.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
    Email (use @ for *)  : thzs*gmx.net
    Description          : Model wrapping a TableModel to display column 
                           comments in a TableCommentsHeaderView

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

#include "TableCommentsHeaderModel.h"

TableCommentsHeaderModel::TableCommentsHeaderModel( TableModel * table_model, QObject * parent )
	: QAbstractTableModel( parent ), m_table_model(table_model)
{
	connect(m_table_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
		this, SIGNAL(headerDataChanged(Qt::Orientation,int,int)));
	connect(m_table_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
		this, SIGNAL(headerDataChanged(Qt::Orientation,int,int)));
	connect(m_table_model, SIGNAL(columnsAboutToBeInserted(const QModelIndex&,int,int)),
		this, SIGNAL(columnsAboutToBeInserted(const QModelIndex&,int,int)));
	connect(m_table_model, SIGNAL(columnsAboutToBeRemoved(const QModelIndex&,int,int)),
		this, SIGNAL(columnsAboutToBeRemoved(const QModelIndex&,int,int)));
	connect(m_table_model, SIGNAL(columnsInserted(const QModelIndex&,int,int)),
		this, SIGNAL(columnsInserted(const QModelIndex&,int,int)));
	connect(m_table_model, SIGNAL(columnsRemoved(const QModelIndex&,int,int)),
		this, SIGNAL(columnsRemoved(const QModelIndex&,int,int)));
}

TableCommentsHeaderModel::~TableCommentsHeaderModel()
{
}

Qt::ItemFlags TableCommentsHeaderModel::flags(const QModelIndex & index ) const
{
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else
		return Qt::ItemIsEnabled;
}


QVariant TableCommentsHeaderModel::data(const QModelIndex &index, int role) const
{
	Q_UNUSED(index);
	Q_UNUSED(role);
	return QVariant();
}

QVariant TableCommentsHeaderModel::headerData(int section, Qt::Orientation orientation,
		int role) const
{
	if( orientation != Qt::Horizontal || role != Qt::DisplayRole || section < 0 || section >= columnCount())
		return QVariant();

	return QVariant(m_table_model->headerData(section, Qt::Horizontal, TableModel::CommentRole));
}

int TableCommentsHeaderModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return m_table_model->rowCount();
}

int TableCommentsHeaderModel::columnCount(const QModelIndex & parent) const
{
	Q_UNUSED(parent)
	return m_table_model->columnCount();
}

