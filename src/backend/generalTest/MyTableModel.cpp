/***************************************************************************
	File                 : MyTableModel.cpp
	Project              : LabPlot
	Description          : Derived class of QStandardItemModel
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)

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

#include "backend/generalTest/MyTableModel.h"

#include <QStandardItemModel>

MyTableModel::MyTableModel(QObject* parent) : QStandardItemModel(parent) {
}

QVariant MyTableModel::data(const QModelIndex &index, int role) const {
	if (role == Qt::FontRole && (index.column() == 0 || index.row() == 0)) {
		QFont font;
		font.setBold(true);
		return font;
	} else if (role == Qt::ForegroundRole) {
		if (index.row() == 0)
			return QColor(Qt::white);
		if (index.column() == 0)
			return QColor(Qt::black);
	}
	else if (role == Qt::BackgroundRole) {
		if (index.row() == 0)
			return QColor(0x008b8b);
		if (index.column() == 0)
			return QColor(Qt::cyan);
	}

	return inherited::data(index, role);
}



