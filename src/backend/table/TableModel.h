/***************************************************************************
    File                 : TableModel.h
    Project              : SciDAVis
    Description          : Model for the access to a Table
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs*gmx.net)
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

#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QStringList>
#include "core/AbstractFilter.h"
#include <QColor>

class Column;
class Table;

//! Model for the access to a Table
/**
	This is a model in the sense of Qt4 model/view framework which is used 
	to access a Table object from any of Qt4s view classes, typically a QTableView. 
	Its main purposes are translating Table signals into QAbstractItemModel signals
	and translating calls to the QAbstractItemModel read/write API into calls
	in the public API of Table. In many cases a pointer to the addressed column
	is obtained by calling Table::column() and the manipulation is done using the
	public API of column. 
  */
class TableModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		//! Constructor
		explicit TableModel(Table * table);
		//! Destructor
		~TableModel();

		//! Custom data roles used in addition to Qt::ItemDataRole
		enum CustomDataRole {
			MaskingRole = Qt::UserRole, //!< bool determining whether the cell is masked
			FormulaRole = Qt::UserRole+1, //!< the cells formula
			CommentRole = Qt::UserRole+2, //!< the column comment (for headerData())
		};

		//! \name Overloaded functions from QAbstractItemModel
		//@{
		Qt::ItemFlags flags( const QModelIndex & index ) const;
		QVariant data(const QModelIndex &index, int role) const;
		QVariant headerData(int section, 
				Qt::Orientation orientation,int role) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
		bool setData(const QModelIndex & index, const QVariant & value, int role);
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & child) const;
		//@}

		Column * column(int index); // this is needed for the comment header view

		void activateFormulaMode(bool on);
		bool formulaModeActive() const { return m_formula_mode; }

	private slots:
		//! \name Handlers for events from Table
		//@{
		void handleColumnsAboutToBeInserted(int, QList<Column*>);
		void handleColumnsInserted(int first, int count);
		void handleColumnsAboutToBeRemoved(int first, int count);
		void handleColumnsRemoved(int first, int count);
		void handleRowsAboutToBeInserted(int before, int count);
		void handleRowsInserted(int first, int count);
		void handleRowsAboutToBeRemoved(int first, int count);
		void handleRowsRemoved(int first, int count);
		void handleDataChanged(int top, int left, int bottom, int right);
		//@}

	private:
		Table * m_table;
		//! Toggle flag for formula mode
		bool m_formula_mode;
}; 

#endif
