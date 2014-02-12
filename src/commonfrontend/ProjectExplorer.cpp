/***************************************************************************
    File                 : ProjectExplorer.cpp
    Project              : SciDAVis/Labplot2
    Description       	 : A tree view for displaying and editing an AspectTreeModel.
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2007-2008 by Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011-2013 Alexander Semke (alexander.semke*web.de)
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
#include "backend/core/AbstractPart.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/core/PartMdiView.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QSignalMapper>
#include <QDebug>
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

ProjectExplorer::ProjectExplorer(QWidget* parent) : m_projectLoading(false) {
    Q_UNUSED(parent);
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	
	frameFilter= new QFrame(this);
	QHBoxLayout *layoutFilter= new QHBoxLayout(frameFilter);
	layoutFilter->setSpacing(0);
	layoutFilter->setContentsMargins(0, 0, 0, 0);

	lFilter = new QLabel(i18n("Search/Filter:"));
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
	bFilterOptions->setText(i18n("Options"));
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

	connect(leFilter, SIGNAL(textChanged(QString)), this, SLOT(filterTextChanged(QString)));
	connect(bFilterOptions, SIGNAL(toggled(bool)), this, SLOT(toggleFilterOptionsMenu(bool)));
}

void ProjectExplorer::createActions(){
    caseSensitiveAction = new QAction(i18n("case sensitive"), this);
    caseSensitiveAction->setCheckable(true);
    caseSensitiveAction->setChecked(false);
    connect(caseSensitiveAction, SIGNAL(triggered()), this, SLOT(toggleFilterCaseSensitivity()));

	matchCompleteWordAction = new QAction(i18n("match complete word"), this);
    matchCompleteWordAction->setCheckable(true);
    matchCompleteWordAction->setChecked(false);
    connect(matchCompleteWordAction, SIGNAL(triggered()), this, SLOT(toggleFilterMatchCompleteWord()));
	
    expandTreeAction = new QAction(i18n("expand all"), this);
    connect(expandTreeAction, SIGNAL(triggered()), m_treeView, SLOT(expandAll()));

	collapseTreeAction = new QAction(i18n("collapse all"), this);
    connect(collapseTreeAction, SIGNAL(triggered()), m_treeView, SLOT(collapseAll()));

    toggleFilterAction = new QAction(i18n("hide search/filter options"), this);
    connect(toggleFilterAction, SIGNAL(triggered()), this, SLOT(toggleFilterWidgets()));

	showAllColumnsAction = new QAction(i18n("show all"),this);
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

		menu->addSeparator()->setText(i18n("Tree options"));
		menu->addAction(expandTreeAction);
		menu->addAction(collapseTreeAction);
		menu->addSeparator();
		menu->addAction(toggleFilterAction);

		//Menu for showing/hiding the columns in the tree view
		QMenu* columnsMenu = menu->addMenu(i18n("show/hide columns"));
		columnsMenu->addAction(showAllColumnsAction);
		columnsMenu->addSeparator();
		for (int i=0; i<list_showColumnActions.size(); i++)
		columnsMenu->addAction(list_showColumnActions.at(i));

		//TODO
		//Menu for showing/hiding the top-level aspects (Worksheet, Spreadhsheet, etc) in the tree view
// 		QMenu* objectsMenu = menu->addMenu(i18n("show/hide objects"));

	}

	menu->exec(event->globalPos());
	delete menu;
}

void ProjectExplorer::setCurrentAspect(const AbstractAspect* aspect){
	qDebug()<<"ProjectExplorer::setCurrentAspect" << aspect->name();
	AspectTreeModel* tree_model = qobject_cast<AspectTreeModel*>(m_treeView->model());
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
	connect(treeModel, SIGNAL(indexSelected(QModelIndex)), this, SLOT(selectIndex(QModelIndex)));
	connect(treeModel, SIGNAL(indexDeselected(QModelIndex)), this, SLOT(deselectIndex(QModelIndex)));
	connect(treeModel, SIGNAL(hiddenAspectSelected(const AbstractAspect*)), this, SIGNAL(hiddenAspectSelected(const AbstractAspect*)));
	
	connect(m_treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
							this, SLOT(currentChanged(QModelIndex,QModelIndex)) );
	connect(m_treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), 
					this, SLOT(selectionChanged(QItemSelection,QItemSelection)) );
	
	//create action for showing/hiding the columns in the tree.
	//this is done here since the number of columns is  not available in createActions() yet.
	showColumnsSignalMapper = new QSignalMapper(this);
	for (int i=0; i<m_treeView->model()->columnCount(); i++){
	  QAction* showColumnAction =  new QAction(m_treeView->model()->headerData(i, Qt::Horizontal).toString(), this);
	  showColumnAction->setCheckable(true);
	  showColumnAction->setChecked(true);
	  list_showColumnActions.append(showColumnAction);

	  connect(showColumnAction, SIGNAL(triggered(bool)), showColumnsSignalMapper, SLOT(map()));
	  showColumnsSignalMapper->setMapping(showColumnAction, i);
	}
	connect(showColumnsSignalMapper, SIGNAL(mapped(int)), this, SLOT(toggleColumn(int)));	
}

void ProjectExplorer::setProject( const Project* project) {
	connect(project, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(aspectAdded(const AbstractAspect*)));
	connect(project, SIGNAL(loadStarted()), this, SLOT(projectLoadStarted()));
	connect(project, SIGNAL(loadFinished()), this, SLOT(projectLoadFinished()));
	connect(project, SIGNAL(requestSaveState(QXmlStreamWriter*)), this, SLOT(save(QXmlStreamWriter*)));
	connect(project, SIGNAL(requestLoadState(XmlStreamReader*)), this, SLOT(load(XmlStreamReader*)));
	m_project = project;
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
void ProjectExplorer::projectLoadStarted() {
	m_projectLoading = true;
}

void ProjectExplorer::projectLoadFinished() {
	m_projectLoading = false;
}

/*!
  expand the aspect \c aspect (the tree index corresponding to it) in the tree view
  and makes it visible and selected. Called when a new aspect is added to the project.
 */
void ProjectExplorer::aspectAdded(const AbstractAspect* aspect){
	if (m_projectLoading)
		return;

	AspectTreeModel* tree_model = qobject_cast<AspectTreeModel*>(m_treeView->model());
	const QModelIndex& index =  tree_model->modelIndexOfAspect(aspect);

	//expand and make the aspect visible
	m_treeView->setExpanded(index, true);
	m_treeView->scrollTo(index);
	m_treeView->setCurrentIndex(index);

	//select the added aspect
	m_treeView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void ProjectExplorer::currentChanged(const QModelIndex & current, const QModelIndex & previous){
	Q_UNUSED(previous);
	emit currentAspectChanged(static_cast<AbstractAspect*>(current.internalPointer()));
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
	  toggleFilterAction->setText(i18n("show search/filter options"));
	}else{
	  frameFilter->show();
	  toggleFilterAction->setText(i18n("hide search/filter options"));
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

void ProjectExplorer::selectIndex(const QModelIndex&  index){
	qDebug()<<"ProjectExplorer::selectIndex";
	m_treeView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
	m_treeView->setExpanded(index, true);
	m_treeView->scrollTo(index);
}
 
void ProjectExplorer::deselectIndex(const QModelIndex & index){
	qDebug()<<"ProjectExplorer::deselectIndex";
	m_treeView->selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
}

void ProjectExplorer::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected){
	qDebug()<<"ProjectExplorer::selectionChanged";
	QModelIndex index;
	QModelIndexList items;
	AbstractAspect* aspect = 0;

	//there are four model indices in each row 
	//-> divide by 4 to obtain the number of selected rows (=aspects)
	items = selected.indexes();
	for (int i=0; i<items.size()/4; ++i){
		index = items.at(i*4);
		aspect = static_cast<AbstractAspect *>(index.internalPointer());
		aspect->setSelected(true);
	}

	items = deselected.indexes();
	for (int i=0; i<items.size()/4; ++i){
		index = items.at(i*4);
		aspect = static_cast<AbstractAspect *>(index.internalPointer());
		aspect->setSelected(false);
	}

	items = m_treeView->selectionModel()->selectedRows();
	QList<AbstractAspect*> selectedAspects;
	foreach(const QModelIndex& index,items){
		aspect = static_cast<AbstractAspect *>(index.internalPointer());
		selectedAspects<<aspect;
	}

	emit selectedAspectsChanged(selectedAspects);
}


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
struct ViewState{
	Qt::WindowStates state;
	QRect geometry;
};

/**
 * \brief Save the current state of the tree view
 * (expanded items and the currently selected item) as XML
 */	
void ProjectExplorer::save(QXmlStreamWriter* writer) const {
	AspectTreeModel* model = qobject_cast<AspectTreeModel*>(m_treeView->model());
	QList<int> selected;
	QList<int> expanded;
	QList<int> withView;
	QList<ViewState> viewStates;

	int currentRow = 0; //row corresponding to the current index in the tree view
	QModelIndexList selectedRows = m_treeView->selectionModel()->selectedRows();

	int row = 0;
	QList<AbstractAspect*> aspects = const_cast<Project*>(m_project)->children("AbstractAspect", AbstractAspect::Recursive);
	foreach(const AbstractAspect* aspect, aspects) {
		QModelIndex index = model->modelIndexOfAspect(aspect);
		
		const AbstractPart* part = dynamic_cast<const AbstractPart*>(aspect);
		if (part && part->hasMdiSubWindow()) {
			withView.push_back(row);
			ViewState s = {part->view()->windowState(), part->view()->geometry()};
			viewStates.push_back(s);
		}
			
		if (m_treeView->isExpanded(index))
			expanded.push_back(row);

		if (selectedRows.indexOf(index) != -1)
			selected.push_back(row);

		if (index == m_treeView->currentIndex())
			currentRow = row;

		row++;
	}
	
	writer->writeStartElement("state");
	
	writer->writeStartElement("expanded");
	for (int i=0; i<expanded.size(); ++i) {
		writer->writeTextElement("row", QString::number(expanded.at(i)));
	}
	writer->writeEndElement();
	
	writer->writeStartElement("selected");
	for (int i=0; i<selected.size(); ++i) {
		writer->writeTextElement("row", QString::number(selected.at(i)));
	}
	writer->writeEndElement();
	
	writer->writeStartElement("view");
	for (int i=0; i<withView.size(); ++i) {
		writer->writeStartElement("row");
		const ViewState& s = viewStates.at(i);
		writer->writeAttribute( "state", QString::number(s.state) );
		writer->writeAttribute( "x", QString::number(s.geometry.x()) );
		writer->writeAttribute( "y", QString::number(s.geometry.y()) );
		writer->writeAttribute( "width", QString::number(s.geometry.width()) );
		writer->writeAttribute( "height", QString::number(s.geometry.height()) );
		writer->writeCharacters(QString::number(withView.at(i)));
		writer->writeEndElement();
	}
	writer->writeEndElement();

	writer->writeStartElement("current");
	writer->writeTextElement("row", QString::number(currentRow));
	writer->writeEndElement();
	
	writer->writeEndElement();
}

/**
 * \brief Load from XML
 */
bool ProjectExplorer::load(XmlStreamReader* reader) {
	AspectTreeModel* model = qobject_cast<AspectTreeModel*>(m_treeView->model());
	QList<AbstractAspect*> aspects = const_cast<Project*>(m_project)->children("AbstractAspect", AbstractAspect::Recursive);

	bool expandedItem = false;
	bool selectedItem = false;
	bool viewItem = false;
	(void)viewItem; // because of a strange g++-warning about unused viewItem
	bool currentItem = false;
	QModelIndex currentIndex;
	QString str;
	int row;
	QList<QModelIndex> selected;
	QXmlStreamAttributes attribs;
	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "state")
            break;

        if (!reader->isStartElement())
            continue;
		
		if (reader->name() == "expanded") {
			expandedItem = true;
			selectedItem = false;
			viewItem = false;
			currentItem = false;
		} else if (reader->name() == "selected") {
			expandedItem = false;
			selectedItem = true;
			viewItem = false;
			currentItem = false;
		} else 	if (reader->name() == "view") {
			expandedItem = false;
			selectedItem = false;
			viewItem = true;
			currentItem = false;
		} else 	if (reader->name() == "current") {
			expandedItem = false;
			selectedItem = false;
			viewItem = false;
			currentItem = true;
		} else if (reader->name() == "row") {
			attribs = reader->attributes();
			row = reader->readElementText().toInt();
			if (row>aspects.size())
				continue;

			const QModelIndex& index = model->modelIndexOfAspect(aspects.at(row));

			if (expandedItem) {
				m_treeView->setExpanded(index, true);
				m_treeView->scrollTo(index);
			} else if (selectedItem) {
				selected.push_back(index);
			} else if (currentItem) {
				currentIndex = index;
			} else if (viewItem) {
				AbstractPart* part = dynamic_cast<AbstractPart*>(aspects.at(row));
				if (!part)
					continue; //TODO: add error/warning message here?
				
				emit currentAspectChanged(part);

				str = attribs.value("state").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'state'"));
				else {
					part->view()->setWindowState(Qt::WindowStates(str.toInt()));
					part->mdiSubWindow()->setWindowState(Qt::WindowStates(str.toInt()));
				}
				
				if (str != "0")
					continue; //no geometry settings required for maximized/minimized windows

				QRect geometry;
				str = attribs.value("x").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'x'"));
				else
					geometry.setX(str.toInt());

				str = attribs.value("y").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'y'"));
				else
					geometry.setY(str.toInt());

				str = attribs.value("width").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'width'"));
				else
					geometry.setWidth(str.toInt());

				str = attribs.value("height").toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'height'"));
				else
					geometry.setHeight(str.toInt());
				
				part->mdiSubWindow()->setGeometry(geometry);
			}
		}
	}

	m_treeView->setCurrentIndex(currentIndex);
	m_treeView->selectionModel()->select(currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
	m_treeView->scrollTo(currentIndex);
	
	foreach(const QModelIndex& index, selected)
		m_treeView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);

	return true;
}
