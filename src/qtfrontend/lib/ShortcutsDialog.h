/***************************************************************************
    File                 : ShortcutsDialog.h
    Project              : SciDAVis
    Description          : Customize keyboard shortcuts dialog
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

#ifndef SHORTCUTSDIALOG_H
#define SHORTCUTSDIALOG_H

#include <QDialog>
class RecordShortcutDelegate;
class ActionManager;
class QTreeView;
class QModelIndex;

// TODO: implement check for conflicts due to multiply assigned shortcuts

//! Customize keyboard shortcuts dialog
/**
 * This dialog is used to let the user assign a primary and a secondary
 * keyboard shortcut to the actions managed by the given action managers.
 * The dialog mainly contains a tree view with three columns:
 * action text, (primarty) shortcut, alternative shortcut. The actions are 
 * displayed as children of their managers (represented by their titles).
 * See ActionManager for details on the organization of the actions.
 */
class ShortcutsDialog : public QDialog
{
	Q_OBJECT

	public:
		ShortcutsDialog(QList<ActionManager *> action_managers, QWidget * parent = 0);
		~ShortcutsDialog();

	private:
		RecordShortcutDelegate * m_delegate;
		QTreeView * m_tree_view;

	private slots:
		void resizeColumns(const QModelIndex & index);
		void resizeColumns(const QModelIndex & top_left, const QModelIndex & bottom_right);
};


#endif // SHORTCUTSDIALOG_H
