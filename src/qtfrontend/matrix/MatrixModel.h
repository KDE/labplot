/***************************************************************************
    File                 : MatrixModel.h
    Project              : SciDAVis
    Description          : Model for the access to a Matrix
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
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

#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include <QAbstractItemModel>
#include <QVector>
#include <QColor>

class Matrix;

//! Model for the access to a Matrix
/**
	This is a model in the sense of Qt4 model/view framework which is used 
	to access a Matrix object from any of Qt4s view classes, typically a QMatrixView. 
	Its main purposes are translating Matrix signals into QAbstractItemModel signals
	and translating calls to the QAbstractItemModel read/write API into calls
	in the public API of Matrix.   
*/
class MatrixModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		//! Constructor
		explicit MatrixModel(Matrix * matrix);
		//! Destructor
		~MatrixModel();

		//! \name Overloaded functions from QAbstractItemModel
		//@{
		Qt::ItemFlags flags( const QModelIndex & index ) const;
		QVariant data(const QModelIndex &index, int role) const;
		QVariant headerData(int section, 
				Qt::Orientation orientation,int role = Qt::DisplayRole) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex & parent = QModelIndex()) const;
		bool setData(const QModelIndex & index, const QVariant & value, int role);
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex & child) const;
		//@}

	private slots:
		//! \name Handlers for events from Matrix
		//@{
		void handleColumnsAboutToBeInserted(int before, int count);
		void handleColumnsInserted(int first, int count);
		void handleColumnsAboutToBeRemoved(int first, int count);
		void handleColumnsRemoved(int first, int count);
		void handleRowsAboutToBeInserted(int before, int count);
		void handleRowsInserted(int first, int count);
		void handleRowsAboutToBeRemoved(int first, int count);
		void handleRowsRemoved(int first, int count);
		void handleDataChanged(int top, int left, int bottom, int right);
		void handleCoordinatesChanged();
		void handleFormatChanged();
		//@}

	private:
		Matrix * m_matrix;
}; 

#endif
