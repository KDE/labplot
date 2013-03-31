/***************************************************************************
    File                		: ProjectExplorer.cpp
    Project              	: SciDAVis/Labplot2
    Description       	: A tree view for displaying and editing an AspectTreeModel.
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2007-2008 by Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011-2012 Alexander Semke (alexander.semke*web.de)
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
#include "ProjectExplorer.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/Project.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QSignalMapper>

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QLineEdit>
#else
#include <KLineEdit>
#include <KLocale>
#include <KMenu>
#endif

/*!
  \class ProjectExplorer
  \brief A tree view for displaying and editing an AspectTreeModel.

  In addition to the functionality of QTreeView, ProjectExplorer allows 
  the usage of the context menus provided by AspectTreeModel
  and propagates the item selection in the view to the model.
  Furthermore, features for searching and filtering in the model are provided.
  
  \ingroup commonfrontend
*/

ProjectExplorer::ProjectExplorer(QWidget *parent){
    Q_UNUSED(parent);
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	
	frameFilter= new QFrame(this);
	QHBoxLayout *layoutFilter= new QHBoxLayout(frameFilter);
	layoutFilter->setSpacing(0);
	layoutFilter->setContentsMargins(0, 0, 0, 0);

	lFilter = new QLabel(tr("Search/Filter:"));
	layoutFilter->addWidget(lFilter);
	
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	leFilter= new QLineEdit(frameFilter);
#else
	leFilter= new KLineEdit(frameFilter);
	qobject_cast<KLineEdit*>(leFilter)->setClearButtonShown(true);
	qobject_cast<KLineEdit*>(leFilter)->setClickMessage(i18n("Search/Filter text"));
#endif
	layoutFilter->addWidget(leFilter);

	bFilterOptions = new QPushButton(frameFilter);
	bFilterOptions->setText(tr("Options"));
	bFilterOptions->setEnabled(true);
	bFilterOptions->setCheckable(true);
	layoutFilter->addWidget(bFilterOptions);

	layout->addWidget(frameFilter);
	 
	m_treeView = new QTreeView(this);
	m_treeView->setAnimated(true);
	m_treeView->setAlternatingRowColors(true);
	m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_treeView->setUniformRowHeights(true);
	m_treeView->header()->setStretchLastSection(true);
	m_treeView->header()->installEventFilter(this);
	
	layout->addWidget(m_treeView);
	
	
	m_columnToHide=0;
	this->createActions();
	
	connect(leFilter, SIGNAL(textChanged(const QString&)), this, SLOT(filterTextChanged(const QString&)));
	connect(bFilterOptions, SIGNAL(toggled(bool)), this, SLOT(toggleFilterOptionsMenu(bool)));
}

void ProjectExplorer::createActions(){
    caseSensitiveAction = new QAction(tr("case sensitive"), this);
    caseSensitiveAction->setCheckable(true);
    caseSensitiveAction->setChecked(false);
    connect(caseSensitiveAction, SIGNAL(triggered()), this, SLOT(toggleFilterCaseSensitivity()));

	matchCompleteWordAction = new QAction(tr("match complete word"), this);
    matchCompleteWordAction->setCheckable(true);
    matchCompleteWordAction->setChecked(false);
    connect(matchCompleteWordAction, SIGNAL(triggered()), this, SLOT(toggleFilterMatchCompleteWord()));
	
    expandTreeAction = new QAction(tr("expand all"), this);
    connect(expandTreeAction, SIGNAL(triggered()), m_treeView, SLOT(expandAll()));

	collapseTreeAction = new QAction(tr("collapse all"), this);
    connect(collapseTreeAction, SIGNAL(triggered()), m_treeView, SLOT(collapseAll()));

    toggleFilterAction = new QAction(tr("hide search/filter options"), this);
    connect(toggleFilterAction, SIGNAL(triggered()), this, SLOT(toggleFilterWidgets()));

	showAllColumnsAction = new QAction(tr("show all"),this);
	showAllColumnsAction->setCheckable(true);
    showAllColumnsAction->setChecked(true);
	showAllColumnsAction->setEnabled(false);
	connect(showAllColumnsAction, SIGNAL(triggered()), this, SLOT(showAllColumns()));
}

/*!
  shows the context menu in the tree. In addition to the context menu of the currently selected aspect,
  treeview specific options are added.
*/
void ProjectExplorer::contextMenuEvent(QContextMenuEvent *event){
	if(!m_treeView->model())
	  return;

	QModelIndex index = m_treeView->indexAt(m_treeView->viewport()->mapFrom(this, event->pos()));
	QVariant menu_value = m_treeView->model()->data(index, AspectTreeModel::ContextMenuRole);
	QMenu *menu = static_cast<QMenu*>(menu_value.value<QWidget*>());

	if (!menu){
		menu = new QMenu();

		menu->addSeparator()->setText(tr("Tree options"));
		menu->addAction(expandTreeAction);
		menu->addAction(collapseTreeAction);
		menu->addSeparator();
		menu->addAction(toggleFilterAction);

		//Menu for showing/hiding the columns in the tree view
		QMenu* columnsMenu = menu->addMenu(tr("show/hide columns"));
		columnsMenu->addAction(showAllColumnsAction);
		columnsMenu->addSeparator();
		for (int i=0; i<list_showColumnActions.size(); i++)
		columnsMenu->addAction(list_showColumnActions.at(i));

		//TODO
		//Menu for showing/hiding the top-level aspects (Worksheet, Spreadhsheet, etc) in the tree view
// 		QMenu* objectsMenu = menu->addMenu(tr("show/hide objects"));

	}

	menu->exec(event->globalPos());
	delete menu;
}

void ProjectExplorer::setCurrentAspect(const AbstractAspect * aspect){
	AspectTreeModel * tree_model = qobject_cast<AspectTreeModel *>(m_treeView->model());
	if(tree_model)
	  m_treeView->setCurrentIndex(tree_model->modelIndexOfAspect(aspect));
}

/*!
  Sets the \c model for the tree view to present.
*/
void ProjectExplorer::setModel(QAbstractItemModel * model){
	m_treeView->setModel(model);
	m_treeView->header()->resizeSections(QHeaderView::ResizeToContents);
	
	AspectTreeModel* treeModel = qobject_cast<AspectTreeModel*>(model);

	connect(treeModel, SIGNAL(renameRequested(QModelIndex)), m_treeView, SLOT(edit(QModelIndex)));
	connect(treeModel, SIGNAL(indexSelected(const QModelIndex&)), this, SLOT(selectIndex(const QModelIndex&) ));
	connect(treeModel, SIGNAL(indexDeselected(const QModelIndex&)), this, SLOT(deselectIndex(const QModelIndex&) ));
	connect(treeModel, SIGNAL(hiddenAspectSelected(const AbstractAspect*)), this, SIGNAL(hiddenAspectSelected(const AbstractAspect*)));
	
	connect(m_treeView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
							this, SLOT(currentChanged(const QModelIndex&, const QModelIndex&)) );
	connect(m_treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
					this, SLOT(selectionChanged(const QItemSelection&, const QItemSelection&) ) );
	
	//create action for showing/hiding the columns in the tree.
	//this is done here since the number of columns is  not available in createActions() yet.
	showColumnsSignalMapper = new QSignalMapper(this);
	QAction* showColumnAction;
	for (int i=0; i<m_treeView->model()->columnCount(); i++){
	  showColumnAction =  new QAction(m_treeView->model()->headerData(i, Qt::Horizontal).toString(), this);
	  showColumnAction->setCheckable(true);
	  showColumnAction->setChecked(true);
	  list_showColumnActions.append(showColumnAction);

	  connect(showColumnAction, SIGNAL(triggered(bool)), showColumnsSignalMapper, SLOT(map()));
	  showColumnsSignalMapper->setMapping(showColumnAction, i);
	}
	 connect(showColumnsSignalMapper, SIGNAL(mapped(int)), this, SLOT(toggleColumn(int)));	
}

void ProjectExplorer::setProject ( const Project* project){
	connect(project, SIGNAL(aspectAdded(const AbstractAspect *)), this, SLOT(expandAspect(const AbstractAspect *)));
}

QModelIndex ProjectExplorer::currentIndex() const{
  return m_treeView->currentIndex();
}

/*!
  Returns the model that this view is presenting.
  \sa setModel()
*/
QAbstractItemModel* ProjectExplorer::model() const{
	return m_treeView->model();
}

/*!
	handles the contextmenu-event of the horizontal header in the tree view.
	Provides a menu for selective showing and hiding of columns.
	//TODO add i18n (and mayby some icons) for KDE.
*/
bool ProjectExplorer::eventFilter(QObject* obj, QEvent* event){
	QHeaderView* h = m_treeView->header();
	if (obj!=h)
	  return QObject::eventFilter(obj, event);
	
	if (event->type() != QEvent::ContextMenu)
		return QObject::eventFilter(obj, event);
	
	QContextMenuEvent* e = static_cast<QContextMenuEvent*>(event);
	
	//Menu for showing/hiding the columns in the tree view
	QMenu* columnsMenu;
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	columnsMenu = new QMenu(h);
	//TODO how to add a caption/title for the QMenu, when used as a context menu?
#else
	columnsMenu = new KMenu(h);
	(qobject_cast<KMenu*>(columnsMenu))->addTitle(i18n("Columns"));
#endif	
	
	columnsMenu->addAction(showAllColumnsAction);
	columnsMenu->addSeparator();
	for (int i=0; i<list_showColumnActions.size(); i++)
	   columnsMenu->addAction(list_showColumnActions.at(i));
	
	columnsMenu->exec(e->globalPos());
	delete columnsMenu;	

	return true;
}


//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################
/*!
  expand the aspect \c aspect (the tree index corresponding to it) in the tree view and makes it visible.
  Called when a new aspect is added to the project.
 */
void ProjectExplorer::expandAspect(const AbstractAspect* aspect){
	AspectTreeModel * tree_model = qobject_cast<AspectTreeModel *>(m_treeView->model());
	const QModelIndex& index =  tree_model->modelIndexOfAspect(aspect);

	m_treeView->setExpanded(index, true);
	m_treeView->scrollTo(index);
	m_treeView->setCurrentIndex(index);
	
	m_treeView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void ProjectExplorer::currentChanged(const QModelIndex & current, const QModelIndex & previous){
	Q_UNUSED(previous);
	emit currentAspectChanged(static_cast<AbstractAspect *>(current.internalPointer()));
}

void ProjectExplorer::toggleColumn(int index){
	//determine the total number of checked column actions
	int checked = 0;  
	foreach(QAction* action, list_showColumnActions){
		if (action->isChecked())
			checked++;
	}
	
	if (list_showColumnActions.at(index)->isChecked()){
		m_treeView->showColumn(index);
		m_treeView->header()->resizeSection(0,0 );
		m_treeView->header()->resizeSections(QHeaderView::ResizeToContents);
		
		foreach(QAction* action, list_showColumnActions)
			action->setEnabled(true);
		
		//deactivate the "show all column"-action, if all actions are checked
		if ( checked == list_showColumnActions.size() ){
			showAllColumnsAction->setEnabled(false);
			showAllColumnsAction->setChecked(true);
		}
	}else{
		m_treeView->hideColumn(index);
		showAllColumnsAction->setEnabled(true);
		showAllColumnsAction->setChecked(false);
		
		//if there is only one checked column-action, deactivated it.
		//It should't be possible to hide all columns
		if ( checked == 1 ){
			int i=0;
			while( !list_showColumnActions.at(i)->isChecked() )
				i++;
			
			list_showColumnActions.at(i)->setEnabled(false);
		}
	}
}

void ProjectExplorer::showAllColumns(){
	for (int i=0; i<m_treeView->model()->columnCount(); i++){
		m_treeView->showColumn(i);
		m_treeView->header()->resizeSection(0,0 );
		m_treeView->header()->resizeSections(QHeaderView::ResizeToContents);
	}
	showAllColumnsAction->setEnabled(false);
	
	foreach(QAction* action, list_showColumnActions){
		action->setEnabled(true);
		action->setChecked(true);
	}
}

/*!
  shows/hides the frame with the search/filter widgets
*/
void ProjectExplorer::toggleFilterWidgets(){
 	if (frameFilter->isVisible()){
	  frameFilter->hide();
	  toggleFilterAction->setText(tr("show search/filter options"));
	}else{
	  frameFilter->show();
	  toggleFilterAction->setText(tr("hide search/filter options"));
	}
}

/*!
  toggles the menu for the filter/search options
*/
void ProjectExplorer::toggleFilterOptionsMenu(bool checked){
    if (checked){
        QMenu menu;
        menu.addAction(caseSensitiveAction);
		menu.addAction(matchCompleteWordAction);
        connect(&menu, SIGNAL(aboutToHide()), bFilterOptions, SLOT(toggle()));
        menu.exec(bFilterOptions->mapToGlobal(QPoint(0,bFilterOptions->height())));
    }
}

/*!
  called when the filter/search text was changend.
*/
void ProjectExplorer::filterTextChanged(const QString& text){
    AspectTreeModel * model = qobject_cast<AspectTreeModel *>(m_treeView->model());
	if(!model)
		return;
	
	model->setFilterString(text);
	m_treeView->update();
}

void ProjectExplorer::toggleFilterCaseSensitivity(){
	AspectTreeModel * model = qobject_cast<AspectTreeModel *>(m_treeView->model());
	if(!model)
		return;
	
	if (caseSensitiveAction->isChecked())
		model->setFilterCaseSensitivity(Qt::CaseSensitive);
	else
		model->setFilterCaseSensitivity(Qt::CaseInsensitive);
	
	model->setFilterString(leFilter->text());
	m_treeView->update();
}


void ProjectExplorer::toggleFilterMatchCompleteWord(){
	AspectTreeModel * model = qobject_cast<AspectTreeModel *>(m_treeView->model());
	if(!model)
		return;
	
	model->setFilterMatchCompleteWord(matchCompleteWordAction->isChecked());
	
	model->setFilterString(leFilter->text());
	m_treeView->update();
}

void ProjectExplorer::selectIndex(const QModelIndex &  index){
  m_treeView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}
 
void ProjectExplorer::deselectIndex(const QModelIndex & index){
  m_treeView->selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
}

//TODO optimize
 void ProjectExplorer::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected){
  QModelIndex index;
  QModelIndexList items;
  AbstractAspect* aspect=0;

  //there are four model indices in each row ->divide by 4 to obtain the number of selected aspects.
  //TODO find out a solution which is not explicitely dependent on the current number of columns.
  items = selected.indexes();
  for (int i=0; i<items.size()/4; ++i){
	index=items.at(i*4);
	aspect = static_cast<AbstractAspect *>(index.internalPointer());
	aspect->setSelected(true);
  }
  
  items = deselected.indexes();
  for (int i=0; i<items.size()/4; ++i){
	index=items.at(i*4);
	aspect = static_cast<AbstractAspect *>(index.internalPointer());
	aspect->setSelected(false);
  }
  
  items = m_treeView->selectionModel()->selectedRows();
  QList<AbstractAspect*> selectedAspects;
  foreach(index,items){
	aspect = static_cast<AbstractAspect *>(index.internalPointer());
	selectedAspects<<aspect;
  }
  
  emit selectedAspectsChanged(selectedAspects);
}
