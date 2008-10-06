/***************************************************************************
    File                 : ShortcutsDialog.cpp
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

#include "ShortcutsDialog.h"
#include "RecordShortcutDelegate.h"
#include "ShortcutsDialogModel.h"
#include "ActionManager.h"
#include <QTreeView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>


ShortcutsDialog::ShortcutsDialog(QList<ActionManager *> action_managers, QWidget * parent) 
	: QDialog(parent)
{
	QVBoxLayout * main_layout = new QVBoxLayout(this);
	m_tree_view = new QTreeView();
	m_tree_view->setModel(new ShortcutsDialogModel(action_managers, m_tree_view));
	m_tree_view->setSelectionBehavior(QAbstractItemView::SelectItems);
	m_tree_view->setAlternatingRowColors(true);
	m_delegate = new RecordShortcutDelegate();
	m_tree_view->setItemDelegate(m_delegate);
	m_tree_view->resizeColumnToContents(0);
	m_tree_view->resizeColumnToContents(1);
	m_tree_view->resizeColumnToContents(2);
	m_tree_view->header()->setMovable(false);
	main_layout->addWidget(m_tree_view);
	QPushButton * close_button = new QPushButton(tr("&Close"));
	main_layout->addWidget(close_button);
	connect(close_button, SIGNAL(clicked()), this, SLOT(close()));
	connect(m_tree_view, SIGNAL(expanded(const QModelIndex &)), this, SLOT(resizeColumns(const QModelIndex &)));
	connect(m_tree_view, SIGNAL(collapsed(const QModelIndex &)), this, SLOT(resizeColumns(const QModelIndex &)));
	connect(m_tree_view->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), 
		this, SLOT(resizeColumns(const QModelIndex &, const QModelIndex &)));
}

void ShortcutsDialog::resizeColumns(const QModelIndex & index)
{
	if(!index.isValid())
		return;
	m_tree_view->resizeColumnToContents(0);
	m_tree_view->resizeColumnToContents(1);
	m_tree_view->resizeColumnToContents(2);
}

void ShortcutsDialog::resizeColumns(const QModelIndex & top_left, const QModelIndex & bottom_right)
{
	if(!top_left.isValid() || !bottom_right.isValid())
		return;
	for (int i=top_left.column(); i<=bottom_right.column(); i++)
		m_tree_view->resizeColumnToContents(i);
}

ShortcutsDialog::~ShortcutsDialog()
{
	delete m_delegate;
}

