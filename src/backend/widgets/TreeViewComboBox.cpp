/***************************************************************************
    File                 : TreeViewComboBox.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses)
    Description          : Provides a QTreeView in a QComboBox

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

#include "TreeViewComboBox.h"
#include "../core/Folder.h"
#include <KDebug>

TreeViewComboBox::TreeViewComboBox(QWidget* parent):QComboBox(parent){
	m_treeView.header()->hide();
 	setView(&m_treeView);
	m_topLevelClasses << "Folder" << "Table" << "Worksheet";
}


TreeViewComboBox::~TreeViewComboBox()
{
}

void TreeViewComboBox::setModel(QAbstractItemModel *model){
 	QComboBox::setModel(model);
	m_treeView.hideColumn(1);
	m_treeView.hideColumn(2);
	m_treeView.hideColumn(3);

	//TODO why this doesn't work?!?
	//m_treeView.expandAll();
	// my guess would be that it requires the view to be visible - Tilman
}

void TreeViewComboBox::showPopup()
{
	m_treeView.expandAll();
	QModelIndex root = model()->index(0,0);
	showTopLevelOnly(root);
	QComboBox::showPopup();
}

void TreeViewComboBox::showTopLevelOnly(const QModelIndex & index)
{
	int rows = index.model()->rowCount(index);
	for (int i=0; i<rows; i++) {
		QModelIndex currentChild = index.child(i, 0);
		showTopLevelOnly(currentChild);
		AbstractAspect *aspect =  static_cast<AbstractAspect*>(currentChild.internalPointer());
		bool isTopLevel = false;
		foreach(const char * classString, m_topLevelClasses)
			if (aspect->inherits(classString))
				isTopLevel = true;
		m_treeView.setRowHidden(i, index, !isTopLevel);
	}
}

/*
void TreeViewComboBox::keyPressEvent(QKeyEvent* e)
{
    QComboBox::keyPressEvent(e);
}

void TreeViewComboBox::keyReleaseEvent(QKeyEvent* e)
{
		kDebug()<<"";
    QComboBox::keyReleaseEvent(e);
}

void TreeViewComboBox::mousePressEvent(QMouseEvent* e)
{
    QComboBox::mousePressEvent(e);
}

void TreeViewComboBox::mouseReleaseEvent(QMouseEvent* e)
{

    QComboBox::mouseReleaseEvent(e);
}

*/
