/***************************************************************************
	File                 : TableModel.cpp
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

#include "TableModel.h"

#include <QFont>
#include <QColor>

TableModel::TableModel(QObject* parent)
	: QStandardItemModel(parent)
{
}

QVariant TableModel::data(const QModelIndex& index, int role) const {
	// Verify that the index is valid
	if (!index.isValid())
		return QVariant();

	// For header cells (first row or first column), use a bold font.
	if (role == Qt::FontRole && (index.row() == 0 || index.column() == 0)) {
		QFont headerFont;
		headerFont.setBold(true);
		return headerFont;
	}

		   // Set custom foreground colors.
	if (role == Qt::ForegroundRole) {
		if (index.row() == 0)
			return QColor(Qt::white);
		else if (index.column() == 0)
			return QColor(Qt::black);
	}

	// Set custom background colors.
	if (role == Qt::BackgroundRole) {
		if (index.row() == 0)
			return QColor::fromRgb(0x00, 0x8B, 0x8B); // Dark Cyan-like color.
		else if (index.column() == 0)
			return QColor(Qt::cyan);
	}

	// For all other roles, use the base class implementation.
	return QStandardItemModel::data(index, role);
}
