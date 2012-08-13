/***************************************************************************
    File                 : TreeViewComboBox.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2012 by Alexander Semke (alexander.semke*web.de)
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
#include "core/AbstractAspect.h"
#include "core/AspectTreeModel.h"

/*!
    \class TreeViewComboBox
    \brief Provides a QTreeView in a QComboBox.

    \ingroup backend/widgets
*/


TreeViewComboBox::TreeViewComboBox(QWidget* parent):QComboBox(parent){
	m_treeView.header()->hide();
	m_treeView.setSelectionMode(QAbstractItemView::SingleSelection);
	m_treeView.setUniformRowHeights(true);

	m_treeView.hide();
	m_treeView.setParent(parent, Qt::Popup);
	m_treeView.installEventFilter(this);
	addItem("");
	setCurrentIndex(0);
	
	connect(&m_treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(treeViewIndexActivated(const QModelIndex&) ) );
}

void TreeViewComboBox::setTopLevelClasses(QList<const char *> list){
  m_topLevelClasses=list;
}

TreeViewComboBox::~TreeViewComboBox(){
}

/*!
	Sets the \a model for the view to present.
*/
void TreeViewComboBox::setModel(QAbstractItemModel *model){
	m_treeView.setModel(model);
	
	//show only the first column in the combo box
	for (int i=1; i<model->columnCount(); i++){
	  m_treeView.hideColumn(i);
	}

	//Expand the complete tree in order to see everything in the first popup.
	m_treeView.expandAll();
}

/*!
	Sets the current item to be the item at \a index and selects it.
	\sa currentIndex()
*/
void TreeViewComboBox::setCurrentModelIndex(const QModelIndex& index){
// 	view()->setCurrentIndex(index);
	m_treeView.setCurrentIndex(index);
	QComboBox::setItemText(0, index.data().toString());
}

/*!
	Returns the model index of the current item.

	\sa setCurrentModelIndex()
*/
QModelIndex TreeViewComboBox::currentModelIndex() const{
	return m_treeView.currentIndex();
}

/*!
	Displays the tree view of items in the combobox.
	Triggers showTopLevelOnly() to show toplevel items only.
*/
void TreeViewComboBox::showPopup(){
	if (!m_treeView.model()->hasChildren())
		return;

	QModelIndex root = m_treeView.model()->index(0,0);
	showTopLevelOnly(root);

	m_treeView.resize(this->width(), 150);
	m_treeView.move(mapToGlobal( this->rect().bottomLeft() ));
	m_treeView.setFocus();
	m_treeView.show();
}


/*!
	Hides the non-toplevel items of the model used in the tree view.
*/
void TreeViewComboBox::showTopLevelOnly(const QModelIndex & index){
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

/*!
	catches the MouseButtonPress-event and hides the tree view on mouse clicking.
*/
bool TreeViewComboBox::eventFilter(QObject *object, QEvent *event){
	if (object==&m_treeView && event->type()==QEvent::MouseButtonPress){
		m_treeView.hide();
		this->setFocus();
		return true;
	}
	return false;
}

//SLOTs

void TreeViewComboBox::treeViewIndexActivated( const QModelIndex & index){
	QComboBox::setItemText(0, index.data().toString());
	m_treeView.hide();
	
	//workaround hack.
	//TODO: make the non-column objects nonselectable (disabled).
	//FIX: an model item without the flags IsSelectable and IsEnabled can still be selected.
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	foreach(const char * classString, m_topLevelClasses){
		if (aspect->inherits("Column"))
			emit currentModelIndexChanged(index);
	}
}
