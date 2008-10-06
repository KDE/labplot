/***************************************************************************
    File                 : ProjectExplorer.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Copyright            : (C) 2007-2008 by Tilman Benkert
    Email (use @ for *)  : thzs*gmx.net
    Description          : A tree view for displaying and editing an
                           AspectTreeModel.

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
#include "ProjectExplorer.h"
#include "AspectTreeModel.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QWidget>
#include <QHeaderView>

ProjectExplorer::ProjectExplorer(QWidget *parent) 
	: QTreeView(parent) 
{
	setAnimated(true);
	setAlternatingRowColors(true);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
}

void ProjectExplorer::contextMenuEvent(QContextMenuEvent *event)
{
	if(!model()) return;
	QVariant menu_value = model()->data(indexAt(event->pos()), AspectTreeModel::ContextMenuRole);
	QMenu *menu = static_cast<QMenu*>(menu_value.value<QWidget*>());
	if (!menu) return;
	menu->exec(event->globalPos());
	delete menu;
}
		
void ProjectExplorer::setCurrentAspect(const AbstractAspect * aspect)
{
	AspectTreeModel * tree_model = qobject_cast<AspectTreeModel *>(model());
	if(tree_model) setCurrentIndex(tree_model->modelIndexOfAspect(aspect));
}

void ProjectExplorer::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
	QTreeView::currentChanged(current, previous);
	emit currentAspectChanged(static_cast<AbstractAspect *>(current.internalPointer()));
}

void ProjectExplorer::setModel(QAbstractItemModel * model)
{
	QTreeView::setModel(model);
	for(int i=0; i<header()->length(); ++i)
		setColumnWidth(i, model->headerData(i, Qt::Horizontal, Qt::SizeHintRole).toSize().width());
	// beats me why QHeaderView doesn't do this... it does use the height() part, though
}
