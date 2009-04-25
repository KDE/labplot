/***************************************************************************
    File                 : SpreadsheetItemDelegate.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
    Email (use @ for *)  : thzs*gmx.net
    Description          : Item delegate for SpreadsheetView

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

#include <QPainter>
#include <QModelIndex>
#include "spreadsheet/SpreadsheetItemDelegate.h"
#include "spreadsheet/SpreadsheetModel.h"

SpreadsheetItemDelegate::SpreadsheetItemDelegate(QObject * parent)
 : QItemDelegate(parent)
{
	m_masking_color = QColor(0xff,0,0);
}

void SpreadsheetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index) const
{
	QItemDelegate::paint(painter, option, index);
	if (!index.data(SpreadsheetModel::MaskingRole).toBool())
		return;
	painter->save();
	// masked cells are displayed as hatched
	painter->fillRect(option.rect, QBrush(m_masking_color, Qt::BDiagPattern));
	painter->restore();
}

void SpreadsheetItemDelegate::setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const
{
	model->setData(index, editor->metaObject()->userProperty().read(editor), Qt::EditRole);
}

void SpreadsheetItemDelegate::setEditorData ( QWidget * editor, const QModelIndex & index ) const
{
	editor->metaObject()->userProperty().write(editor, index.data(Qt::EditRole));
}

