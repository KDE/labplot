/***************************************************************************
    File                 : TableModel.cpp
    Project              : SciDAVis
    Description          : Model for the access to a Table
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs*gmx.net)
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

#include "core/column/Column.h"
#include "table/Table.h"
#include "table/TableModel.h"
#include <QString>
#include <QBrush>
#include <QIcon>
#include <QPixmap>

TableModel::TableModel(Table * table)
	: QAbstractItemModel(0), m_table(table), m_formula_mode(false)
{
	connect(m_table, SIGNAL(columnsAboutToBeInserted(int, QList<Column *>)),
			this, SLOT(handleColumnsAboutToBeInserted(int, QList<Column *>)));
	connect(m_table, SIGNAL(columnsInserted(int, int)),
			this, SLOT(handleColumnsInserted(int, int)));
	connect(m_table, SIGNAL(columnsAboutToBeRemoved(int, int)),
			this, SLOT(handleColumnsAboutToBeRemoved(int, int)));
	connect(m_table, SIGNAL(columnsRemoved(int, int)),
			this, SLOT(handleColumnsRemoved(int, int)));
	connect(m_table, SIGNAL(rowsAboutToBeInserted(int, int)),
			this, SLOT(handleRowsAboutToBeInserted(int, int)));
	connect(m_table, SIGNAL(rowsInserted(int, int)),
			this, SLOT(handleRowsInserted(int, int)));
	connect(m_table, SIGNAL(rowsAboutToBeRemoved(int, int)),
			this, SLOT(handleRowsAboutToBeRemoved(int, int)));
	connect(m_table, SIGNAL(rowsRemoved(int, int)),
			this, SLOT(handleRowsRemoved(int, int)));
	connect(m_table, SIGNAL(dataChanged(int, int, int, int)),
			this, SLOT(handleDataChanged(int, int, int, int)));
	connect(m_table, SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
			this, SIGNAL(headerDataChanged(Qt::Orientation, int, int)));
}

TableModel::~TableModel()
{
}

Qt::ItemFlags TableModel::flags(const QModelIndex & index ) const
{
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	else
		return Qt::ItemIsEnabled;
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
	if( !index.isValid() )
		return QVariant();
	
	int row = index.row();
	int col = index.column();
	Column * col_ptr = m_table->column(col);
	if(!col_ptr)
		return QVariant();

	QString postfix;
	switch(role)
	{
		case Qt::ToolTipRole:
				if(col_ptr->isMasked(row))
					postfix = " " + tr("(masked)");
				if(col_ptr->isInvalid(row))
					return QVariant(tr("invalid cell (ignored in all operations)","tooltip string for invalid rows") + postfix);
		case Qt::EditRole:
				if(!m_formula_mode && col_ptr->isInvalid(row))
					return QVariant();
		case Qt::DisplayRole:
			{
				if(m_formula_mode)
					return QVariant(col_ptr->formula(row));
				if(col_ptr->isInvalid(row))
					return QVariant(tr("-","string for invalid rows"));
				
				return QVariant(col_ptr->asStringColumn()->textAt(row) + postfix);
			}
		case Qt::ForegroundRole:
			{
				if(col_ptr->isInvalid(index.row()))
					return QVariant(QBrush(QColor(0xff,0,0))); // invalid -> red letters
				else
					return QVariant(QBrush(QColor(0,0,0)));
			}
		case MaskingRole:
			return QVariant(col_ptr->isMasked(row));
		case FormulaRole:
			return QVariant(col_ptr->formula(row));
		case Qt::DecorationRole:
			if(m_formula_mode)
				return QIcon(QPixmap(":/equals.png"));
			break;
	}

	return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	return m_table->headerData(section, orientation, role);
}

int TableModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return m_table->rowCount();
}

int TableModel::columnCount(const QModelIndex & parent) const
{
	Q_UNUSED(parent)
	return m_table->columnCount();
}

bool TableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	if (!index.isValid())
		return false;

	int row = index.row();

	switch(role)
	{  
		case Qt::EditRole:
			{
				Column* col_ptr = m_table->column(index.column());
				// remark: the validity of the cell is determined by the input filter
				if (m_formula_mode)
					col_ptr->setFormula(row, value.toString());
				else
					col_ptr->asStringColumn()->setTextAt(row, value.toString());
				return true;
			}
		case MaskingRole:
			{
				m_table->column(index.column())->setMasked(row, value.toBool());  
				return true;
			}
		case FormulaRole:
			{
				m_table->column(index.column())->setFormula(row, value.toString());  
				return true;
			}
	}
	
	return false;
}

QModelIndex TableModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return createIndex(row, column);
}

QModelIndex TableModel::parent(const QModelIndex & child) const
{
	Q_UNUSED(child)
    return QModelIndex();
}

void TableModel::handleColumnsAboutToBeInserted(int before, QList<Column *> cols)
{
	int count = cols.count();

	beginInsertColumns(QModelIndex(), before, before+count-1);
}

void TableModel::handleColumnsInserted(int first, int count)
{
	Q_UNUSED(first)
	Q_UNUSED(count)
	endInsertColumns();
}

void TableModel::handleColumnsAboutToBeRemoved(int first, int count)
{
	beginRemoveColumns(QModelIndex(), first, first+count-1);
}

void TableModel::handleColumnsRemoved(int first, int count)
{
	Q_UNUSED(first)
	Q_UNUSED(count)
	endRemoveColumns();	 
}

void TableModel::handleRowsAboutToBeInserted(int before, int count)
{
	beginInsertRows(QModelIndex(), before, before+count-1);
}

void TableModel::handleRowsInserted(int first, int count)
{
	Q_UNUSED(first)
	Q_UNUSED(count)
	endInsertRows();
}

void TableModel::handleRowsAboutToBeRemoved(int first, int count)
{
	beginRemoveRows(QModelIndex(), first, first+count-1);
}

void TableModel::handleRowsRemoved(int first, int count)
{
	Q_UNUSED(first)
	Q_UNUSED(count)
	endRemoveRows();	 
}

void TableModel::handleDataChanged(int top, int left, int bottom, int right)
{
	emit dataChanged(index(top, left), index(bottom, right));
}

Column * TableModel::column(int index)
{
	return m_table->column(index);
}
		
void TableModel::activateFormulaMode(bool on)
{
	if (m_formula_mode == on) return;
	m_formula_mode = on;
	int rows = m_table->rowCount();
	int cols = m_table->columnCount();
	if (rows > 0 && cols > 0)
		handleDataChanged(0, 0, rows-1, cols-1);
}
