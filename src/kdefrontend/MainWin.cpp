/***************************************************************************
    File                 : MainWin.cc
    Project              : LabPlot
    --------------------------------------------------------------------
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

//****** GUI **************
#include "FunctionPlotDialog.h"
#include "DataPlotDialog.h"
#include "ImportFileDialog.h"
#include "ProjectDialog.h"
#include "SettingsDialog.h"
#include "AxesDialog.h"
#include "SpreadsheetView.h"

#include <KApplication>
#include <KActionCollection>
#include <KStandardAction>
 #include <kxmlguifactory.h>
#include <KMessageBox>
#include <KStatusBar>
#include <KLocale>
#include <KDebug>
#include <KFilterDev>

#include "pixmaps/pixmap.h" //TODO remove this. Use Qt's resource system instead.

//****** Backend **************
#include "core/Project.h"
#include "core/Folder.h"
#include "core/ProjectExplorer.h"
#include "core/AspectTreeModel.h"
#include "spreadsheet/Spreadsheet.h"
#include "worksheet/Worksheet.h"
#include "worksheet/WorksheetView.h"
// #include "datasources/FileDataSource.h"

 /*!
	\class MainWin
	\brief Main application window.

	\ingroup kdefrontend
 */

MainWin::MainWin(QWidget *parent, const QString& filename)
	: KXmlGuiWindow(parent){

	m_mdi_area = new QMdiArea;
	setCentralWidget(m_mdi_area);
	setCaption("LabPlot "LVERSION);
	statusBar()->showMessage(i18n("Welcome to LabPlot") + " " + LVERSION);
	setupActions();
	setupGUI();
	setAttribute( Qt::WA_DeleteOnClose );

	m_fileName=filename;
  	m_project = 0;
  	m_project_explorer = 0;

	QTimer::singleShot( 0, this, SLOT(initObject()) );
}

MainWin::~MainWin() {
	//write settings
	//TODO m_recentProjectsAction->saveEntries( KGlobal::config()->group("Recent Files") );
	//etc...
	 KGlobal::config()->sync();

	 if (m_project!=0){
		m_mdi_area->closeAllSubWindows();
		disconnect(m_project, 0, this, 0);
		delete m_project;
	}
}

void MainWin::initObject(){
//TODO 	m_recentProjectsAction->loadEntries( KGlobal::config()->group("Recent Files") );

	if ( !m_fileName.isEmpty() )
		openProject(m_fileName);

	//TODO There is no file to open -> create a new project or open the last used project.
	// Make this selection - new or last used - optional in the settings.
 	updateGUI();
}

void MainWin::setupActions() {
	KAction *action;

	// ******************** File-menu *******************************
		//add some standard actions
 	action = KStandardAction::openNew(this, SLOT(newProject()),actionCollection());
	action = KStandardAction::open(this, SLOT(openProject()),actionCollection());
  	KRecentFilesAction* m_recentProjectsAction = KStandardAction::openRecent(this, SLOT(openRecentProject()),actionCollection());
	m_closeAction = KStandardAction::close(this, SLOT(closeProject()),actionCollection());
	m_saveAction = KStandardAction::save(this, SLOT(save()),actionCollection());
	m_saveAsAction = KStandardAction::saveAs(this, SLOT(saveAs()),actionCollection());
	m_printAction = KStandardAction::print(this, SLOT(print()),actionCollection());
	m_printPreviewAction = KStandardAction::printPreview(this, SLOT(printPreview()),actionCollection());

	//New Folder/Spreadsheet/Worksheet/Datasources
	m_newSpreadsheetAction = new KAction(KIcon("insert-table"),i18n("New Spreadsheet"),this);
	m_newSpreadsheetAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	actionCollection()->addAction("new_spreadsheet", m_newSpreadsheetAction);
	connect(m_newSpreadsheetAction, SIGNAL(triggered()),SLOT(newSpreadsheet()));

	m_newMatrixAction = new KAction(KIcon("insert-table"),i18n("New Matrix"),this);
	m_newMatrixAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	actionCollection()->addAction("new_matrix", m_newMatrixAction);
	connect(m_newMatrixAction, SIGNAL(triggered()),SLOT(newMatrix()));

	m_newWorksheetAction= new KAction(KIcon(QIcon(worksheet_xpm)),i18n("New Worksheet"),this);
	m_newWorksheetAction->setShortcut(Qt::ALT+Qt::Key_X);
	actionCollection()->addAction("new_worksheet", m_newWorksheetAction);
	connect(m_newWorksheetAction, SIGNAL(triggered()), SLOT(newWorksheet()));

	m_newScriptAction = new KAction(KIcon("insert-table"),i18n("New Note/Script"),this);
	actionCollection()->addAction("new_script", m_newScriptAction);
	connect(m_newScriptAction, SIGNAL(triggered()),SLOT(newScript()));

	m_newFolderAction = new KAction(KIcon("folder-new"),i18n("New Folder"),this);
	actionCollection()->addAction("new_folder", m_newFolderAction);
	connect(m_newFolderAction, SIGNAL(triggered()),SLOT(newFolder()));

	//"New file datasources"
	QActionGroup* newFileDataSourceActions = new QActionGroup(this);
	action = new KAction(KIcon("application-octet-stream"),i18n("New Vector Data Source "),this);
	actionCollection()->addAction("new_file_vector_datasource", action);
	newFileDataSourceActions->addAction(action);

	action = new KAction(KIcon("table"),i18n("New Matrix Data Source "),this);
	actionCollection()->addAction("new_file_matrix_datasource", action);
	newFileDataSourceActions->addAction(action);

	action = new KAction(KIcon("image-x-generic"),i18n("New Image Data Source "),this);
	actionCollection()->addAction("new_file_image_datasource", action);
	newFileDataSourceActions->addAction(action);

	action = new KAction(KIcon("audio basic"),i18n("New Sound Data Source "),this);
	actionCollection()->addAction("new_file_sound_datasource", action);
	newFileDataSourceActions->addAction(action);

	connect(newFileDataSourceActions, SIGNAL(triggered(QAction*)), this, SLOT(newFileDataSourceActionTriggered(QAction*)));

	//"New database datasources"
	QActionGroup* newSqlDataSourceActions = new QActionGroup(this);
	action = new KAction(KIcon("application-octet-stream"),i18n("New Vector Data Source "),this);
	actionCollection()->addAction("new_database_vector_datasource", action);
	newSqlDataSourceActions->addAction(action);

	action = new KAction(KIcon("table"),i18n("New Matrix Data Source "),this);
	actionCollection()->addAction("new_database_matrix_datasource", action);
	newSqlDataSourceActions->addAction(action);

	action = new KAction(KIcon("image-x-generic"),i18n("New Image Data Source "),this);
	actionCollection()->addAction("new_database_image_datasource", action);
	newSqlDataSourceActions->addAction(action);

	action = new KAction(KIcon("audio basic"),i18n("New Sound Data Source "),this);
	actionCollection()->addAction("new_database_sound_datasource", action);
	newSqlDataSourceActions->addAction(action);

	connect(newFileDataSourceActions, SIGNAL(triggered(QAction*)), this, SLOT(newFileDataSourceActionTriggered(QAction*)));


	//"New plots"
	QActionGroup* newPlotActions = new QActionGroup(this);
	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Plot"),this);
	actionCollection()->addAction("new_2D_plot", action);
	newPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Surface Plot"),this);
	actionCollection()->addAction("new_2D_surface_plot", action);
	newPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Polar Plot"),this);
	actionCollection()->addAction("new_2D_polar_plot", action);
	newPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 3D Plot"),this);
	actionCollection()->addAction("new_3D_plot", action);
	newPlotActions->addAction(action);

	connect(newPlotActions, SIGNAL(triggered(QAction*)), this, SLOT(newPlotActionTriggered(QAction*)));

	//Function plots
   QActionGroup* functionPlotActions = new QActionGroup(this);
	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Function Plot"),this);
	actionCollection()->addAction("new_2D_function_plot", action);
	functionPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Surface Function Plot"),this);
	actionCollection()->addAction("new_2D_surface_function_plot", action);
	functionPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Polar Function Plot"),this);
	actionCollection()->addAction("new_2D_polar_function_plot", action);
	functionPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 3D Function Plot"),this);
	actionCollection()->addAction("new_3D_function_plot", action);
	functionPlotActions->addAction(action);

	connect(functionPlotActions, SIGNAL(triggered(QAction*)), this, SLOT(functionPlotActionTriggered(QAction*)));

	//"New data plots"
	QActionGroup* dataPlotActions = new QActionGroup(this);
	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Data Plot"),this);
	actionCollection()->addAction("new_2D_data_plot", action);
	dataPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Surface Data Plot"),this);
	actionCollection()->addAction("new_2D_surface_data_plot", action);
	dataPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Polar Data Plot"),this);
	actionCollection()->addAction("new_2D_polar_data_plot", action);
	dataPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 3D Data Plot"),this);
	actionCollection()->addAction("new_3D_data_plot", action);
	dataPlotActions->addAction(action);

	connect(dataPlotActions, SIGNAL(triggered(QAction*)), this, SLOT(dataPlotActionTriggered(QAction*)));

	m_importAction = new KAction(KIcon("document-import-database"), i18n("Import"), this);
	m_importAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_L);
	actionCollection()->addAction("import", m_importAction);
	connect(m_importAction, SIGNAL(triggered()),SLOT(importFileDialog()));

	m_projectInfoAction = new KAction (KIcon("help-about"),i18n("Project &Info"), this);
	actionCollection()->addAction("project", m_projectInfoAction);
	connect(m_projectInfoAction, SIGNAL(triggered()),SLOT(projectDialog()));

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
	connect(m_historyAction, SIGNAL(triggered()),SLOT(showHistory()));

	// Appearance
	// Analysis
	// Drawing
	// Script

	// worksheet menu
	action = new KAction (KIcon(QIcon(project_xpm)),i18n("Axes Settings"), this);
	actionCollection()->addAction("axes", action);
	connect(action, SIGNAL(triggered()),SLOT(axesDialog()));

	//Windows
	action  = new KAction(i18n("Cl&ose"), this);
	action->setShortcut(i18n("Ctrl+F4"));
	action->setStatusTip(i18n("Close the active window"));
	actionCollection()->addAction("close window", action);
	connect(action, SIGNAL(triggered()), m_mdi_area, SLOT(closeActiveSubWindow()));

	action = new KAction(i18n("Close &All"), this);
	action->setStatusTip(i18n("Close all the windows"));
	actionCollection()->addAction("close all windows", action);
	connect(action, SIGNAL(triggered()), m_mdi_area, SLOT(closeAllSubWindows()));

	action = new KAction(i18n("&Tile"), this);
	action->setStatusTip(i18n("Tile the windows"));
	actionCollection()->addAction("tile windows", action);
	connect(action, SIGNAL(triggered()), m_mdi_area, SLOT(tileSubWindows()));

	action = new KAction(i18n("&Cascade"), this);
	action->setStatusTip(i18n("Cascade the windows"));
	actionCollection()->addAction("cascade windows", action);
	connect(action, SIGNAL(triggered()), m_mdi_area, SLOT(cascadeSubWindows()));

	action = new KAction(i18n("Ne&xt"), this);
	action->setStatusTip(i18n("Move the focus to the next window"));
	actionCollection()->addAction("next window", action);
	connect(action, SIGNAL(triggered()), m_mdi_area, SLOT(activateNextSubWindow()));

	action = new KAction(i18n("Pre&vious"), this);
	action->setStatusTip(i18n("Move the focus to the previous window"));
	actionCollection()->addAction("previous window", action);
	connect(action, SIGNAL(triggered()), m_mdi_area, SLOT(activatePreviousSubWindow()));

	//"Standard actions"
	KStandardAction::preferences(this, SLOT(settingsDialog()), actionCollection());
	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
}

bool MainWin::warnModified() {
// 	if(m_mdi_area->subWindowList().size() > 0 && m_project->hasChanged()) {
	if(m_project->hasChanged()) {
		int want_save = KMessageBox::warningYesNoCancel( this,
			i18n("The current project has been modified.\nDo you want to save it?"),
			i18n("Save Project"));
		switch (want_save) {
		case KMessageBox::Yes:
			saveProject();
			break;
		case KMessageBox::No:
			break;
		case KMessageBox::Cancel:
			return 1;
			break;
		}
	}

	return 0;
}

/*!
	disables/enables menu items etc. depending on the currently selected Aspect.
*/
void MainWin::updateGUI() {
	KXMLGUIFactory* factory=this->guiFactory();

	//disable all menus if there is no project
	if ( m_project==0 ){
		m_saveAction->setEnabled(false);
		m_saveAsAction->setEnabled(false);
		m_printAction->setEnabled(false);
		m_printPreviewAction->setEnabled(false);
		m_importAction->setEnabled(false);
		m_projectInfoAction->setEnabled(false);
		m_closeAction->setEnabled(false);
		factory->container("new", this)->setEnabled(false);
		factory->container("edit", this)->setEnabled(false);
		factory->container("spreadsheet", this)->setEnabled(false);
		factory->container("worksheet", this)->setEnabled(false);
		factory->container("analysis", this)->setEnabled(false);
 		factory->container("worksheet_view", this)->setEnabled(false);
 		factory->container("script", this)->setEnabled(false);
		factory->container("windows", this)->setEnabled(false);
		factory->container("drawing", this)->setEnabled(false);
		return;
	}else{
		m_saveAction->setEnabled(true);
		m_saveAsAction->setEnabled(true);
		m_printAction->setEnabled(true);
		m_printPreviewAction->setEnabled(true);
		m_importAction->setEnabled(true);
		m_projectInfoAction->setEnabled(true);
		m_closeAction->setEnabled(true);
		factory->container("new", this)->setEnabled(true);
		factory->container("edit", this)->setEnabled(true);
		factory->container("spreadsheet", this)->setEnabled(true);
		factory->container("worksheet", this)->setEnabled(true);
		factory->container("analysis", this)->setEnabled(true);
 		factory->container("worksheet_view", this)->setEnabled(true);
 		factory->container("script", this)->setEnabled(true);
		factory->container("windows", this)->setEnabled(true);
		factory->container("drawing", this)->setEnabled(true);
	}


	KActionCollection* collection=this->actionCollection();

	//Handle the Worksheet-object
	Worksheet* w=this->activeWorksheet();
	if (w==0){
		//no worksheet selected -> deactivate worksheet related menus
		factory->container("worksheet", this)->setEnabled(false);
 		factory->container("worksheet_view", this)->setEnabled(false);
 		factory->container("drawing", this)->setEnabled(false);

		//Handle the Spreadsheet-object
		Spreadsheet* spreadsheet=activeSpreadsheet();
		if (spreadsheet){
			//enable spreadsheet related menus
			factory->container("analysis", this)->setEnabled(true);
			factory->container("spreadsheet", this)->setEnabled(true);

			SpreadsheetView* view=qobject_cast<SpreadsheetView*>(spreadsheet->view());
			QMenu* menu=qobject_cast<QMenu*>(factory->container("spreadsheet", this));
			view->createMenu(menu);
		}else{
			//no spreadsheet selected -> deactivate spreadsheet related menus
			factory->container("analysis", this)->setEnabled(false);
			factory->container("spreadsheet", this)->setEnabled(false);
		}
	}else{
		//enable worksheet related menus
		factory->container("worksheet", this)->setEnabled(true);
		factory->container("worksheet_view", this)->setEnabled(true);
		factory->container("drawing", this)->setEnabled(true);
		factory->container("analysis", this)->setEnabled(true);

		//populate worksheet-menu
		WorksheetView* view=qobject_cast<WorksheetView*>(w->view());
		QMenu* menu=qobject_cast<QMenu*>(factory->container("worksheet", this));
		//view->createMenu(menu);

		//disable spreadsheet related menus
		factory->container("analysis", this)->setEnabled(false);
		factory->container("spreadsheet", this)->setEnabled(false);
	}

	kDebug()<<"GUI updated"<<endl;
}

/*!
	creates a new empty project, initialises the project explorer if called for the first time.
*/
void MainWin::newProject(){
	//close the current project, if available
	if (m_project!=0){
		if(warnModified())
			return;

 		m_mdi_area->closeAllSubWindows();
		m_project->disconnect();
		delete m_project;
	}

	m_project = new Project();
  	m_current_aspect = m_project;
 	m_current_folder = m_project;

	//newProject is called for the first time, there is no project explorer yet -> create one.
	if ( m_project_explorer==0 ){
		initProjectExplorer();
		m_project_explorer->setModel(new AspectTreeModel(m_project, this));
		m_project_explorer->setCurrentAspect(m_project);
	}
	m_project_explorer_dock->show();
	updateGUI();

		connect(m_mdi_area, SIGNAL(subWindowActivated(QMdiSubWindow*)),
			this, SLOT(handleCurrentSubWindowChanged(QMdiSubWindow*)));
	connect(m_project, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)),
		this, SLOT(handleAspectDescriptionChanged(const AbstractAspect *)));
	connect(m_project, SIGNAL(aspectAdded(const AbstractAspect *)),
		this, SLOT(handleAspectAdded(const AbstractAspect *)));
	connect(m_project, SIGNAL(aspectRemoved(const AbstractAspect *, const AbstractAspect *, const AbstractAspect *)),
		this, SLOT(handleAspectRemoved(const AbstractAspect *)));
	connect(m_project, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *)),
		this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect *)));
	connect(m_project, SIGNAL(statusInfo(const QString&)),
			statusBar(), SLOT(showMessage(const QString&)));

	//TODO the signal in Project is not implemented yet.
	connect(m_project, SIGNAL(changed()), this, SLOT(projectChanged()));

	connect(m_project, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
	connect(m_project, SIGNAL(requestFolderContextMenu(const Folder*, QMenu*)), this, SLOT(createFolderContextMenu(const Folder*, QMenu*)));
	connect(m_project, SIGNAL(mdiWindowVisibilityChanged()), this, SLOT(updateMdiWindowVisibility()));

//   	m_project->setChanged(true);
 	m_undoViewEmptyLabel = i18n("Project %1 created").arg(m_project->name());
 	setCaption("LabPlot "  + QString(LVERSION) + "  " + i18n("Project") + " " + m_project->name());
	handleAspectDescriptionChanged(m_project);
	kDebug()<<"new project created"<<endl;

		//TODO
// /*	gvpart=0;
// 	defining_region=0;
// 	defining_line=0;
// 	defining_rect=0;
// 	defining_ellipse=0;
// 	defining_label=false;
// 	defining_image=false;
// 	defining_baseline=false;
// 	defining_maglens=0;
// 	defining_panzoom=0;
// */
}

void MainWin::openProject(){
	QString fileName = QFileDialog::getOpenFileName(this,i18n("Open project"),QString::null,
			i18n("LabPlot Projects (*.lml *.lml.gz *.lml.bz2 *.LML *.LML.GZ *.LML.BZ2)"));
	this->openProject(fileName);
}

void MainWin::openProject(QString filename){
	if(filename.isEmpty())
		return;

	QIODevice *file = KFilterDev::deviceForFile(filename,QString::null,true);
	if (file==0) file = new QFile(filename);
	if ( file->open( QIODevice::ReadOnly | QFile::Text) == 0) {
		KMessageBox::error(this, i18n("Sorry. Could not open file for reading!"));
		return;
	}
	newProject();
	openXML(file);
	m_project->setFileName(filename);
 	m_project->setChanged(false);
	m_undoViewEmptyLabel = i18n("Project %1 opened").arg(m_project->name());
	m_project_explorer->setModel(new AspectTreeModel(m_project, this));
	m_project_explorer->setCurrentAspect(m_project);
	m_recentProjectsAction->addUrl( KUrl(filename) );

	setCaption("LabPlot "  + QString(LVERSION) + "  " + i18n("Project") + " " + m_project->name());
	kDebug()<<"Project "<<filename<<" opened"<<endl;
}

void MainWin::openXML(QIODevice *file) {
	kDebug()<<"	reading ..."<<endl;
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
	file->close();
}

/*!
	Closes the current project, if available.
*/
void MainWin::closeProject(){
	if (m_project==0)
		return;

	if(warnModified())
		return;

	m_mdi_area->closeAllSubWindows();
	m_project->disconnect();
	delete m_project;

	m_project_explorer_dock->hide();
	m_current_aspect=0;
	m_current_folder=0;
 	m_project=0;
 	updateGUI();
}

void MainWin::saveProject(QString filename) {
	if (filename.isEmpty() ) {
		if(m_project->fileName().isEmpty()) {
			saveProjectAs();	// need a file name
			return;
		}
		else
			filename = m_project->fileName();
	}

	kDebug()<<filename<<endl;
	if(m_project->fileName() == filename && m_project->hasChanged() == false) {
		kDebug()<<"no changes to be saved"<<endl;
		return;
	}


	QIODevice *xmlfile = KFilterDev::deviceForFile(filename,QString::null,true);
	if (xmlfile==0) xmlfile = new QFile(filename);
	saveXML(xmlfile);
	m_project->setFileName(filename);

	setCaption("LabPlot "  + QString(LVERSION) + "  " + i18n("Project") + " " + m_project->name());
}

void MainWin::saveXML(QIODevice *file) {
	kDebug()<<endl;

	if(file->open(QIODevice::WriteOnly | QFile::Text)) {
		QXmlStreamWriter writer(file);
		m_project->save(&writer);
		m_project->undoStack()->clear();
		m_project->setChanged(false);
		statusBar()->showMessage(i18n("Project saved"));
		file->close();
	}
	else
		KMessageBox::error(this, i18n("Sorry. Could not open file for writing!"));
}

void MainWin::saveProjectAs() {
	QString fileName = QFileDialog::getSaveFileName(this, i18n("Save project"),QString::null,
		i18n("LabPlot Projects (*.lml *.lml.gz *.lml.bz2 *.LML *.LML.GZ *.LML.BZ2)"));

	if( fileName.isEmpty() )	// "Cancel"
		return;

	if( fileName.contains(QString(".lml"),Qt::CaseInsensitive) == false )
			fileName.append(".lml");

	QFile file(fileName);
	if ( file.exists() ) {
		int answer = KMessageBox::warningYesNoCancel( this,
				i18n( "Overwrite\n\'%1\'?" ).arg(fileName), i18n("Save Project"));
		if (answer == KMessageBox::Cancel)
			return;
		else if (answer == KMessageBox::No)
			saveProjectAs();
	}

	saveProject(fileName);
	m_recentProjectsAction->addUrl( KUrl(fileName) );
	kDebug()<<"Porject saved as "<<fileName<<endl;
}

/*!
	prints the current Worksheet
*/
  //TODO
void MainWin::print() {
//  Worksheet *w = activeWorksheet();
//   if (w)
// 	  w->print();

// 	statusBar()->showMessage(i18n("Printed worksheet"));
}

/*!
	adds a new Table (Spreadsheet) to the project.
*/
Spreadsheet* MainWin::newSpreadsheet() {
	Spreadsheet * spreadsheet = new Spreadsheet(0, 100, 2, i18n("Spreadsheet %1").arg(1));

	QModelIndex index = m_project_explorer->currentIndex();

	if(!index.isValid())
		m_project->addChild(spreadsheet);
	else {
		AbstractAspect * parent_aspect = static_cast<AbstractAspect *>(index.internalPointer());
		Q_ASSERT(parent_aspect->folder()); // every aspect contained in the project should have a folder
		parent_aspect->folder()->addChild(spreadsheet);
	}

	kDebug()<<"new spreadsheet created"<<endl;
    return spreadsheet;
}

/*!
	adds a new Worksheet to the project.
*/
Worksheet* MainWin::newWorksheet() {
	Worksheet* worksheet= new Worksheet(0,  i18n("Worksheet %1").arg(1));
	QModelIndex index = m_project_explorer->currentIndex();

	if(!index.isValid())
		m_project->addChild(worksheet);
	else {
		AbstractAspect * parent_aspect = static_cast<AbstractAspect *>(index.internalPointer());
		Q_ASSERT(parent_aspect->folder()); // every aspect contained in the project should have a folder
		parent_aspect->folder()->addChild(worksheet);
	}

	kDebug()<<"new worksheet created"<<endl;
    return worksheet;
}


/*!
	returns a pointer to a Spreadsheet-object, if the currently active/selected Aspect is of type \a Spreadsheet.
	Otherwise returns \a 0.
*/
Spreadsheet* MainWin::activeSpreadsheet() const{
	Spreadsheet* t=0;
	if ( m_current_aspect )
  		t=qobject_cast<Spreadsheet*>(m_current_aspect);

	return t;
}

/*!
	returns a pointer to a Worksheet-object, if the currently active/selected Aspect is of type \a Worksheet.
	Otherwise returns \a 0.
*/
Worksheet* MainWin::activeWorksheet() const{
	Worksheet* w=0;
	if ( m_current_aspect )
  		w=qobject_cast<Worksheet*>(m_current_aspect);

	return w;
}

//TODO remove?
// SpreadsheetView* MainWin::getSpreadsheet(QString name) const{
// // TODO: port to use aspects
// 	QList<QMdiSubWindow *> wlist = m_mdi_area->subWindowList();
// 	for (int i=0; i<wlist.size(); i++)
// 		if(wlist.at(i)->windowTitle() == name)
// 			return (SpreadsheetView *)wlist.at(i);
// 	return 0;
// }


//TODO remove?
// Worksheet* MainWin::getWorksheet(QString name) const {
// TODO: port to use aspects
// 	QList<QMdiSubWindow *> wlist = m_mdi_area->subWindowList();
// 	for (int i=0; i<wlist.size(); i++)
// 		if(wlist.at(i)->windowTitle() == name)
// 			return (Worksheet *)wlist.at(i);
//  	return 0;
// }


/******************** dialogs *****************************/
void MainWin::importFileDialog(){
// 	ImportFileDialog* dlg = new ImportFileDialog(this);
// 	if ( dlg->exec() == QDialog::Accepted ) {
//   		FileDataSource* dataSource = new FileDataSource(0,  i18n("File data source%1").arg(1));
// // 		dlg->saveSettings(dataSource);
// // 		dataSource->read();
//
// 		QModelIndex index = m_project_explorer->currentIndex();
// 		if(!index.isValid())
// 			m_project->addChild(dataSource);
// 		else {
// 			AbstractAspect * parent_aspect = static_cast<AbstractAspect *>(index.internalPointer());
// 			Q_ASSERT(parent_aspect->folder()); // every aspect contained in the project should have a folder
// 			parent_aspect->folder()->addChild(dataSource);
// 		}
//
// 		kDebug()<<"new file data source created"<<endl;
// 	}
}

void MainWin::axesDialog() { (new AxesDialog(this))->show(); }

void MainWin::projectDialog(){
	ProjectDialog* dlg = new ProjectDialog(this, m_project);
	dlg->show();
}


/*!
	creates a new worksheet if there are no sheets (worksheet or spreadsheet) in the model at all.
	TODO: dirty hack. redesign and remove this function and hasSheet().
*/
void MainWin::ensureSheet(){
	if (this->hasSheet(m_project_explorer->model()->index(0,0))==false)
		this->newWorksheet();
}

bool MainWin::hasSheet(const QModelIndex & index) const{
	int rows = index.model()->rowCount(index);
	AbstractAspect *aspect;
	for (int i=0; i<rows; i++) {
		QModelIndex currentChild = index.child(i, 0);
		hasSheet(currentChild);
		aspect =  static_cast<AbstractAspect*>(currentChild.internalPointer());
		if (aspect->inherits("Worksheet") || aspect->inherits("Spreadsheet"))
				return true;
	}
	return false;
}

/*!
	adds a new file data source to the current project.
*/
void MainWin::newFileDataSourceActionTriggered(QAction* action){
	//TODO
}

/*!
	adds a new file data source to the current project.
*/
void MainWin::newSqlDataSourceActionTriggered(QAction* action){
	//TODO
}


/*!
	adds a new plot to the current worksheet.
*/
void MainWin::newPlotActionTriggered(QAction* action){
	Plot::PlotType type;
	QString name=action->objectName();
	kDebug()<<name<<endl;
	if (name == "new_2D_plot")
		type=Plot::PLOT2D;
	else if (name == "new_2D_surface_plot")
		type=Plot::PLOTSURFACE;
	else if (name == "new_2D_polar_plot")
		type=Plot::PLOTPOLAR;
	else
		type=Plot::PLOT3D;

	this->ensureSheet();
	Worksheet* w=activeWorksheet();
	if (w){
		//w->createPlot(type);
		this->updateGUI();
	}
}

/*!
	shows the \a FunctionPlotDialog.
	Creates a data set from a function and adds it to the current Worksheet or Spreadsheet.
*/
void MainWin::functionPlotActionTriggered(QAction* action){
	Plot::PlotType type;
	QString name=action->objectName();
	if (name == "new_2D_function_plot")
		type=Plot::PLOT2D;
	else if (name == "new_2D_surface_function_plot")
		type=Plot::PLOTSURFACE;
	else if (name == "new_2D_polar_function_plot")
		type=Plot::PLOTPOLAR;
	else
		type=Plot::PLOT3D;

	this->ensureSheet();

	FunctionPlotDialog* dlg = new FunctionPlotDialog(this, type);
	AspectTreeModel* model=new AspectTreeModel(m_project, this);
	model->setFolderSelectable(false);
	dlg->setModel( model );
 	dlg->setCurrentIndex( m_project_explorer->currentIndex());

	if ( dlg->exec() == QDialog::Accepted ) {
		Set set(Set::SET2D);
		dlg->saveSet(&set);
		QModelIndex index=dlg->currentIndex();
		AbstractAspect * aspect = static_cast<AbstractAspect *>(index.internalPointer());
		Worksheet* w=0;
		w=qobject_cast<Worksheet*>(aspect);
		if (w!=0){
			//w->addSet(set, type);
		}else{
			//TODO
// 			Spreadsheet* t=0;
// 			if (t!=0){
// 				t->addSet(set, type);
		}
	}
}

void MainWin::dataPlotActionTriggered(QAction* action){
	Plot::PlotType type;
	QString name=action->objectName();
	if (name == "new_2D_data_plot")
		type=Plot::PLOT2D;
	else if (name == "new_2D_surface_data_plot")
		type=Plot::PLOTSURFACE;
	else if (name == "new_2D_polar_data_plot")
		type=Plot::PLOTPOLAR;
	else
		type=Plot::PLOT3D;

	this->ensureSheet();

	DataPlotDialog* dlg = new DataPlotDialog(this, type);
	AspectTreeModel* model=new AspectTreeModel(m_project, this);
	model->setFolderSelectable(false);
	dlg->setModel( model );
	dlg->setCurrentIndex( m_project_explorer->currentIndex());

	if ( dlg->exec() == QDialog::Accepted ) {
		//TODO
	}
}

void MainWin::settingsDialog(){
	//TODO
	(new SettingsDialog(this))->show();
}
/******************** dialogs end *****************************/

/*!
	called if there were changes on the the projects.
	Adds "changed" to the window caption and activates the save-Action.
*/
void MainWin::projectChanged(){
 	setCaption( "LabPlot "  + QString(LVERSION) + "  " + i18n("Project") + " " + m_project->name() + "    [" + i18n("Changed") + "]" );
	//TODO enable the save-action
	return;
}

void MainWin::handleCurrentSubWindowChanged(QMdiSubWindow* win)
{
	PartMdiView *view = qobject_cast<PartMdiView*>(win);
	if (!view) return;
	emit partActivated(view->part());
	m_project_explorer->setCurrentAspect(view->part());
	updateGUI();
}

void MainWin::handleAspectDescriptionChanged(const AbstractAspect *aspect){
	if (aspect == static_cast<AbstractAspect *>(m_project))
		setCaption("LabPlot "  + QString(LVERSION) + "  " + i18n("Project") + " " + m_project->name());
}

void MainWin::handleAspectAdded(const AbstractAspect *aspect)
{
	handleAspectAddedInternal(aspect);
	updateMdiWindowVisibility();
	handleCurrentSubWindowChanged(m_mdi_area->currentSubWindow());
}

void MainWin::handleAspectAddedInternal(const AbstractAspect * aspect)
{
	foreach(const AbstractAspect * child, aspect->children<AbstractAspect>())
		handleAspectAddedInternal(child);

	const AbstractPart *part = qobject_cast<const AbstractPart*>(aspect);
	if (part)
	{
		PartMdiView *win = part->mdiSubWindow();
		Q_ASSERT(win);
		m_mdi_area->addSubWindow(win);
		connect(win, SIGNAL(statusChanged(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)),
				this, SLOT(handleSubWindowStatusChange(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)));
	}
}

void MainWin::handleAspectRemoved(const AbstractAspect *parent)
{
	m_project_explorer->setCurrentAspect(parent);
}

void MainWin::handleAspectAboutToBeRemoved(const AbstractAspect *aspect)
{
	const AbstractPart *part = qobject_cast<const AbstractPart*>(aspect);
	if (!part) return;
	PartMdiView *win = part->mdiSubWindow();
	Q_ASSERT(win);
	disconnect(win, SIGNAL(statusChanged(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)),
		this, SLOT(handleSubWindowStatusChange(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)));
	m_mdi_area->removeSubWindow(win);
	updateGUI();
}

/*!
	Initialises the  project explorer and the corresponding dock widget.
*/
void MainWin::initProjectExplorer(){
	m_project_explorer_dock = new QDockWidget(this);
	m_project_explorer_dock->setWindowTitle(tr("Project Explorer"));
	m_project_explorer = new ProjectExplorer(m_project_explorer_dock);
// 	m_project_explorer->setModel(new AspectTreeModel(m_project, this));
	m_project_explorer_dock->setWidget(m_project_explorer);
	addDockWidget(Qt::BottomDockWidgetArea, m_project_explorer_dock);
	connect(m_project_explorer, SIGNAL(currentAspectChanged(AbstractAspect *)),
		this, SLOT(handleCurrentAspectChanged(AbstractAspect *)));
// 	m_project_explorer->setCurrentAspect(m_project);
}

void MainWin::handleCurrentAspectChanged(AbstractAspect *aspect)
{
	if(!aspect) aspect = m_project; // should never happen, just in case
	if(aspect->folder() != m_current_folder)
	{
		m_current_folder = aspect->folder();
		updateMdiWindowVisibility();
	}
//  	if(aspect != m_current_aspect)
//  	{
 		m_current_aspect = aspect;
		AbstractPart * part = qobject_cast<AbstractPart*>(aspect);
		if (part)
			m_mdi_area->setActiveSubWindow(part->mdiSubWindow());
//   	}
//  	m_current_aspect = aspect;
	kDebug()<<"current aspect  "<<m_current_aspect->name()<<endl;
}

void MainWin::handleSubWindowStatusChange(PartMdiView * view, PartMdiView::SubWindowStatus from, PartMdiView::SubWindowStatus to){
	Q_UNUSED(from);
	Q_UNUSED(to);
	if (view == m_mdi_area->currentSubWindow()) {
		updateGUI();
	}
}

void MainWin::setMdiWindowVisibility(QAction * action)
{
	// TODO: write a KAction based version of this
#if 0
	m_project->setMdiWindowVisibility((Project::MdiWindowVisibility)(action->data().toInt()));
#endif
}

void MainWin::newScript(){
	//TODO
}

Folder* MainWin::newFolder() {
	kDebug()<<endl;

	Folder * folder = new Folder(tr("Folder %1").arg(1));

	QModelIndex index = m_project_explorer->currentIndex();

	if(!index.isValid())
		m_project->addChild(folder);
	else
	{
		AbstractAspect * parent_aspect = static_cast<AbstractAspect *>(index.internalPointer());
		Q_ASSERT(parent_aspect->folder()); // every aspect contained in the project should have a folder
		parent_aspect->folder()->addChild(folder);
	}

	updateGUI();

    return folder;
}

/*!
	shows the dialog with the Undo-history.
*/
void MainWin::showHistory()
{
	if (!m_project->undoStack())
		 return;

	KDialog dialog;
	dialog.setWindowIcon( KIcon("view-history") );
	dialog.setWindowTitle(i18n("Undo/Redo History"));
	dialog.setButtons( KDialog::Ok | KDialog::Cancel );

	int index = m_project->undoStack()->index();
	QUndoView undo_view(m_project->undoStack());
	undo_view.setCleanIcon( KIcon("edit-clear-history") );
 	undo_view.setEmptyLabel(m_undoViewEmptyLabel);
	undo_view.setMinimumWidth(350);
	undo_view.setWhatsThis(i18n("List of all performed steps/actions.")+"\n"
			 + i18n("Select an item in the list to navigate to the corresponding step."));
	dialog.setMainWidget(&undo_view);

	if (dialog.exec() == QDialog::Accepted)
		return;

	m_project->undoStack()->setIndex(index);
}

/*!
	this is called on a right click on the root folder in the project explorer
*/
void MainWin::createContextMenu(QMenu * menu) const
{
	kDebug()<<"ffffffffffff";
	menu->addAction(m_newFolderAction);
	menu->addAction(m_newSpreadsheetAction);
	menu->addAction(m_newWorksheetAction);
}

/*!
	this is called on a right click on a non-root folder in the project explorer
*/
void MainWin::createFolderContextMenu(const Folder * folder, QMenu * menu) const{
	Q_UNUSED(folder);

	//Folder provides it's own context menu. Add a separator befor adding additional actions.
	menu->addSeparator();
	menu->addAction(m_newFolderAction);
	menu->addAction(m_newSpreadsheetAction);
	menu->addAction(m_newWorksheetAction);
}

void MainWin::undo(){
	WAIT_CURSOR;
	m_project->undoStack()->undo();
	RESET_CURSOR;
}

void MainWin::redo(){
	WAIT_CURSOR;
	m_project->undoStack()->redo();
	RESET_CURSOR;
}

/*!
	Shows/hides mdi windows depending on the currend folder
*/
void MainWin::updateMdiWindowVisibility()
{
	QList<QMdiSubWindow *> windows = m_mdi_area->subWindowList();
	PartMdiView * part_view;
	switch(m_project->mdiWindowVisibility())
	{
		case Project::allMdiWindows:
			foreach(QMdiSubWindow *window, windows)
			{
				part_view = qobject_cast<PartMdiView *>(window);
				Q_ASSERT(part_view);
				part_view->show();
			}
			break;
		case Project::folderOnly:
			foreach(QMdiSubWindow *window, windows)
			{
				part_view = qobject_cast<PartMdiView *>(window);
				Q_ASSERT(part_view);
				if(part_view->part()->folder() == m_current_folder)
					part_view->show();
				else
					part_view->hide();
			}
			break;
		case Project::folderAndSubfolders:
			foreach(QMdiSubWindow *window, windows)
			{
				part_view = qobject_cast<PartMdiView *>(window);
				if(part_view->part()->isDescendantOf(m_current_folder))
					part_view->show();
				else
					part_view->hide();
			}
			break;
	}
}
