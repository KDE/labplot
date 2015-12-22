/***************************************************************************
    File                 : SpreadsheetModel.cpp
    Project              : LabPlot
    Description          : Model for the access to a Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2009 Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2013-2015 Alexander Semke (alexander.semke@web.de)

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

#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/spreadsheet/SpreadsheetModel.h"

#include <QIcon>
#include <QFontMetrics>

#include <KLocale>

SpreadsheetModel::SpreadsheetModel(Spreadsheet * spreadsheet)
	: QAbstractItemModel(0), m_spreadsheet(spreadsheet), m_formula_mode(false)
{
	updateVerticalHeader();
	updateHorizontalHeader();

	QFont font;
	font.setFamily(font.defaultFamily());
	QFontMetrics fm(font);
	m_defaultHeaderHeight = fm.height()+5;

	connect(m_spreadsheet, SIGNAL(aspectAboutToBeAdded(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
			this, SLOT(handleAspectAboutToBeAdded(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(aspectAdded(const AbstractAspect*)),
			this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
			this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
			this, SLOT(handleAspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),
			this, SLOT(handleDescriptionChange(const AbstractAspect*)));

	for (int i=0; i < spreadsheet->columnCount(); i++) {
		beginInsertColumns(QModelIndex(), i, i);
		handleAspectAdded(spreadsheet->column(i));
	}
}

SpreadsheetModel::~SpreadsheetModel()
{
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex & index ) const
{
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	else
		return Qt::ItemIsEnabled;
}

QVariant SpreadsheetModel::data(const QModelIndex &index, int role) const
{
	if( !index.isValid() )
		return QVariant();

	int row = index.row();
	int col = index.column();
	Column * col_ptr = m_spreadsheet->column(col);
	if(!col_ptr)
		return QVariant();

	QString postfix;
	switch(role)
	{
		case Qt::ToolTipRole:
				if(!col_ptr->isValid(row)) {
					QString s;
					if(col_ptr->isMasked(row))
						s = i18n("invalid cell (ignored in all operations) (masked)");
					else
						s = i18n("invalid cell (ignored in all operations)");
					return QVariant(s);
				}
		case Qt::EditRole:
				if(!m_formula_mode && !col_ptr->isValid(row))
					return QVariant();
		case Qt::DisplayRole:
			{
				if(m_formula_mode)
					return QVariant(col_ptr->formula(row));
				if(!col_ptr->isValid(row))
					return QVariant("-");

				return QVariant(col_ptr->asStringColumn()->textAt(row) + postfix);
			}
		case Qt::ForegroundRole:
			{
				if(!col_ptr->isValid(index.row()))
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

QVariant SpreadsheetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (section>m_spreadsheet->columnCount()-1)
	  return QVariant();

	switch(orientation) {
		case Qt::Horizontal:
			switch(role) {
				case Qt::DisplayRole:
				case Qt::ToolTipRole:
				case Qt::EditRole:
					return m_horizontal_header_data.at(section);
				case Qt::DecorationRole:
					return m_spreadsheet->child<Column>(section)->icon();
				case SpreadsheetModel::CommentRole:
					return m_spreadsheet->child<Column>(section)->comment();
				case Qt::SizeHintRole:
					return QSize(m_spreadsheet->child<Column>(section)->width(), m_defaultHeaderHeight);
			}
		case Qt::Vertical:
			switch(role) {
				case Qt::DisplayRole:
				case Qt::ToolTipRole:
					return m_vertical_header_data.at(section);
			}
	}
	return QVariant();
}

int SpreadsheetModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return m_spreadsheet->rowCount();
}

int SpreadsheetModel::columnCount(const QModelIndex & parent) const
{
	Q_UNUSED(parent)
	return m_spreadsheet->columnCount();
}

bool SpreadsheetModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	if (!index.isValid())
		return false;

	int row = index.row();

	switch(role)
	{
		case Qt::EditRole:
			{
				Column* col_ptr = m_spreadsheet->column(index.column());
				// remark: the validity of the cell is determined by the input filter
				if (m_formula_mode)
					col_ptr->setFormula(row, value.toString());
				else
					col_ptr->asStringColumn()->setTextAt(row, value.toString());
				return true;
			}
		case MaskingRole:
			{
				m_spreadsheet->column(index.column())->setMasked(row, value.toBool());
				return true;
			}
		case FormulaRole:
			{
				m_spreadsheet->column(index.column())->setFormula(row, value.toString());
				return true;
			}
	}

	return false;
}

QModelIndex SpreadsheetModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return createIndex(row, column);
}

QModelIndex SpreadsheetModel::parent(const QModelIndex & child) const
{
	Q_UNUSED(child)
    return QModelIndex();
}

bool SpreadsheetModel::hasChildren(const QModelIndex& parent) const {
	Q_UNUSED(parent)
	return false;
}

void SpreadsheetModel::handleAspectAboutToBeAdded(const AbstractAspect * parent, const AbstractAspect * before, const AbstractAspect * new_child)
{
//	qDebug()<<"handleAspectAboutToBeAdded";
	const Column * col = qobject_cast<const Column*>(new_child);
	if (!col || parent != static_cast<AbstractAspect*>(m_spreadsheet))
		return;

	//TODO: breaks undo/redo
	Q_UNUSED(before);
// 	int index = before ? m_spreadsheet->indexOfChild<Column>(before) : 0;
	//beginInsertColumns(QModelIndex(), index, index);
}

void SpreadsheetModel::handleAspectAdded(const AbstractAspect * aspect)
{
//	qDebug()<<"handleAspectAdded";
	const Column * col = qobject_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != static_cast<AbstractAspect*>(m_spreadsheet))
		return;

	// unused
	//int old_rows = m_vertical_header_data.size();

	updateVerticalHeader();
	updateHorizontalHeader();

	emit headerDataChanged(Qt::Horizontal, 0, m_spreadsheet->columnCount()-1);
	//emit headerDataChanged(Qt::Vertical, old_rows, m_spreadsheet->rowCount()-1);
	emit headerDataChanged(Qt::Vertical, 0, m_spreadsheet->rowCount()-1);

	connect(col, SIGNAL(plotDesignationChanged(const AbstractColumn*)), this,
			SLOT(handlePlotDesignationChange(const AbstractColumn*)));
	connect(col, SIGNAL(modeChanged(const AbstractColumn*)), this,
			SLOT(handleDataChange(const AbstractColumn*)));
	connect(col, SIGNAL(dataChanged(const AbstractColumn*)), this,
			SLOT(handleDataChange(const AbstractColumn*)));
	connect(col, SIGNAL(modeChanged(const AbstractColumn*)), this,
			SLOT(handleModeChange(const AbstractColumn*)));
	connect(col, SIGNAL(rowsInserted(const AbstractColumn*,int,int)), this,
			SLOT(handleRowsInserted(const AbstractColumn*,int,int)));
	connect(col, SIGNAL(rowsRemoved(const AbstractColumn*,int,int)), this,
			SLOT(handleRowsRemoved(const AbstractColumn*,int,int)));
	connect(col, SIGNAL(maskingChanged(const AbstractColumn*)), this,
			SLOT(handleDataChange(const AbstractColumn*)));

	beginResetModel();
	//TODO: breaks undo/redo
	//endInsertColumns();
	endResetModel();

	m_spreadsheet->emitColumnCountChanged();
}

void SpreadsheetModel::handleAspectAboutToBeRemoved(const AbstractAspect * aspect)
{
//	qDebug()<<"handleAspectAboutToBeRemoved";
	const Column * col = qobject_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != static_cast<AbstractAspect*>(m_spreadsheet))
		return;

	int index = m_spreadsheet->indexOfChild<Column>(col);
	beginRemoveColumns(QModelIndex(), index, index);
	disconnect(col, 0, this, 0);
}

void SpreadsheetModel::handleAspectRemoved(const AbstractAspect * parent, const AbstractAspect * before, const AbstractAspect * child)
{
//	qDebug()<<"handleAspectRemoved";
	Q_UNUSED(before)
	const Column * col = qobject_cast<const Column*>(child);
	if (!col || parent != static_cast<AbstractAspect*>(m_spreadsheet))
		return;

	int old_rows = m_vertical_header_data.size();

	updateVerticalHeader();
	updateHorizontalHeader();

	emit headerDataChanged(Qt::Horizontal, 0, m_spreadsheet->columnCount()-1);
	emit headerDataChanged(Qt::Vertical, old_rows, m_spreadsheet->rowCount()-1);

	beginResetModel();
	endRemoveColumns();
	endResetModel();

	m_spreadsheet->emitColumnCountChanged();
}

void SpreadsheetModel::handleDescriptionChange(const AbstractAspect * aspect)
{
	const Column * col = qobject_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != static_cast<AbstractAspect*>(m_spreadsheet))
		return;
	updateHorizontalHeader();
	int index = m_spreadsheet->indexOfChild<Column>(col);
	emit headerDataChanged(Qt::Horizontal, index, index);
}

void SpreadsheetModel::handleModeChange(const AbstractColumn * col)
{
	updateHorizontalHeader();
	int index = m_spreadsheet->indexOfChild<Column>(col);
	emit headerDataChanged(Qt::Horizontal, index, index);
}

void SpreadsheetModel::handlePlotDesignationChange(const AbstractColumn * col)
{
	updateHorizontalHeader();
	int index = m_spreadsheet->indexOfChild<Column>(col);
	emit headerDataChanged(Qt::Horizontal, index, m_spreadsheet->columnCount()-1);
}

void SpreadsheetModel::handleDataChange(const AbstractColumn * col)
{
	int i = m_spreadsheet->indexOfChild<Column>(col);
	emit dataChanged(index(0, i), index(col->rowCount()-1, i));
}

void SpreadsheetModel::handleRowsInserted(const AbstractColumn * col, int before, int count)
{
	Q_UNUSED(before) Q_UNUSED(count)
	updateVerticalHeader();
	int i = m_spreadsheet->indexOfChild<Column>(col);
	emit dataChanged(index(0, i), index(col->rowCount()-1, i));
	m_spreadsheet->emitRowCountChanged();
}

void SpreadsheetModel::handleRowsRemoved(const AbstractColumn * col, int first, int count)
{
	Q_UNUSED(first) Q_UNUSED(count)
	updateVerticalHeader();
	int i = m_spreadsheet->indexOfChild<Column>(col);
	emit dataChanged(index(0, i), index(col->rowCount()-1, i));
	m_spreadsheet->emitRowCountChanged();
}

void SpreadsheetModel::updateVerticalHeader()
{
	int old_rows = m_vertical_header_data.size();
	int new_rows = m_spreadsheet->rowCount();
	if (new_rows > old_rows) {
		beginInsertRows(QModelIndex(), old_rows, new_rows-1);
		for(int i=old_rows+1; i<=new_rows; i++)
			m_vertical_header_data << i;
		endInsertRows();
	} else if (new_rows < old_rows) {
		beginRemoveRows(QModelIndex(), new_rows, old_rows-1);
		while (m_vertical_header_data.size() > new_rows)
			m_vertical_header_data.removeLast();
		endRemoveRows();
	}

	Q_ASSERT(m_vertical_header_data.size() == m_spreadsheet->rowCount());
}

void SpreadsheetModel::updateHorizontalHeader()
{
	int column_count = m_spreadsheet->childCount<Column>();

	while(m_horizontal_header_data.size() < column_count)
		m_horizontal_header_data << QString();
	while(m_horizontal_header_data.size() > column_count)
		m_horizontal_header_data.removeLast();

// 	int x_cols;
// 	if(m_spreadsheet->columnCount(AbstractColumn::X) <= 1)
// 		x_cols = -1;
// 	else
// 		x_cols = 0;

	for (int i=0; i<column_count; i++) {
		Column * col = m_spreadsheet->child<Column>(i);

		QString middle_section;
		switch(col->columnMode()) {
			case AbstractColumn::Numeric:
				middle_section = " {numeric} ";
				break;
			case AbstractColumn::Text:
				middle_section = " {text} ";
				break;
			case AbstractColumn::DateTime:
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				middle_section = " {datetime} ";
				break;
		}

		/*
		 //TODO: activate later when plot designation is somehow used in the application
		QString designation_section;
		switch(col->plotDesignation()) {
			case AbstractColumn::X:
				designation_section = x_cols>-1 ? QString("[X%1]").arg(++x_cols) : QString("[X]");
				break;

			case AbstractColumn::Y:
				designation_section = x_cols>0 ? QString("[Y%1]").arg(x_cols) : QString("[Y]");
				break;

			case AbstractColumn::Z:
				designation_section = x_cols>0 ? QString("[Z%1]").arg(x_cols) : QString("[Z]");
				break;

			case AbstractColumn::xErr:
				designation_section = x_cols>0 ? QString("[xEr%1]").arg(x_cols) : QString("[xEr]");
				break;

			case AbstractColumn::yErr:
				designation_section = x_cols>0 ? QString("[yEr%1]").arg(x_cols) : QString("[yEr]");
				break;
			case AbstractColumn::noDesignation:
				break;
		}

		m_horizontal_header_data.replace(i, col->name() + middle_section + designation_section);
		*/
		m_horizontal_header_data.replace(i, col->name() + middle_section);
	}

	Q_ASSERT(m_horizontal_header_data.size() == m_spreadsheet->columnCount());
}

Column * SpreadsheetModel::column(int index)
{
	return m_spreadsheet->column(index);
}

void SpreadsheetModel::activateFormulaMode(bool on)
{
	if (m_formula_mode == on) return;
	m_formula_mode = on;
	int rows = m_spreadsheet->rowCount();
	int cols = m_spreadsheet->columnCount();
	if (rows > 0 && cols > 0)
		emit dataChanged(index(0,0), index(rows-1,cols-1));
}
