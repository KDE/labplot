/***************************************************************************
    File                 : SpreadsheetCommentsHeaderModel.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert (thzs@gmx.net)

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

#include "commonfrontend/spreadsheet/SpreadsheetCommentsHeaderModel.h"

 /*!
	\class SpreadsheetCommentsHeaderModel
	\brief Model class wrapping a SpreadsheetModel to display column comments in a SpreadsheetCommentsHeaderView

	\ingroup commonfrontend
 */

SpreadsheetCommentsHeaderModel::SpreadsheetCommentsHeaderModel(SpreadsheetModel* spreadsheet_model, QObject* parent)
	: QAbstractTableModel(parent), m_spreadsheet_model(spreadsheet_model) {
	  
	connect(m_spreadsheet_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
		this, SIGNAL(headerDataChanged(Qt::Orientation,int,int)));
	connect(m_spreadsheet_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
		this, SIGNAL(headerDataChanged(Qt::Orientation,int,int)));
	connect(m_spreadsheet_model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
		this, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)));
	connect(m_spreadsheet_model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
		this, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)));
	connect(m_spreadsheet_model, SIGNAL(columnsInserted(QModelIndex,int,int)),
		this, SIGNAL(columnsInserted(QModelIndex,int,int)));
	connect(m_spreadsheet_model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
		this, SIGNAL(columnsRemoved(QModelIndex,int,int)));
}

Qt::ItemFlags SpreadsheetCommentsHeaderModel::flags(const QModelIndex& index ) const {
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else
		return Qt::ItemIsEnabled;
}

QVariant SpreadsheetCommentsHeaderModel::data(const QModelIndex& index, int role) const {
	Q_UNUSED(index);
	Q_UNUSED(role);
	return QVariant();
}

QVariant SpreadsheetCommentsHeaderModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if( orientation != Qt::Horizontal || role != Qt::DisplayRole || section < 0 || section >= columnCount())
		return QVariant();

	return QVariant(m_spreadsheet_model->headerData(section, Qt::Horizontal, SpreadsheetModel::CommentRole));
}

int SpreadsheetCommentsHeaderModel::rowCount(const QModelIndex& parent) const{
	Q_UNUSED(parent)
	return m_spreadsheet_model->rowCount();
}

int SpreadsheetCommentsHeaderModel::columnCount(const QModelIndex& parent) const{
	Q_UNUSED(parent)
	return m_spreadsheet_model->columnCount();
}
