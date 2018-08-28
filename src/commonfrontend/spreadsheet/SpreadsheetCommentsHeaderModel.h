/***************************************************************************
    File                 : SpreadsheetCommentsHeaderModel.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs@gmx.net)

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
