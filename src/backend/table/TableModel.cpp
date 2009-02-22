/***************************************************************************
    File                 : TableModel.cpp
    Project              : SciDAVis
    Description          : Model for the access to a Table
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2009 Knut Franke (knut.franke*gmx.de)
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
	updateVerticalHeader();
	updateHorizontalHeader();

	connect(m_table, SIGNAL(aspectAboutToBeAdded(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
			this, SLOT(handleAspectAboutToBeAdded(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)));
	connect(m_table, SIGNAL(aspectAdded(const AbstractAspect*)),
			this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(m_table, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
			this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
	connect(m_table, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
			this, SLOT(handleAspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)));
	connect(m_table, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),
			this, SLOT(handleDescriptionChange(const AbstractAspect*)));
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
					return QVariant(tr("-","string for invalid cells"));
				
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
	switch(orientation) {
		case Qt::Horizontal:
			if (section < 0 || section >= m_table->columnCount())
				return QVariant();
			switch(role) {
				case Qt::DisplayRole:
				case Qt::ToolTipRole:
				case Qt::EditRole:
					return m_horizontal_header_data.at(section);
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
				case Qt::DecorationRole:
					return m_table->child<Column>(section)->icon();
#endif
				case TableModel::CommentRole:
					return m_table->child<Column>(section)->comment();
				case Qt::SizeHintRole:
					return QSize(m_table->child<Column>(section)->width(), 20);
			}
		case Qt::Vertical:
			if (section < 0 || section >= m_table->rowCount())
				return QVariant();
			switch(role) {
				case Qt::DisplayRole:
				case Qt::ToolTipRole:
					return m_vertical_header_data.at(section);
			}
	}
	return QVariant();
}

bool TableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
	if (orientation == Qt::Horizontal && role == Qt::SizeHintRole) {
		m_table->child<Column>(section)->setWidth(value.toSize().width());
		emit headerDataChanged(Qt::Horizontal, section, section);
	} else
		QAbstractItemModel::setHeaderData(section, orientation, value, role);
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

void TableModel::handleAspectAboutToBeAdded(const AbstractAspect * parent, const AbstractAspect * before, const AbstractAspect * new_child)
{
	const Column * col = qobject_cast<const Column*>(new_child);
	if (!col || parent != static_cast<AbstractAspect*>(m_table))
		return;

	int index = before ? m_table->indexOfChild<Column>(before) : 0;
	beginInsertColumns(QModelIndex(), index, index);
}

void TableModel::handleAspectAdded(const AbstractAspect * aspect)
{
	const Column * col = qobject_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != static_cast<AbstractAspect*>(m_table))
		return;

	int old_rows = m_vertical_header_data.size();

	updateVerticalHeader();
	updateHorizontalHeader();
	endInsertColumns();

	emit headerDataChanged(Qt::Horizontal, 0, m_table->columnCount()-1);
	//emit headerDataChanged(Qt::Vertical, old_rows, m_table->rowCount()-1);
	emit headerDataChanged(Qt::Vertical, 0, m_table->rowCount()-1);

	reset();

	connect(col, SIGNAL(plotDesignationChanged(const AbstractColumn *)), this,
			SLOT(handlePlotDesignationChange(const AbstractColumn *)));
	connect(col, SIGNAL(modeChanged(const AbstractColumn *)), this,
			SLOT(handleDataChange(const AbstractColumn *)));
	connect(col, SIGNAL(dataChanged(const AbstractColumn *)), this,
			SLOT(handleDataChange(const AbstractColumn *)));
	connect(col, SIGNAL(modeChanged(const AbstractColumn *)), this,
			SLOT(handleModeChange(const AbstractColumn *)));
	connect(col, SIGNAL(rowsInserted(const AbstractColumn *, int, int)), this,
			SLOT(handleRowsInserted(const AbstractColumn *,int,int)));
	connect(col, SIGNAL(rowsRemoved(const AbstractColumn *, int, int)), this,
			SLOT(handleRowsRemoved(const AbstractColumn *,int,int)));
	connect(col, SIGNAL(maskingChanged(const AbstractColumn *)), this,
			SLOT(handleDataChange(const AbstractColumn *)));
}

void TableModel::handleAspectAboutToBeRemoved(const AbstractAspect * aspect)
{
	const Column * col = qobject_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != static_cast<AbstractAspect*>(m_table))
		return;

	int index = m_table->indexOfChild<Column>(col);
	beginRemoveColumns(QModelIndex(), index, index);
	disconnect(col, 0, this, 0);
}

void TableModel::handleAspectRemoved(const AbstractAspect * parent, const AbstractAspect * before, const AbstractAspect * child)
{
	Q_UNUSED(before)
	const Column * col = qobject_cast<const Column*>(child);
	if (!col || parent != static_cast<AbstractAspect*>(m_table))
		return;

	int old_rows = m_vertical_header_data.size();

	updateVerticalHeader();
	updateHorizontalHeader();
	endRemoveColumns();

	emit headerDataChanged(Qt::Horizontal, 0, m_table->columnCount()-1);
	emit headerDataChanged(Qt::Vertical, old_rows, m_table->rowCount()-1);

	reset();
}

void TableModel::handleDescriptionChange(const AbstractAspect * aspect)
{
	const Column * col = qobject_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != static_cast<AbstractAspect*>(m_table))
		return;
	updateHorizontalHeader();
	int index = m_table->indexOfChild<Column>(col);
	emit headerDataChanged(Qt::Horizontal, index, index);
}

void TableModel::handleModeChange(const AbstractColumn * col)
{
	updateHorizontalHeader();
	int index = m_table->indexOfChild<Column>(col);
	emit headerDataChanged(Qt::Horizontal, index, index);
}

void TableModel::handlePlotDesignationChange(const AbstractColumn * col)
{
	updateHorizontalHeader();
	int index = m_table->indexOfChild<Column>(col);
	emit headerDataChanged(Qt::Horizontal, index, m_table->columnCount()-1);
}

void TableModel::handleDataChange(const AbstractColumn * col)
{
	int i = m_table->indexOfChild<Column>(col);
	emit dataChanged(index(0, i), index(col->rowCount()-1, i));
}

void TableModel::handleRowsInserted(const AbstractColumn * col, int before, int count)
{
	Q_UNUSED(before) Q_UNUSED(count)
	updateVerticalHeader();
	int i = m_table->indexOfChild<Column>(col);
	emit dataChanged(index(0, i), index(col->rowCount()-1, i));
}

void TableModel::handleRowsRemoved(const AbstractColumn * col, int first, int count)
{
	Q_UNUSED(first) Q_UNUSED(count)
	int i = m_table->indexOfChild<Column>(col);
	emit dataChanged(index(0, i), index(col->rowCount()-1, i));
}

void TableModel::updateVerticalHeader()
{
	int old_rows = m_vertical_header_data.size();
	int new_rows = m_table->rowCount();
	if (new_rows > old_rows) {
		for(int i=old_rows+1; i<=new_rows; i++)
			m_vertical_header_data << QString::number(i);
	} else
		while (m_vertical_header_data.size() > new_rows)
			m_vertical_header_data.removeLast();

	Q_ASSERT(m_vertical_header_data.size() == m_table->rowCount());
}

void TableModel::updateHorizontalHeader()
{
	int column_count = m_table->childCount<Column>();

	while(m_horizontal_header_data.size() < column_count)
		m_horizontal_header_data << QString();
	while(m_horizontal_header_data.size() > column_count)
		m_horizontal_header_data.removeLast();

	int x_cols;
	if(m_table->columnCount(SciDAVis::X) <= 1)
		x_cols = -1;
	else
		x_cols = 0;

	for (int i=0; i<column_count; i++) {
		Column * col = m_table->child<Column>(i);

		QString middle_section;
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		switch(col->dataType()) {
				case SciDAVis::TypeDouble:
			middle_section = " {numeric} ";
			break;
				case SciDAVis::TypeQString:
			middle_section = " {text} ";
			break;
				case SciDAVis::TypeQDateTime:
			middle_section = " {datetime} ";
			break;
		}
#endif
		QString designation_section;
		switch(col->plotDesignation()) {
			case SciDAVis::X:
				designation_section = x_cols>-1 ? QString("[X%1]").arg(++x_cols) : QString("[X]");
				break;

			case SciDAVis::Y:
				designation_section = x_cols>0 ? QString("[Y%1]").arg(x_cols) : QString("[Y]");
				break;

			case SciDAVis::Z:
				designation_section = x_cols>0 ? QString("[Z%1]").arg(x_cols) : QString("[Z]");
				break;

			case SciDAVis::xErr:
				designation_section = x_cols>0 ? QString("[xEr%1]").arg(x_cols) : QString("[xEr]");
				break;

			case SciDAVis::yErr:
				designation_section = x_cols>0 ? QString("[yEr%1]").arg(x_cols) : QString("[yEr]");
				break;
		}

		m_horizontal_header_data.replace(i, col->name() + middle_section + designation_section);
	}

	Q_ASSERT(m_horizontal_header_data.size() == m_table->columnCount());
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
		emit dataChanged(index(0,0), index(rows-1,cols-1));
}
