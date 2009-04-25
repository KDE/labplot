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

/*!
    \class TreeViewComboBox
    \brief Provides a QTreeView in a QComboBox

    \ingroup backend/widgets
 */

TreeViewComboBox::TreeViewComboBox(QWidget* parent):QComboBox(parent)
{
	m_treeView.header()->hide();
	m_treeView.setSelectionMode(QAbstractItemView::SingleSelection);
 	setView(&m_treeView);
	m_topLevelClasses << "Folder" << "Spreadsheet" << "Worksheet";
	m_firstPopup=true;
}


TreeViewComboBox::~TreeViewComboBox()
{
}

/*!
	Sets the \a model for the view to present.
*/
void TreeViewComboBox::setModel(QAbstractItemModel *model){
 	QComboBox::setModel(model);
	m_treeView.hideColumn(1);
	m_treeView.hideColumn(2);
	m_treeView.hideColumn(3);
}

/*!
	Sets the current item to be the item at \a index and selects it.

	\sa currentIndex()
*/
void TreeViewComboBox::setCurrentIndex(const QModelIndex& index){
	//TODO selection of the current index doesn't work if treeview is used.
	m_treeView.setCurrentIndex(index);
// 	view()->selectionModel()->select(index, QItemSelectionModel::Select);
	m_treeView.setExpanded(index, true);
}


/*!
	Returns the model index of the current item.

	\sa setCurrentIndex()
*/
QModelIndex TreeViewComboBox::currentIndex() const{
	return m_treeView.currentIndex();
}

/*!
	Displays the tree view of items in the combobox.
	Triggers showTopLevelOnly() to show toplevel items only.
	Expands the complete tree on the first call of this function.
*/
void TreeViewComboBox::showPopup()
{
	if (m_firstPopup){
		m_treeView.expandAll();
		m_firstPopup=false;
	}

	QModelIndex root = model()->index(0,0);
	showTopLevelOnly(root);
	QComboBox::showPopup();
}

/*!
	Hides the non-toplevel items of the model used in the tree view.
*/
void TreeViewComboBox::showTopLevelOnly(const QModelIndex & index)
{
	int rows = index.model()->rowCount(index);
	AbstractAspect *aspect;
	QModelIndex currentChild;
	bool isTopLevel;
	for (int i=0; i<rows; i++) {
		currentChild = index.child(i, 0);
		showTopLevelOnly(currentChild);
		aspect =  static_cast<AbstractAspect*>(currentChild.internalPointer());
		isTopLevel = false;
		foreach(const char * classString, m_topLevelClasses)
			if (aspect->inherits(classString))
				isTopLevel = true;

		m_treeView.setRowHidden(i, index, !isTopLevel);
	}
}
