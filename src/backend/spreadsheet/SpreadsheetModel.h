/***************************************************************************
    File                 : SpreadsheetModel.h
    Project              : SciDAVis
    Description          : Model for the access to a Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2009 Knut Franke (knut.franke*gmx.de)
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

#ifndef SPREADSHEETMODEL_H
#define SPREADSHEETMODEL_H

#include <QAbstractItemModel>
#include <QStringList>

class Column;
class Spreadsheet;
class AbstractAspect;
class AbstractColumn;

//! Model for the access to a Spreadsheet
/**
	This is a model in the sense of Qt4 model/view framework which is used 
	to access a Spreadsheet object from any of Qt4s view classes, typically a QTableView. 
	Its main purposes are translating Spreadsheet signals into QAbstractItemModel signals
	and translating calls to the QAbstractItemModel read/write API into calls
	in the public API of Spreadsheet. In many cases a pointer to the addressed column
	is obtained by calling Spreadsheet::column() and the manipulation is done using the
	public API of column. 
  */
class SpreadsheetModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		//! Constructor
		explicit SpreadsheetModel(Spreadsheet * spreadsheet);
		//! Destructor
		~SpreadsheetModel();

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
		bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & child) const;
		//@}

		Column * column(int index); // this is needed for the comment header view

		void activateFormulaMode(bool on);
		bool formulaModeActive() const { return m_formula_mode; }

	private slots:
		//! \name Column insert/remove event handlers
		//@{
		void handleAspectAboutToBeAdded(const AbstractAspect * parent, const AbstractAspect * before, const AbstractAspect * child);
		void handleAspectAdded(const AbstractAspect * aspect);
		void handleAspectAboutToBeRemoved(const AbstractAspect * aspect);
		void handleAspectRemoved(const AbstractAspect * parent, const AbstractAspect * before, const AbstractAspect * child);
		//@}
		//! \name Column event handlers
		//@{
		void handleDescriptionChange(const AbstractAspect * aspect);
		void handleModeChange(const AbstractColumn * col);
		void handlePlotDesignationChange(const AbstractColumn * col);
		void handleDataChange(const AbstractColumn * col);
		void handleRowsInserted(const AbstractColumn * col, int before, int count);
		void handleRowsRemoved(const AbstractColumn * col, int first, int count);
		//@}

	protected:
		void updateVerticalHeader();
		void updateHorizontalHeader();

	private:
		Spreadsheet * m_spreadsheet;
		//! Toggle flag for formula mode
		bool m_formula_mode;
		//! Vertical header data
		QStringList m_vertical_header_data;
		//! Horizontal header data
		QStringList m_horizontal_header_data;
}; 

#endif
