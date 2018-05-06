/***************************************************************************
    File                 : SpreadsheetItemDelegate.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2010-2017 by Alexander Semke (alexander.semke@web.de)

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
#include "SpreadsheetItemDelegate.h"
#include "backend/spreadsheet/SpreadsheetModel.h"

#include <QAbstractItemModel>
#include <QKeyEvent>
#include <QMetaProperty>
#include <QPainter>

/*!
\class SpreadsheetItemDelegate
\brief Item delegate for SpreadsheetView.

Overides QItemDelegate::paint() and provides shaded representation
of masked cells used in SpreadsheetView.

\ingroup commonfrontend
*/

SpreadsheetItemDelegate::SpreadsheetItemDelegate(QObject* parent)  : QItemDelegate(parent),  m_maskingColor(0xff,0,0) {
		installEventFilter(this);

}

void SpreadsheetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {

	QItemDelegate::paint(painter, option, index);
	if (!index.data(SpreadsheetModel::MaskingRole).toBool())
		return;

	painter->save();
	painter->fillRect(option.rect, QBrush(m_maskingColor, Qt::BDiagPattern));
	painter->restore();
}

/*!
  Sets the color for masked cells to \c color
 */
void SpreadsheetItemDelegate::setMaskingColor(const QColor& color) {
	m_maskingColor = color;
}

/*!
  Returns the color for masked cells.
 */
QColor SpreadsheetItemDelegate::maskingColor() const {
	return m_maskingColor;
}

void SpreadsheetItemDelegate::setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const {
	model->setData(index, editor->metaObject()->userProperty().read(editor), Qt::EditRole);
}

void SpreadsheetItemDelegate::setEditorData ( QWidget * editor, const QModelIndex & index ) const {
	editor->metaObject()->userProperty().write(editor, index.data(Qt::EditRole));
}

 bool SpreadsheetItemDelegate::eventFilter(QObject* editor, QEvent* event) {
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
			emit commitData((QWidget*)editor);
			emit closeEditor((QWidget*)editor, QAbstractItemDelegate::EditNextItem);
			return true;
		}
	}

	return QItemDelegate::eventFilter(editor, event);
}
