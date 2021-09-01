/*
    File                 : SpreadsheetCommentsHeaderModel.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Tilman Benkert (thzs@gmx.net)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

	connect(m_spreadsheet_model, &SpreadsheetModel::headerDataChanged,
		this, &SpreadsheetCommentsHeaderModel::headerDataChanged);
	connect(m_spreadsheet_model, &SpreadsheetModel::headerDataChanged,
		this, &SpreadsheetCommentsHeaderModel::headerDataChanged);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsAboutToBeInserted,
		this, &SpreadsheetCommentsHeaderModel::columnsAboutToBeInserted);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsAboutToBeRemoved,
		this, &SpreadsheetCommentsHeaderModel::columnsAboutToBeRemoved);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsInserted,
		this, &SpreadsheetCommentsHeaderModel::columnsInserted);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsRemoved,
		this, &SpreadsheetCommentsHeaderModel::columnsRemoved);
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
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole || section < 0 || section >= columnCount())
		return QVariant();

	return QVariant(m_spreadsheet_model->headerData(section, Qt::Horizontal, static_cast<int>(SpreadsheetModel::CustomDataRole::CommentRole)));
}

int SpreadsheetCommentsHeaderModel::rowCount(const QModelIndex& parent) const{
	Q_UNUSED(parent)
	return m_spreadsheet_model->rowCount();
}

int SpreadsheetCommentsHeaderModel::columnCount(const QModelIndex& parent) const{
	Q_UNUSED(parent)
	return m_spreadsheet_model->columnCount();
}
