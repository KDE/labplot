/*
	File                 : SpreadsheetCommentsHeaderModel.cpp
	Project              : LabPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "commonfrontend/spreadsheet/SpreadsheetSparkLineHeaderModel.h"

/*!
   \class SpreadsheetSparkLineHeaderModel
   \brief Model class wrapping a SpreadsheetModel to display column SparkLine in a SpreadsheetSparkLineHeaderView

\ingroup commonfrontend
*/

SpreadsheetSparkLineHeaderModel::SpreadsheetSparkLineHeaderModel(SpreadsheetModel* spreadsheet_model, QObject* parent)
	: QAbstractTableModel(parent)
	, m_spreadsheet_model(spreadsheet_model) {
	connect(m_spreadsheet_model, &SpreadsheetModel::headerDataChanged, this, &SpreadsheetSparkLineHeaderModel::headerDataChanged);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsAboutToBeInserted, this, &SpreadsheetSparkLineHeaderModel::columnsAboutToBeInserted);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsAboutToBeRemoved, this, &SpreadsheetSparkLineHeaderModel::columnsAboutToBeRemoved);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsInserted, this, &SpreadsheetSparkLineHeaderModel::columnsInserted);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsRemoved, this, &SpreadsheetSparkLineHeaderModel::columnsRemoved);
}

Qt::ItemFlags SpreadsheetSparkLineHeaderModel::flags(const QModelIndex& index) const {
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else
		return Qt::ItemIsEnabled;
}

QVariant SpreadsheetSparkLineHeaderModel::data(const QModelIndex& /*index*/, int /*role*/) const {
	return {};
}

QVariant SpreadsheetSparkLineHeaderModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole || section < 0 || section >= columnCount())
		return {};

	return {m_spreadsheet_model->headerData(section, Qt::Horizontal, static_cast<int>(SpreadsheetModel::CustomDataRole::CommentRole))};
}

int SpreadsheetSparkLineHeaderModel::rowCount(const QModelIndex& /*parent*/) const {
	return m_spreadsheet_model->rowCount();
}

int SpreadsheetSparkLineHeaderModel::columnCount(const QModelIndex& /*parent*/) const {
	return m_spreadsheet_model->columnCount();
}
