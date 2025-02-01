/***************************************************************************
	File                 : TableModel.h
	Project              : LabPlot
	Description          : Derived class of QStandardItemModel with custom styling
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal
						   (agarwaldevanshu8@gmail.com)
						   (C) 2025 Kuntal Bar
						   (barkuntal6@gmail.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA  02110-1301  USA
***************************************************************************/

#ifndef MYTABLEMODEL_H
#define MYTABLEMODEL_H

#include <QStandardItemModel>

class TableModel : public QStandardItemModel {
	Q_OBJECT

public:
	explicit TableModel(QObject* parent = nullptr);

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

#endif // MYTABLEMODEL_H
