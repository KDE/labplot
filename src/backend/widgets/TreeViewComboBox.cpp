/***************************************************************************
    File                 : TreeViewComboBox.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
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
	treeView.header()->hide();
 	setView(&treeView);
}


TreeViewComboBox::~TreeViewComboBox()
{
}

void TreeViewComboBox::setModel(QAbstractItemModel *model){
 	QComboBox::setModel(model);
	treeView.hideColumn(1);
	treeView.hideColumn(2);
	treeView.hideColumn(3);
// 	int rows=model->rowCount(model->parent());
// // // 	kDebug()<<"rows"<<rows<<endl;
//  	for (int i=0; i<rows; i++)
// 		treeView.setExpanded(model()->index(i, 0, model->parent()), true);

	//TODO why this doesn't work?!?
	treeView.expandAll();
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