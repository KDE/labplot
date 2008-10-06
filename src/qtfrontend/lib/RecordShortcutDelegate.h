/***************************************************************************
    File                 : RecordShortcutDelegate.h
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

#ifndef RECORDSHORTCUTDELEGATE_H
#define RECORDSHORTCUTDELEGATE_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QItemDelegate>

//! Editor widget that records a key combination as a QKeySequence
/**
 * This editor displays the key combination (modifiers + key) while
 * it is pressed and returns once the non-modifier key is released. 
 * The key combination can then be read out by text().
 */
class RecordShortcutDelegateEditor : public QLineEdit
{
	Q_OBJECT

	public:
		RecordShortcutDelegateEditor(QWidget * parent = 0);

	protected:
		virtual void keyPressEvent(QKeyEvent * event);
		virtual void keyReleaseEvent(QKeyEvent * event);
};

//! Item delegate that records a key combination from the keyboard
/**
 * This item delegate ist meant to be used in a dialog that configures
 * keyboard shortcuts. It reads key combinations directly (i.e. you
 * press [ALT]+[CTRL]+A instead of typing 'A L T + C T R L + A') and 
 * sets a string to the model that can be used to create a key sequence 
 * using QKeySequence::QKeySequence(const QString & key).
 */
class RecordShortcutDelegate : public QItemDelegate
{
    Q_OBJECT

	public:
		RecordShortcutDelegate(QObject *parent = 0) : QItemDelegate(parent) {};

		QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
		void setEditorData(QWidget * editor, const QModelIndex & index) const;
		void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;
		void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // RECORDSHORTCUTDELEGATE_H
