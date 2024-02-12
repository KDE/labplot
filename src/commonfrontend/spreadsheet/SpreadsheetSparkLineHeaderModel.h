/*
	File                 : SpreadsheetSparkLineHeaderModel.h
	Project              : LabPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPREADSHEETSPARKLINEHEADERMODEL_H
#define SPREADSHEETSPARKLINEHEADERMODEL_H

#include <backend/spreadsheet/SpreadsheetModel.h>

class SpreadsheetSparkLinesHeaderModel : public QAbstractTableModel {
	Q_OBJECT

public:
	explicit SpreadsheetSparkLinesHeaderModel(SpreadsheetModel*, QObject* parent = nullptr);

	Qt::ItemFlags flags(const QModelIndex&) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	// show sparkLine of respective column
	static QPixmap showSparkLines(Column* col);

	static void sparkLine(Column* col);

	SpreadsheetModel* spreadsheetModel();

private:
	SpreadsheetModel* m_spreadsheet_model;
};

#endif
