/***************************************************************************
    File                 : RecordShortcutDelegate.cpp
    Project              : SciDAVis
    Description          : Item delegate that records a key combination from the keyboard
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

#include "RecordShortcutDelegate.h"
#include <QModelIndex>

RecordShortcutDelegateEditor::RecordShortcutDelegateEditor(QWidget * parent)
	: QLineEdit(parent) 
{
}

void RecordShortcutDelegateEditor::keyPressEvent(QKeyEvent * event)
{
	if (event->isAutoRepeat())
		return;
	int key;
	switch (event->key())
	{
		case Qt::Key_Shift:
		case Qt::Key_Control:
		case Qt::Key_Alt:
		case Qt::Key_AltGr:
		case Qt::Key_Meta:
			key = 0x0;
			break;
		default:
			key = event->key();
	}
	if (event->modifiers() & Qt::ShiftModifier)
		key |= Qt::SHIFT;
	if (event->modifiers() & Qt::ControlModifier)
		key |= Qt::CTRL;
	if (event->modifiers() & Qt::AltModifier)
		key |= Qt::ALT;
	if (event->modifiers() & Qt::MetaModifier)
		key |= Qt::META;
	setText(QKeySequence(key).toString());
}

void RecordShortcutDelegateEditor::keyReleaseEvent(QKeyEvent * event)
{
	if (event->isAutoRepeat())
		return;
	switch (event->key())
	{
		case Qt::Key_Shift:
		case Qt::Key_Control:
		case Qt::Key_Alt:
		case Qt::Key_AltGr:
		case Qt::Key_Meta:
			return;
	}
	int key = event->key();
	if (event->modifiers() & Qt::ShiftModifier)
		key |= Qt::SHIFT;
	if (event->modifiers() & Qt::ControlModifier)
		key |= Qt::CTRL;
	if (event->modifiers() & Qt::AltModifier)
		key |= Qt::ALT;
	if (event->modifiers() & Qt::MetaModifier)
		key |= Qt::META;
	close();
}

QWidget * RecordShortcutDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option,
    const QModelIndex & index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);
    return new RecordShortcutDelegateEditor(parent);
}

void RecordShortcutDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    static_cast<RecordShortcutDelegateEditor *>(editor)->setText(index.model()->data(index, Qt::DisplayRole).toString());
}

void RecordShortcutDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex &index) const
{
    model->setData(index, static_cast<RecordShortcutDelegateEditor *>(editor)->text(), Qt::EditRole);
}

void RecordShortcutDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, 
	const QModelIndex & index) const
{
	Q_UNUSED(index);
    editor->setGeometry(option.rect);
}



