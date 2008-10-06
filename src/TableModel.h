/***************************************************************************
    File                 : TableModel.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : model for table data
                           
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

#include <QList>
#include <QString>
#include <QAbstractTableModel>

class TableModel : public QAbstractTableModel
{
public:
	TableModel(QObject *parent=0);
	void setRowCount(int c);
	void setColumnCount(int c);
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
	bool insertColumns(int col, int count, const QModelIndex &parent = QModelIndex());
	bool removeColumns(int col, int count, const QModelIndex &parent = QModelIndex());
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

	QModelIndex index(int row, int column, const QModelIndex & = QModelIndex()) const { return createIndex(row, column); }
	QModelIndex parent(const QModelIndex &) const { return QModelIndex(); }
	Qt::ItemFlags flags(const QModelIndex &index) const;
private:
	QList< QList<QString> > table;
	QList< QString> header;
//	QList< QList<bool> > masked;	TODO
};

#endif // TABLEMODEL_H
