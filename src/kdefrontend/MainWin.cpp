/***************************************************************************
    File                 : MainWin.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2012 Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2008 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    Copyright            : (C) 2007-2008 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2007-2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses)
    Description          : main window
    

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

#include "MainWin.h"

#include "backend/core/Project.h"
#include "backend/core/Folder.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/datasources/FileDataSource.h"

#include "commonfrontend/ProjectExplorer.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "commonfrontend/worksheet/WorksheetView.h"

#include "kdefrontend/worksheet/ExportWorksheetDialog.h"
#include "kdefrontend/datasources/ImportFileDialog.h"
#include "kdefrontend/HistoryDialog.h"
#include "kdefrontend/SettingsDialog.h"
#include "kdefrontend/GuiObserver.h"

#include <QDockWidget>
#include <QStackedWidget>

#include <KApplication>
#include <KActionCollection>
#include <KStandardAction>
#include <kxmlguifactory.h>
#include <KMessageBox>
#include <KStatusBar>
#include <KLocale>
#include <KDebug>
#include <KFilterDev>

 /*!
	\class MainWin
	\brief Main application window.

	\ingroup kdefrontend
 */

MainWin::MainWin(QWidget *parent, const QString& filename)
	: KXmlGuiWindow(parent),
	m_currentSubWindow(0),
	m_project(0),
	m_aspectTreeModel(0),
	m_projectExplorer(0),
	m_projectExplorerDock(0),
	m_propertiesDock(0),
	m_currentAspect(0),
	m_currentFolder(0),
	m_fileName(filename),
	m_suppressCurrentSubWindowChangedEvent(false),
	m_visibilityMenu(0),
	m_newMenu(0),
	axisDock(0),
	cartesianPlotDock(0),
	columnDock(0),
	spreadsheetDock(0),
	projectDock(0),
	lineSymbolCurveDock(0),
	worksheetDock(0),
	textLabelDock(0){

	m_mdiArea = new QMdiArea;
	setCentralWidget(m_mdiArea);
	statusBar()->showMessage(i18n("Welcome to LabPlot") + " " + LVERSION);
	initActions();
	initMenus();
	setupGUI();
	setAttribute( Qt::WA_DeleteOnClose );

	connect(m_mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), 
			this, SLOT(handleCurrentSubWindowChanged(QMdiSubWindow*)));
	
	QTimer::singleShot( 0, this, SLOT(initGUI()) );
}

MainWin::~MainWin() {
	kDebug()<<"write settings"<<endl;
	//write settings
	//TODO
	m_recentProjectsAction->saveEntries( KGlobal::config()->group("Recent Files") );
	//etc...
	 KGlobal::config()->sync();

	 if (m_project!=0){
		m_mdiArea->closeAllSubWindows();
		disconnect(m_project, 0, this, 0);
		delete m_aspectTreeModel;
		delete m_project;
	}
}

void MainWin::initGUI(){
  //TODO make the tabbed view optional and/or accessible via the menu.
  //The tabbed view collides with the visibility policy for the subwindows.
  //Hide the menus for the visibility policy if the tabbed view is used.
//   	m_mdiArea->setViewMode(QMdiArea::TabbedView);
// 	m_mdiArea->setTabPosition(QTabWidget::South);

  	m_recentProjectsAction->loadEntries( KGlobal::config()->group("Recent Files") );
	m_recentProjectsAction->setEnabled(true);

	if ( !m_fileName.isEmpty() )
		openProject(m_fileName);

	//TODO There is no file to open -> create a new project or open the last used project.
	// Make this selection - new or last used - optional in the settings.
 	updateGUI();
}

void MainWin::initActions() {
	KAction *action;

	// ******************** File-menu *******************************
		//add some standard actions
 	action = KStandardAction::openNew(this, SLOT(newProject()),actionCollection());
	action = KStandardAction::open(this, SLOT(openProject()),actionCollection());
  	m_recentProjectsAction = KStandardAction::openRecent(this, SLOT(openRecentProject(const KUrl&)),actionCollection());
	m_closeAction = KStandardAction::close(this, SLOT(closeProject()),actionCollection());
	m_saveAction = KStandardAction::save(this, SLOT(saveProject()),actionCollection());
	m_saveAsAction = KStandardAction::saveAs(this, SLOT(saveProjectAs()),actionCollection());
	m_printAction = KStandardAction::print(this, SLOT(print()),actionCollection());
	m_printPreviewAction = KStandardAction::printPreview(this, SLOT(printPreview()),actionCollection());

	//New Folder/Spreadsheet/Worksheet/Datasources
	m_newSpreadsheetAction = new KAction(KIcon("insert-table"),i18n("Spreadsheet"),this);
	m_newSpreadsheetAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	actionCollection()->addAction("new_spreadsheet", m_newSpreadsheetAction);
	connect(m_newSpreadsheetAction, SIGNAL(triggered()),SLOT(newSpreadsheet()));

	m_newMatrixAction = new KAction(KIcon("insert-table"),i18n("Matrix"),this);
	m_newMatrixAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	actionCollection()->addAction("new_matrix", m_newMatrixAction);
	connect(m_newMatrixAction, SIGNAL(triggered()),SLOT(newMatrix()));

	m_newWorksheetAction= new KAction(KIcon("archive-insert"),i18n("Worksheet"),this);
	m_newWorksheetAction->setShortcut(Qt::ALT+Qt::Key_X);
	actionCollection()->addAction("new_worksheet", m_newWorksheetAction);
	connect(m_newWorksheetAction, SIGNAL(triggered()), SLOT(newWorksheet()));

	m_newScriptAction = new KAction(KIcon("insert-text"),i18n("Note/Script"),this);
	actionCollection()->addAction("new_script", m_newScriptAction);
	connect(m_newScriptAction, SIGNAL(triggered()),SLOT(newScript()));

	m_newFolderAction = new KAction(KIcon("folder-new"),i18n("Folder"),this);
	actionCollection()->addAction("new_folder", m_newFolderAction);
	connect(m_newFolderAction, SIGNAL(triggered()),SLOT(newFolder()));

	//"New file datasources"
	m_newFileDataSourceAction = new KAction(KIcon("application-octet-stream"),i18n("File Data Source "),this);
	actionCollection()->addAction("new_file_datasource", m_newFileDataSourceAction);
	connect(m_newFileDataSourceAction, SIGNAL(triggered()), this, SLOT(newFileDataSourceActionTriggered()));

	//"New database datasources"
	m_newSqlDataSourceAction = new KAction(KIcon("server-database"),i18n("SQL Data Source "),this);
	actionCollection()->addAction("new_database_datasource", m_newSqlDataSourceAction);
	connect(m_newSqlDataSourceAction, SIGNAL(triggered()), this, SLOT(newSqlDataSourceActionTriggered()));

/*
	//"New plots"
	QActionGroup* newPlotActions = new QActionGroup(this);
	//TODO add KDE-icon
// 	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Plot"),this);
	action = new KAction(KIcon(),i18n("New 2D Plot"),this);
	actionCollection()->addAction("new_2D_plot", action);
	newPlotActions->addAction(action);

	action = new KAction(i18n("New 2D Surface Plot"),this);
	actionCollection()->addAction("new_2D_surface_plot", action);
	newPlotActions->addAction(action);

	action = new KAction(i18n("New 2D Polar Plot"),this);
	actionCollection()->addAction("new_2D_polar_plot", action);
	newPlotActions->addAction(action);

	action = new KAction(i18n("New 3D Plot"),this);
	actionCollection()->addAction("new_3D_plot", action);
	newPlotActions->addAction(action);

	connect(newPlotActions, SIGNAL(triggered(QAction*)), this, SLOT(newPlotActionTriggered(QAction*)));

	//Function plots
   QActionGroup* functionPlotActions = new QActionGroup(this);
	action = new KAction(i18n("New 2D Function Plot"),this);
	actionCollection()->addAction("new_2D_function_plot", action);
	functionPlotActions->addAction(action);

	action = new KAction(i18n("New 2D Surface Function Plot"),this);
	actionCollection()->addAction("new_2D_surface_function_plot", action);
	functionPlotActions->addAction(action);

	action = new KAction(i18n("New 2D Polar Function Plot"),this);
	actionCollection()->addAction("new_2D_polar_function_plot", action);
	functionPlotActions->addAction(action);

	action = new KAction(i18n("New 3D Function Plot"),this);
	actionCollection()->addAction("new_3D_function_plot", action);
	functionPlotActions->addAction(action);

	connect(functionPlotActions, SIGNAL(triggered(QAction*)), this, SLOT(functionPlotActionTriggered(QAction*)));

	//"New data plots"
	QActionGroup* dataPlotActions = new QActionGroup(this);
	action = new KAction(i18n("New 2D Data Plot"),this);
	actionCollection()->addAction("new_2D_data_plot", action);
	dataPlotActions->addAction(action);

	action = new KAction(i18n("New 2D Surface Data Plot"),this);
	actionCollection()->addAction("new_2D_surface_data_plot", action);
	dataPlotActions->addAction(action);

	action = new KAction(i18n("New 2D Polar Data Plot"),this);
	actionCollection()->addAction("new_2D_polar_data_plot", action);
	dataPlotActions->addAction(action);

	action = new KAction(i18n("New 3D Data Plot"),this);
	actionCollection()->addAction("new_3D_data_plot", action);
	dataPlotActions->addAction(action);

	connect(dataPlotActions, SIGNAL(triggered(QAction*)), this, SLOT(dataPlotActionTriggered(QAction*)));
*/
	m_importAction = new KAction(KIcon("document-import-database"), i18n("Import"), this);
	m_importAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_L);
	actionCollection()->addAction("import", m_importAction);
	connect(m_importAction, SIGNAL(triggered()),SLOT(importFileDialog()));
	
	m_exportAction = new KAction(KIcon("document-export-database"), i18n("Export"), this);
	actionCollection()->addAction("export", m_exportAction);
	connect(m_exportAction, SIGNAL(triggered()),SLOT(exportDialog()));

	// Edit
	//Undo/Redo-stuff
	m_undoAction = new KAction(KIcon("edit-undo"),i18n("Undo"),this);
	actionCollection()->addAction("undo", m_undoAction);
	connect(m_undoAction, SIGNAL(triggered()),SLOT(undo()));

	m_redoAction = new KAction(KIcon("edit-redo"),i18n("Redo"),this);
	actionCollection()->addAction("redo", m_redoAction);
	connect(m_redoAction, SIGNAL(triggered()),SLOT(redo()));

	m_historyAction = new KAction(KIcon("view-history"), i18n("Undo/Redo History"),this);
	actionCollection()->addAction("history", m_historyAction);
	connect(m_historyAction, SIGNAL(triggered()),SLOT(historyDialog()));

	// Appearance
	// Analysis
	// Drawing
	// Script

	//Windows
	action  = new KAction(i18n("Cl&ose"), this);
	action->setShortcut(i18n("Ctrl+F4"));
	action->setStatusTip(i18n("Close the active window"));
	actionCollection()->addAction("close window", action);
	connect(action, SIGNAL(triggered()), m_mdiArea, SLOT(closeActiveSubWindow()));

	action = new KAction(i18n("Close &All"), this);
	action->setStatusTip(i18n("Close all the windows"));
	actionCollection()->addAction("close all windows", action);
	connect(action, SIGNAL(triggered()), m_mdiArea, SLOT(closeAllSubWindows()));

	action = new KAction(i18n("&Tile"), this);
	action->setStatusTip(i18n("Tile the windows"));
	actionCollection()->addAction("tile windows", action);
	connect(action, SIGNAL(triggered()), m_mdiArea, SLOT(tileSubWindows()));

	action = new KAction(i18n("&Cascade"), this);
	action->setStatusTip(i18n("Cascade the windows"));
	actionCollection()->addAction("cascade windows", action);
	connect(action, SIGNAL(triggered()), m_mdiArea, SLOT(cascadeSubWindows()));

	action = new KAction(i18n("Ne&xt"), this);
	action->setStatusTip(i18n("Move the focus to the next window"));
	actionCollection()->addAction("next window", action);
	connect(action, SIGNAL(triggered()), m_mdiArea, SLOT(activateNextSubWindow()));

	action = new KAction(i18n("Pre&vious"), this);
	action->setStatusTip(i18n("Move the focus to the previous window"));
	actionCollection()->addAction("previous window", action);
	connect(action, SIGNAL(triggered()), m_mdiArea, SLOT(activatePreviousSubWindow()));

	//"Standard actions"
	KStandardAction::preferences(this, SLOT(settingsDialog()), actionCollection());
	KStandardAction::quit(this, SLOT(close()), actionCollection());

	//Actions for window visibility
	QActionGroup * windowVisibilityActions = new QActionGroup(this);
	windowVisibilityActions->setExclusive(true);
	
	m_visibilityFolderAction = new KAction(KIcon("folder"), tr("Current &Folder Only"), windowVisibilityActions);
	m_visibilityFolderAction->setCheckable(true);
	m_visibilityFolderAction->setChecked(true);
	m_visibilityFolderAction->setData(Project::folderOnly);
	
	m_visibilitySubfolderAction = new KAction(KIcon("folder-documents"), tr("Current Folder and &Subfolders"), windowVisibilityActions);
	m_visibilitySubfolderAction->setCheckable(true);
	m_visibilitySubfolderAction->setData(Project::folderAndSubfolders);
	
	m_visibilityAllAction = new KAction(tr("&All"), windowVisibilityActions);
	m_visibilityAllAction->setCheckable(true);
	m_visibilityAllAction->setData(Project::allMdiWindows);
	
	connect(windowVisibilityActions, SIGNAL(triggered(QAction*)), this, SLOT(setMdiWindowVisibility(QAction*)));
	
	//Actions for hiding/showing the dock widgets
	QActionGroup * docksActions = new QActionGroup(this);
	docksActions->setExclusive(false);
	
	m_toggleProjectExplorerDockAction = new KAction(KIcon("view-list-tree"), i18n("Project explorer"), docksActions);
	m_toggleProjectExplorerDockAction->setCheckable(true);
	m_toggleProjectExplorerDockAction->setChecked(true);
	actionCollection()->addAction("toggle_project_explorer_dock", m_toggleProjectExplorerDockAction);
	
	m_togglePropertiesDockAction = new KAction(KIcon("view-list-details"), i18n("Properties explorer"), docksActions);
	m_togglePropertiesDockAction->setCheckable(true);
	m_togglePropertiesDockAction->setChecked(true);
	actionCollection()->addAction("toggle_properties_explorer_dock", m_togglePropertiesDockAction);
	
	connect(docksActions, SIGNAL(triggered(QAction*)), this, SLOT(toggleDockWidget(QAction*)));
}

void MainWin::initMenus(){
	//menu for adding new aspects
	m_newMenu = new QMenu(i18n("Add new"));
	m_newMenu->setIcon(KIcon("document-new"));
	m_newMenu->addAction(m_newFolderAction);
	m_newMenu->addAction(m_newSpreadsheetAction);
	m_newMenu->addAction(m_newWorksheetAction);
	m_newMenu->addSeparator();
	m_newMenu->addAction(m_newFileDataSourceAction);
	m_newMenu->addAction(m_newSqlDataSourceAction);
	
	//menu subwindow visibility policy
	m_visibilityMenu = new QMenu(i18n("Window visibility policy"));
	m_visibilityMenu->setIcon(KIcon("window-duplicate"));
	m_visibilityMenu ->addAction(m_visibilityFolderAction);
	m_visibilityMenu ->addAction(m_visibilitySubfolderAction);
	m_visibilityMenu ->addAction(m_visibilityAllAction);
}

/*!
	Asks to save the project if it was modified.
	\return \c true if the project still needs to be saved ("cancel" clicked), \c false otherwise.
 */
bool MainWin::warnModified() {
	if(m_project->hasChanged()) {
		int want_save = KMessageBox::warningYesNoCancel( this,
			i18n("The current project %1 has been modified. Do you want to save it?").arg(m_project->name()),
			i18n("Save Project"));
		switch (want_save) {
			case KMessageBox::Yes:
				return !saveProject();
				break;
			case KMessageBox::No:
				break;
			case KMessageBox::Cancel:
				return true;
				break;
		}
	}

	return false;
}

/*!
	disables/enables menu items etc. depending on the currently selected Aspect.
*/
void MainWin::updateGUI() {
	KXMLGUIFactory* factory=this->guiFactory();

	//disable all menus if there is no project
	bool b = (m_project==0);
	m_saveAction->setEnabled(!b);
	m_saveAsAction->setEnabled(!b);
	m_printAction->setEnabled(!b);
	m_printPreviewAction->setEnabled(!b);
	m_importAction->setEnabled(!b);
	m_exportAction->setEnabled(!b);
	m_newSpreadsheetAction->setEnabled(!b);
	m_newWorksheetAction->setEnabled(!b);
	m_closeAction->setEnabled(!b);
	m_toggleProjectExplorerDockAction->setEnabled(!b);
	m_togglePropertiesDockAction->setEnabled(!b);
	factory->container("new", this)->setEnabled(!b);
	factory->container("edit", this)->setEnabled(!b);
	factory->container("spreadsheet", this)->setEnabled(!b);
	factory->container("worksheet", this)->setEnabled(!b);
// 		factory->container("analysis", this)->setEnabled(!b);
//  	factory->container("script", this)->setEnabled(!b);
// 		factory->container("drawing", this)->setEnabled(!b);
	factory->container("windows", this)->setEnabled(!b);

	if (b) {
		m_undoAction->setEnabled(false);
		m_redoAction->setEnabled(false);
		factory->container("worksheet_toolbar", this)->hide();
		factory->container("cartesian_plot_toolbar", this)->hide();
		factory->container("spreadsheet_toolbar", this)->hide();
		return;
	}

	//Activate/deactivate menus and toolbar depending on the currently active window (worksheet or spreadsheet).
	Worksheet* w = this->activeWorksheet();
	if (w!=0){
		//enable worksheet related menus
		factory->container("worksheet", this)->setEnabled(true);
// 		factory->container("drawing", this)->setEnabled(true);

		//disable spreadsheet related menus
		factory->container("spreadsheet", this)->setEnabled(false);
// 		factory->container("analysis", this)->setEnabled(false);
		
		//populate worksheet-menu
		WorksheetView* view=qobject_cast<WorksheetView*>(w->view());
		QMenu* menu=qobject_cast<QMenu*>(factory->container("worksheet", this));
		menu->clear();
		view->createContextMenu(menu);
		
		//populate worksheet-toolbar
		QToolBar* toolbar=qobject_cast<QToolBar*>(factory->container("worksheet_toolbar", this));
		toolbar->setVisible(true);
		toolbar->clear();
		view->fillToolBar(toolbar);
		
		//hide the spreadsheet toolbar
		factory->container("spreadsheet_toolbar", this)->setVisible(false);
	}else{
		//no worksheet selected -> deactivate worksheet related menus and the toolbar
		factory->container("worksheet", this)->setEnabled(false);
//  		factory->container("drawing", this)->setEnabled(false);
		factory->container("worksheet_toolbar", this)->setVisible(false);

		//Handle the Spreadsheet-object
		Spreadsheet* spreadsheet = this->activeSpreadsheet();
		if (spreadsheet){
			//enable spreadsheet related menus
// 			factory->container("analysis", this)->setEnabled(true);
			factory->container("spreadsheet", this)->setEnabled(true);

			//populate spreadsheet-menu
			SpreadsheetView* view=qobject_cast<SpreadsheetView*>(spreadsheet->view());
			QMenu* menu=qobject_cast<QMenu*>(factory->container("spreadsheet", this));
			menu->clear();
			view->createContextMenu(menu);
			
			//populate spreadsheet-toolbar
			QToolBar* toolbar=qobject_cast<QToolBar*>(factory->container("spreadsheet_toolbar", this));
			toolbar->setVisible(true);
			toolbar->clear();
			view->fillToolBar(toolbar);
		}else{
			//no spreadsheet selected -> deactivate spreadsheet related menus
// 			factory->container("analysis", this)->setEnabled(false);
			factory->container("spreadsheet", this)->setEnabled(false);
			factory->container("spreadsheet_toolbar", this)->setVisible(false);
		}
	}
}

/*!
	creates a new empty project. Returns \c true, if a new project was created.
*/
bool MainWin::newProject(){
	//close the current project, if available
	if (!closeProject())
		return false;
	
	if (m_project)
		delete m_project;

	m_project = new Project();
  	m_currentAspect = m_project;
 	m_currentFolder = m_project;

	m_aspectTreeModel = new AspectTreeModel(m_project, this);
		
	//newProject is called for the first time, there is no project explorer yet 
	//-> initialize the project explorer,  the GUI-observer and the dock widgets.
	if ( m_projectExplorer==0 ){
		m_projectExplorerDock = new QDockWidget(this);
		m_projectExplorerDock->setObjectName("projectexplorer");
		m_projectExplorerDock->setWindowTitle(tr("Project Explorer"));
		addDockWidget(Qt::LeftDockWidgetArea, m_projectExplorerDock);
		
		m_projectExplorer = new ProjectExplorer(m_projectExplorerDock);
		m_projectExplorerDock->setWidget(m_projectExplorer);
						
		connect(m_projectExplorer, SIGNAL(currentAspectChanged(AbstractAspect *)),
			this, SLOT(handleCurrentAspectChanged(AbstractAspect *)));
			
		//Properties dock
		m_propertiesDock = new QDockWidget(this);
		m_propertiesDock->setObjectName("aspect_properties_dock");
		m_propertiesDock->setWindowTitle(tr("Properties"));
		addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
		
		stackedWidget = new QStackedWidget(m_propertiesDock);
		m_propertiesDock->setWidget(stackedWidget);
		
		//GUI-observer;
		m_guiObserver = new GuiObserver(this);
	}
	
	m_projectExplorer->setModel(m_aspectTreeModel);
	m_projectExplorer->setProject(m_project);
	m_projectExplorer->setCurrentAspect(m_project);
	
	m_projectExplorerDock->show();
	m_propertiesDock->show();
	updateGUI();

	connect(m_project, SIGNAL(aspectRemoved(const AbstractAspect *, const AbstractAspect *, const AbstractAspect *)),
		this, SLOT(handleAspectRemoved(const AbstractAspect *)));
	connect(m_project, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *)),
		this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect *)));
	connect(m_project, SIGNAL(statusInfo(const QString&)), statusBar(), SLOT(showMessage(const QString&)));
	connect(m_project, SIGNAL(changed()), this, SLOT(projectChanged()));
	connect(m_project, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
	connect(m_project, SIGNAL(requestFolderContextMenu(const Folder*, QMenu*)), this, SLOT(createFolderContextMenu(const Folder*, QMenu*)));
	connect(m_project, SIGNAL(mdiWindowVisibilityChanged()), this, SLOT(updateMdiWindowVisibility()));

 	m_undoViewEmptyLabel = i18n("Project %1 created").arg(m_project->name());
 	setCaption(m_project->name());
	 
	return true;
}

void MainWin::openProject(){
	QString fileName = QFileDialog::getOpenFileName(this,i18n("Open project"),QString::null,
			i18n("LabPlot Projects (*.lml *.lml.gz *.lml.bz2 *.LML *.LML.GZ *.LML.BZ2)"));

	if (!fileName.isEmpty())
		this->openProject(fileName);
}

void MainWin::openProject(const QString& filename) {
	if(filename.isEmpty())
		return;

	QIODevice *file = KFilterDev::deviceForFile(filename,QString::null,true);
	if (file==0)
		file = new QFile(filename);

	if ( file->open( QIODevice::ReadOnly | QFile::Text) == 0) {
		KMessageBox::error(this, i18n("Sorry. Could not open file for reading!"));
		return;
	}

	if (!newProject()){
		file->close();
		delete file;
		return;
	}

	openXML(file);
	file->close();
	delete file;

	m_project->setFileName(filename);
 	m_project->setChanged(false);
	m_project->undoStack()->clear();
	m_undoViewEmptyLabel = i18n("Project %1 opened").arg(m_project->name());
	m_projectExplorer->setCurrentAspect(m_project);
	m_recentProjectsAction->addUrl( KUrl(filename) );
	setCaption(m_project->name());
}

void MainWin::openRecentProject(const KUrl& url) {
	this->openProject(url.path());
}
  
void MainWin::openXML(QIODevice *file) {
	XmlStreamReader reader(file);
	if (m_project->load(&reader) == false) {
		kDebug()<<"ERROR: reading file content"<<endl;
		QString msg_text = reader.errorString();
		KMessageBox::error(this, msg_text, i18n("Error opening project"));
		statusBar()->showMessage(msg_text);
		return;
	}
	if (reader.hasWarnings()) {
		QString msg_text = i18n("The following problems occured when loading the project:\n");
		QStringList warnings = reader.warningStrings();
		foreach(QString str, warnings)
			msg_text += str + "\n";
		KMessageBox::error(this, msg_text, i18n("Project loading partly failed"));
		statusBar()->showMessage(msg_text);
	}
}

/*!
	Closes the current project, if available. Return \c true, if the project was closed.
*/
bool MainWin::closeProject(){
	if (m_project==0)
		return true; //nothing to close

	int b = KMessageBox::warningYesNo( this,
										i18n("The current project %1 will be closed. Do you want to continue?").arg(m_project->name()),
										i18n("Close Project"));
	if (b==KMessageBox::No)
		return false;
	
	if(warnModified())
		return false;

	m_mdiArea->closeAllSubWindows();
	delete m_aspectTreeModel;
	m_aspectTreeModel=0;
	delete m_project;
 	m_project=0;

	m_projectExplorerDock->hide();
	m_propertiesDock->hide();
	m_currentAspect=0;
	m_currentFolder=0;
 	updateGUI();
	return true;
}

/*!
	saves the project to the file \c filename
*/
bool MainWin::saveProject(){
	QString fileName = m_project->fileName();
	if(fileName.isEmpty()) {
		if (!saveProjectAs())
			return false; //cancel clicked in "save as"-dialog
	}

	QIODevice* file = KFilterDev::deviceForFile(fileName, QString::null, true);
	if (file == 0)
		file = new QFile(fileName);

	bool ok;
	if(file->open(QIODevice::WriteOnly | QFile::Text)){
		QXmlStreamWriter writer(file);
		m_project->save(&writer);
		m_project->undoStack()->clear();
		m_project->setChanged(false);
		file->close();
		
		setCaption(m_project->name());
		statusBar()->showMessage(i18n("Project saved"));	
		m_saveAction->setEnabled(false);
		m_saveAsAction->setEnabled(false);
		m_recentProjectsAction->addUrl( KUrl(fileName) );
		ok = true;
	}else{
		KMessageBox::error(this, i18n("Sorry. Could not open file for writing!"));
		ok = false;
	}
	
	if (file != 0)
		delete file;
	return ok;
}

bool MainWin::saveProjectAs() {
	QString fileName = QFileDialog::getSaveFileName(this, i18n("Save project as"),QString::null,
		i18n("LabPlot Projects (*.lml *.lml.gz *.lml.bz2 *.LML *.LML.GZ *.LML.BZ2)"));

	if( fileName.isEmpty() )// "Cancel" was clicked
		return false;

	if( fileName.contains(QString(".lml"),Qt::CaseInsensitive) == false )
		fileName.append(".lml");

	m_project->setFileName(fileName);
	saveProject();
	return true;
}

/*!
	prints the current sheet (worksheet or spreadsheet)
*/
void MainWin::print(){
	QPrinter printer;
	
	//determine first, whether we want to export a worksheet or a spreadsheet
	Worksheet* w=this->activeWorksheet();
	if (w!=0){ //worksheet
		QPrintDialog *dialog = new QPrintDialog(&printer, this);
		dialog->setWindowTitle(tr("Print worksheet"));
		if (dialog->exec() != QDialog::Accepted)
			return;
	 
		WorksheetView* view = qobject_cast<WorksheetView*>(w->view());
		view->print(&printer);
		statusBar()->showMessage(i18n("Worksheet printed"));
	}else{
		//Spreadsheet
		Spreadsheet* s=this->activeSpreadsheet();
		QPrintDialog *dialog = new QPrintDialog(&printer, this);
		dialog->setWindowTitle(tr("Print spreadsheet"));
		if (dialog->exec() != QDialog::Accepted)
			return;
	 
		SpreadsheetView* view = qobject_cast<SpreadsheetView*>(s->view());
		view->print(&printer);
		
		statusBar()->showMessage(i18n("Spreadsheet printed"));
	}
}

void MainWin::printPreview(){
	Worksheet* w=this->activeWorksheet();
	if (w!=0){ //worksheet
		WorksheetView* view = qobject_cast<WorksheetView*>(w->view());
		QPrintPreviewDialog *dialog = new QPrintPreviewDialog(this);
		connect(dialog, SIGNAL(paintRequested(QPrinter*)), view, SLOT(print(QPrinter*)));
		dialog->exec();
	}else{
		//Spreadsheet
		Spreadsheet* s=this->activeSpreadsheet();
		if (s!=0){
			SpreadsheetView* view = qobject_cast<SpreadsheetView*>(s->view());
			QPrintPreviewDialog *dialog = new QPrintPreviewDialog(this);
			connect(dialog, SIGNAL(paintRequested(QPrinter*)), view, SLOT(print(QPrinter*)));
			dialog->exec();
		}
	}
}

/*!
	adds a new Spreadsheet to the project.
*/
void MainWin::newSpreadsheet(){
	Spreadsheet* spreadsheet = new Spreadsheet(0, 100, 2, i18n("Spreadsheet"));
	connect(spreadsheet, SIGNAL(exportRequested()), this, SLOT(exportDialog()));
	connect(spreadsheet, SIGNAL(printRequested()), this, SLOT(print()));
	connect(spreadsheet, SIGNAL(printPreviewRequested()), this, SLOT(printPreview()));
	connect(spreadsheet, SIGNAL(showRequested()), this, SLOT(handleShowSubWindowRequested()));
	this->addAspectToProject(spreadsheet);
}

/*!
 * adds a new Spreadsheet to the project.
 * this slot is only supposed to be called from ImportFileDialog
 */
void MainWin::newSpreadsheetForImportFileDialog(const QString& name){
	if (!m_importFileDialog)
		return;

	Spreadsheet * spreadsheet = new Spreadsheet(0, 100, 2, name);
	this->addAspectToProject(spreadsheet);

	std::auto_ptr<QAbstractItemModel> model(new AspectTreeModel(m_project, this));
	m_importFileDialog->updateModel( model );

	//TODO add Matrix here in future.
	 if ( m_currentAspect->inherits("Spreadsheet") )
		m_importFileDialog->setCurrentIndex( m_projectExplorer->currentIndex());
}
/*!
	adds a new Worksheet to the project.
*/
void MainWin::newWorksheet() {
	Worksheet* worksheet= new Worksheet(0,  i18n("Worksheet"));
	connect(worksheet, SIGNAL(exportRequested()), this, SLOT(exportDialog()));
	connect(worksheet, SIGNAL(printRequested()), this, SLOT(print()));
	connect(worksheet, SIGNAL(printPreviewRequested()), this, SLOT(printPreview()));
	connect(worksheet, SIGNAL(showRequested()), this, SLOT(handleShowSubWindowRequested()));
	this->addAspectToProject(worksheet);
}


/*!
	returns a pointer to a Spreadsheet-object, if the currently active Mdi-Subwindow is \a SpreadsheetView.
	Otherwise returns \a 0.
*/
Spreadsheet* MainWin::activeSpreadsheet() const{
	QMdiSubWindow* win =  m_mdiArea->currentSubWindow();
	if (!win)
		return 0;
	
	AbstractPart* part = qobject_cast<PartMdiView*>(win)->part();
	return  qobject_cast<Spreadsheet*>(part);
}

/*!
	returns a pointer to a Worksheet-object, if the currently active Mdi-Subwindow is \a WorksheetView
	Otherwise returns \a 0.
*/
Worksheet* MainWin::activeWorksheet() const{
	QMdiSubWindow* win =  m_mdiArea->currentSubWindow();
	if (!win)
		return 0;
	
	AbstractPart* part = qobject_cast<PartMdiView*>(win)->part();
	return  qobject_cast<Worksheet*>(part);
}

/*!
	called if there were changes on the the projects.
	Adds "changed" to the window caption and activates the save-Action.
*/
void MainWin::projectChanged(){
	setCaption(m_project->name() + "    [" + i18n("Changed") + "]" );
	m_saveAction->setEnabled(true);
	m_saveAsAction->setEnabled(true);
	m_undoAction->setEnabled(true);
	return;
}

void MainWin::handleCurrentSubWindowChanged(QMdiSubWindow* win){
	PartMdiView *view = qobject_cast<PartMdiView*>(win);
	if (!view)
		return;

	if (view == m_currentSubWindow){
		//do nothing, if the current sub-window gets selected again.
		//This event happens, when labplot loses the focus (modal window is opened or the user switches to another application)
		//and gets it back (modal window is closed or the user switches back to labplot).
		return;
	}else{
		m_currentSubWindow = view;
	}

	updateGUI();
	if (!m_suppressCurrentSubWindowChangedEvent)
		m_projectExplorer->setCurrentAspect(view->part());
}

void MainWin::handleAspectRemoved(const AbstractAspect *parent){
	m_projectExplorer->setCurrentAspect(parent);
}

void MainWin::handleAspectAboutToBeRemoved(const AbstractAspect *aspect){
	const AbstractPart *part = qobject_cast<const AbstractPart*>(aspect);
	if (!part) return;
	PartMdiView *win = part->mdiSubWindow();
	Q_ASSERT(win);
	disconnect(win, SIGNAL(statusChanged(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)),
		this, SLOT(handleSubWindowStatusChange(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)));
	m_mdiArea->removeSubWindow(win);
	updateGUI();
}

/*!
  called when the current aspect in the tree of the project explorer was changed.
  Selects the new aspect.
*/
void MainWin::handleCurrentAspectChanged(AbstractAspect *aspect){
	if (!aspect)
	  aspect = m_project; // should never happen, just in case

	m_suppressCurrentSubWindowChangedEvent = true;
	if(aspect->folder() != m_currentFolder)	{
		m_currentFolder = aspect->folder();
		updateMdiWindowVisibility();
	}

	m_currentAspect = aspect;

	//activate the corresponding MDI sub window for the current aspect
	activateSubWindowForAspect(aspect);
	m_suppressCurrentSubWindowChangedEvent = false;
}

void MainWin::activateSubWindowForAspect(const AbstractAspect* aspect) const {
	const AbstractPart* part = qobject_cast<const AbstractPart*>(aspect);
	if (part) {
		PartMdiView* win = part->mdiSubWindow();
		if (m_mdiArea->subWindowList().indexOf(win) == -1) {
			m_mdiArea->addSubWindow(win);
			connect(win, SIGNAL(statusChanged(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)),
				this, SLOT(handleSubWindowStatusChange(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)));
		}
		win->show();
		m_mdiArea->setActiveSubWindow(win);
	} else {
		AbstractAspect* parent = aspect->parentAspect();
		if (parent)
			activateSubWindowForAspect(parent);
	}
	return;
}

void MainWin::handleSubWindowStatusChange(PartMdiView * view, PartMdiView::SubWindowStatus from, PartMdiView::SubWindowStatus to){
	Q_UNUSED(from);
	Q_UNUSED(to);
	if (view == m_mdiArea->currentSubWindow()) {
		updateGUI();
	}
}

void MainWin::setMdiWindowVisibility(QAction * action){
	m_project->setMdiWindowVisibility((Project::MdiWindowVisibility)(action->data().toInt()));
}

/*!
	shows the sub window of a worksheet or a spreadsheet.
	Used if the window was closed before and the user asks to show
	the window again via the context menu in the project explorer.
*/
void MainWin::handleShowSubWindowRequested() {
	activateSubWindowForAspect(m_currentAspect);
}

void MainWin::newScript(){
	//TODO
}

void MainWin::newMatrix(){
	//TODO
}

void MainWin::newFolder() {
	Folder * folder = new Folder(tr("Folder %1").arg(1));
	this->addAspectToProject(folder);
}

/*!
	this is called on a right click on the root folder in the project explorer
*/
void MainWin::createContextMenu(QMenu * menu) const{
	menu->addMenu(m_newMenu);
	menu->addMenu(m_visibilityMenu);
}

/*!
	this is called on a right click on a non-root folder in the project explorer
*/
void MainWin::createFolderContextMenu(const Folder * folder, QMenu * menu) const{
	Q_UNUSED(folder);

	//Folder provides it's own context menu. Add a separator befor adding additional actions.
	menu->addSeparator();
	menu->addMenu(m_newMenu);
	menu->addMenu(m_visibilityMenu);
}

void MainWin::undo(){
	WAIT_CURSOR;
	m_project->undoStack()->undo();
	if (m_project->undoStack()->index()==0) {
		setCaption(m_project->name());
		m_saveAction->setEnabled(false);
		m_saveAsAction->setEnabled(false);
		m_undoAction->setEnabled(false);
		m_project->setChanged(false);
	}
	m_redoAction->setEnabled(true);
	RESET_CURSOR;
}

void MainWin::redo(){
	WAIT_CURSOR;
	m_project->undoStack()->redo();
	projectChanged();
	if (m_project->undoStack()->index() == m_project->undoStack()->count())
		m_redoAction->setEnabled(false);
	RESET_CURSOR;
}

/*!
	Shows/hides mdi sub-windows depending on the currend visibility policy.
*/
void MainWin::updateMdiWindowVisibility() const{
	QList<QMdiSubWindow *> windows = m_mdiArea->subWindowList();
	PartMdiView * part_view;
	switch(m_project->mdiWindowVisibility()){
		case Project::allMdiWindows:
			foreach(QMdiSubWindow *window, windows){
				part_view = qobject_cast<PartMdiView *>(window);
				Q_ASSERT(part_view);
				part_view->show();
			}
			break;
		case Project::folderOnly:
			foreach(QMdiSubWindow *window, windows){
				part_view = qobject_cast<PartMdiView *>(window);
				qDebug()<<"name "<<part_view->part()->name();
				Q_ASSERT(part_view);
				if(part_view->part()->folder() == m_currentFolder)
					part_view->show();
				else
					part_view->hide();
			}
			break;
		case Project::folderAndSubfolders:
			foreach(QMdiSubWindow *window, windows){
				part_view = qobject_cast<PartMdiView *>(window);
				if(part_view->part()->isDescendantOf(m_currentFolder))
					part_view->show();
				else
					part_view->hide();
			}
			break;
	}
}

void MainWin::toggleDockWidget(QAction* action) const{
	if (action->objectName() == "toggle_project_explorer_dock"){
		if (m_projectExplorerDock->isVisible())
			m_projectExplorerDock->hide();
		else
			m_projectExplorerDock->show();
	}else if (action->objectName() == "toggle_properties_explorer_dock"){
		if (m_propertiesDock->isVisible())
			m_propertiesDock->hide();
		else
			m_propertiesDock->show();
	}
}

void MainWin::closeEvent(QCloseEvent* event) {
	if (!this->closeProject())
		event->ignore();
}

/***************************************************************************************/
/************************************** dialogs ***************************************/
/***************************************************************************************/
/*!
	shows the dialog with the Undo-history.
*/
void MainWin::historyDialog(){
	if (!m_project->undoStack())
		 return;

	HistoryDialog* dialog = new HistoryDialog(this, m_project->undoStack(), m_undoViewEmptyLabel);
	int index = m_project->undoStack()->index();
	if (dialog->exec() != QDialog::Accepted)
		m_project->undoStack()->setIndex(index);
}

/*!
  Opens the dialog to import data to the selected spreadsheet
*/
void MainWin::importFileDialog(){
	m_importFileDialog = new ImportFileDialog(this);
	connect (m_importFileDialog, SIGNAL(newSpreadsheetRequested(const QString&)),
			 this, SLOT(newSpreadsheetForImportFileDialog(const QString&)));
	std::auto_ptr<QAbstractItemModel> model(new AspectTreeModel(m_project, this));
	m_importFileDialog->setModel( model );
	
	//TODO add Matrix here in future.
	 if ( m_currentAspect->inherits("Spreadsheet") )
		m_importFileDialog->setCurrentIndex( m_projectExplorer->currentIndex());
	
	if ( m_importFileDialog->exec() == QDialog::Accepted )
		m_importFileDialog->importToSpreadsheet();

	delete m_importFileDialog;
	m_importFileDialog = 0;
}

/*!
	opens the dialog for the export of the currently active worksheet/spreadsheet.
 */
void MainWin::exportDialog(){
	//determine first, whether we want to export a worksheet or a spreadsheet
	Worksheet* w=this->activeWorksheet();
	if (w!=0){ //worksheet
		ExportWorksheetDialog* dlg = new ExportWorksheetDialog(this);
		dlg->setFileName(w->name());
		if (dlg->exec()==QDialog::Accepted){
			QString path = dlg->path();
			WorksheetView::ExportFormat format = dlg->exportFormat();
			WorksheetView::ExportArea area = dlg->exportArea();
			
			WorksheetView* view = qobject_cast<WorksheetView*>(w->view());
			view->exportToFile(path, format, area);
		}
	}else{//Spreadsheet
		//TODO
	}
}

/*!
	adds a new file data source to the current project.
*/
void MainWin::newFileDataSourceActionTriggered(){
  ImportFileDialog* dlg = new ImportFileDialog(this);
  if ( dlg->exec() == QDialog::Accepted ) {
	  FileDataSource* dataSource = new FileDataSource(0,  i18n("File data source%1").arg(1));
	  dlg->importToFileDataSource(dataSource);
	  this->addAspectToProject(dataSource);
	  kDebug()<<"new file data source created"<<endl;
  }
  delete dlg;
}

/*!
  adds a new SQL data source to the current project.
*/
void MainWin::newSqlDataSourceActionTriggered(){
  //TODO
}

void MainWin::addAspectToProject(AbstractAspect* aspect){
	QModelIndex index = m_projectExplorer->currentIndex();
	if(!index.isValid())
		m_project->addChild(aspect);
	else {
		AbstractAspect * parent_aspect = static_cast<AbstractAspect *>(index.internalPointer());
        // every aspect contained in the project should have a folder
        Q_ASSERT(parent_aspect->folder());
		parent_aspect->folder()->addChild(aspect);
	}
}

void MainWin::settingsDialog(){
	//TODO
	(new SettingsDialog(this))->show();
}
