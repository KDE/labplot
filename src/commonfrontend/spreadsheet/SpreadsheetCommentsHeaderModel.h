/***************************************************************************
    File                 : SpreadsheetCommentsHeaderModel.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
    Email (use @ for *)  : thzs*gmx.net
    Description          : Model wrapping a SpreadsheetModel to display column 
                           comments in a SpreadsheetCommentsHeaderView

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
#include <spreadsheet/SpreadsheetModel.h>

class SpreadsheetCommentsHeaderModel : public QAbstractTableModel{
	Q_OBJECT

	public:
		explicit SpreadsheetCommentsHeaderModel( SpreadsheetModel * spreadsheet_model, QObject * parent = 0 );
		virtual ~SpreadsheetCommentsHeaderModel();

		//! \name Overloaded functions from QAbstractItemModel
		//@{
		Qt::ItemFlags flags( const QModelIndex & index ) const;
		QVariant data(const QModelIndex &index, int role) const;
		QVariant headerData(int section, 
				Qt::Orientation orientation,int role) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
		//@}

	private:
		SpreadsheetModel * m_spreadsheet_model;
};

#endif
