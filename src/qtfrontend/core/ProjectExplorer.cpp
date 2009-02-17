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
#include "core/AspectTreeModel.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QWidget>
#include <QHeaderView>
#include <QMessageBox>

ProjectExplorer::ProjectExplorer(QWidget *parent)
	: QTreeView(parent)
{
	setAnimated(true);
	setAlternatingRowColors(true);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	m_contextMenuColumn=0;

	header()->installEventFilter(this);
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


/*!
	handles the contextmenu-event of the header.
	//TODO add i18n (and mayby some icons) for KDE.
*/
bool ProjectExplorer::eventFilter(QObject* obj, QEvent* event)
{
	QHeaderView* h=header();
	if (obj==h){
		if (event->type() == QEvent::ContextMenu){
			QContextMenuEvent* e = static_cast<QContextMenuEvent*>(event);
			m_contextMenuColumn=h->logicalIndexAt(e->pos());

			QMenu *menu = new QMenu(h);
			QAction* action=0;

			//allow column hiding only if there are more than one column visible
			if ( model()->columnCount()-h->hiddenSectionCount()>1 ){
				action=menu->addAction( tr("Hide %1").arg(model()->headerData(m_contextMenuColumn, Qt::Horizontal).toString() ) );
				connect( action, SIGNAL(triggered()), SLOT(hideColumnSlot()) );
			}

			//show-menu is only visible, if there are hidden columns
			//show-submenu lists only hidden columns
			if (h->hiddenSectionCount()>0){
				QMenu* showMenu = menu->addMenu( tr("Show") );
				QActionGroup* showActions = new QActionGroup(this);
				for (int i=0; i<model()->columnCount(); i++){
					if ( h->isSectionHidden(i) ){
						action = new QAction(model()->headerData(i, Qt::Horizontal).toString(), menu);
						action->setShortcut(0);
						showMenu->addAction(action);
						showActions->addAction( action );
					}
				}
				connect(showActions, SIGNAL(triggered(QAction*)), SLOT(showColumnSlot(QAction*)));
			}

 			menu->exec(e->globalPos());
			return true;
		}
	}

	return QObject::eventFilter(obj, event);
}

void ProjectExplorer::hideColumnSlot()
{
	hideColumn(m_contextMenuColumn);
}

void ProjectExplorer::showColumnSlot(QAction* action)
{
	QString name=action->text().remove("&");
	for (int i=0; i<model()->columnCount(); i++){
		if (model()->headerData(i, Qt::Horizontal).toString()==name){
			showColumn(i);

			// the just shown column gets extendend a lot :-(
			//TODO better way to adjust the width?
			//TODO remove scrollbar wenn not needed (e.g. if only one column is visible)
			for(int k=0; k<header()->length(); k++)
				setColumnWidth(k, model()->headerData(k, Qt::Horizontal, Qt::SizeHintRole).toSize().width());

			return;
		}
	}
}
