/***************************************************************************
    File                 : MatrixModel.h
    Project              : LabPlot
    Description          : Matrix data model
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2009 Tilman Benkert (thzs@gmx.net)

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

class Matrix;

class MatrixModel : public QAbstractItemModel {
	Q_OBJECT

public:
	explicit MatrixModel(Matrix*);

	//! \name Overloaded functions from QAbstractItemModel
	//@{
	Qt::ItemFlags flags(const QModelIndex&) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& child) const override;
	//@}

	void updateHeader();
	void setSuppressDataChangedSignal(bool);
	void setChanged();

private slots:
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

private:
	Matrix* m_matrix;
	bool m_suppressDataChangedSignal{false};

signals:
	void changed();
};

#endif
