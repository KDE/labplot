/*
    File                 : SpreadsheetCommentsHeaderModel.h
    Project              : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Tilman Benkert (thzs@gmx.net)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef SPREADSHEETCOMMENTSHEADERMODEL_H
#define SPREADSHEETCOMMENTSHEADERMODEL_H

#include <QAbstractTableModel>
#include <backend/spreadsheet/SpreadsheetModel.h>

class SpreadsheetCommentsHeaderModel : public QAbstractTableModel {
	Q_OBJECT

public:
	explicit SpreadsheetCommentsHeaderModel(SpreadsheetModel* , QObject* parent = nullptr);

	Qt::ItemFlags flags( const QModelIndex&) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

private:
	SpreadsheetModel* m_spreadsheet_model;
};

#endif
