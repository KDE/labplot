/*
	File                 : SpreadsheetCommentsHeaderModel.cpp
	Project              : LabPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetSparkLineHeaderModel.h"

/*!
   \class SpreadsheetSparkLineHeaderModel
   \brief Model class wrapping a SpreadsheetModel to display column SparkLine in a SpreadsheetSparkLineHeaderView

\ingroup commonfrontend
*/

SpreadsheetSparkLinesHeaderModel::SpreadsheetSparkLinesHeaderModel(SpreadsheetModel* spreadsheet_model, QObject* parent)
	: QAbstractTableModel(parent)
	, m_spreadsheet_model(spreadsheet_model) {
	connect(m_spreadsheet_model, &SpreadsheetModel::headerDataChanged, this, &SpreadsheetSparkLinesHeaderModel::headerDataChanged);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsAboutToBeInserted, this, &SpreadsheetSparkLinesHeaderModel::columnsAboutToBeInserted);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsAboutToBeRemoved, this, &SpreadsheetSparkLinesHeaderModel::columnsAboutToBeRemoved);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsInserted, this, &SpreadsheetSparkLinesHeaderModel::columnsInserted);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsRemoved, this, &SpreadsheetSparkLinesHeaderModel::columnsRemoved);
}

Qt::ItemFlags SpreadsheetSparkLinesHeaderModel::flags(const QModelIndex& index) const {
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else
		return Qt::ItemIsEnabled;
}

QVariant SpreadsheetSparkLinesHeaderModel::data(const QModelIndex& /*index*/, int /*role*/) const {
	return {};
}

QVariant SpreadsheetSparkLinesHeaderModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole || section < 0 || section >= columnCount())
		return {};

	return {m_spreadsheet_model->headerData(section, Qt::Horizontal, static_cast<int>(SpreadsheetModel::CustomDataRole::SparkLineRole))};
}

int SpreadsheetSparkLinesHeaderModel::rowCount(const QModelIndex& /*parent*/) const {
	return m_spreadsheet_model->rowCount();
}

int SpreadsheetSparkLinesHeaderModel::columnCount(const QModelIndex& /*parent*/) const {
	return m_spreadsheet_model->columnCount();
}
