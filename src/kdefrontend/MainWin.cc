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
#include "AxesDialog.h"
#include "FunctionPlotDialog.h"
#include "ImportDialog.h"
#include "LegendDialog.h"
#include "ProjectDialog.h"
#include "SettingsDialog.h"
#include "TitleDialog.h"

#include <KApplication>
#include <KActionCollection>
#include <KStandardAction>
 #include <kxmlguifactory.h>
#include <KMessageBox>
#include <KStatusBar>
#include <KLocale>
#include <KDebug>
#include <KFilterDev>

#include "Spreadsheet.h"
#include "Worksheet.h"
#include "elements/Set.h"
#include "plots/Plot.h"
#include "pixmaps/pixmap.h" //TODO remove this. Use Qt's resource system instead.

#include "core/Project.h"
#include "core/Folder.h"
#include "core/ProjectExplorer.h"
#include "core/AspectTreeModel.h"
#include "table/Table.h"

MainWin::MainWin(QWidget *parent, const QString& filename)
	: KXmlGuiWindow(parent)
{
	m_fileName=filename;
	m_mdi_area = new QMdiArea;
	setCentralWidget(m_mdi_area);

	setCaption("LabPlot "LVERSION);
	setupActions();
	setupGUI();

	m_project = NULL;
 	openNew();

 	m_current_aspect = m_project;
	m_current_folder = m_project;

	initProjectExplorer();

	connect(m_mdi_area, SIGNAL(subWindowActivated(QMdiSubWindow*)),
			this, SLOT(handleCurrentSubWindowChanged(QMdiSubWindow*)));
	connect(m_project, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)),
		this, SLOT(handleAspectDescriptionChanged(const AbstractAspect *)));
	connect(m_project, SIGNAL(aspectAdded(const AbstractAspect *, int)),
		this, SLOT(handleAspectAdded(const AbstractAspect *, int)));
	connect(m_project, SIGNAL(aspectRemoved(const AbstractAspect *, int)),
		this, SLOT(handleAspectRemoved(const AbstractAspect *, int)));
	connect(m_project, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *, int)),
		this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect *, int)));
	connect(m_project, SIGNAL(statusInfo(const QString&)),
			statusBar(), SLOT(showMessage(const QString&)));

	handleAspectDescriptionChanged(m_project);

	spreadsheetmenu = static_cast<QMenu*> (guiFactory()->container("spreadsheet",this));
	connect(spreadsheetmenu, SIGNAL(aboutToShow()), SLOT(SpreadsheetMenu()) );

	statusBar()->showMessage(i18n("Welcome to LabPlot") + " " + LVERSION);

	connect(m_project, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));
	connect(m_project, SIGNAL(requestFolderContextMenu(const Folder*, QMenu*)), this, SLOT(createFolderContextMenu(const Folder*, QMenu*)));
	connect(m_project, SIGNAL(mdiWindowVisibilityChanged()), this, SLOT(updateMdiWindowVisibility()));

	QTimer::singleShot( 0, this, SLOT(initObject()) );
}

MainWin::~MainWin() {
	delete m_project;
}

void MainWin::initObject(){
	if ( !m_fileName.isEmpty() )
		open(m_fileName);
}

void MainWin::setupActions() {
	// File
	KAction *action = KStandardAction::openNew(this, SLOT(openNew()),actionCollection());
	action = KStandardAction::open(this, SLOT(open()),actionCollection());
	action->setEnabled(false);
	action = KStandardAction::save(this, SLOT(save()),actionCollection());
	action = KStandardAction::saveAs(this, SLOT(saveAs()),actionCollection());
	action = KStandardAction::print(this, SLOT(print()),actionCollection());
	action = KStandardAction::printPreview(this, SLOT(printPreview()),actionCollection());
	action->setEnabled(false);

	// Doesn't work	: action = new KAction("import", i18n("Import"), actionCollection(), "import");
	action = new KAction(KIcon("document-import-database"), i18n("Import"), this);
	action->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_L);
	actionCollection()->addAction("import", action);
	connect(action, SIGNAL(triggered()),SLOT(importDialog()));

	action = new KAction (KIcon(QIcon(project_xpm)),i18n("Project &Info"), this);
	action->setShortcut(Qt::ALT+Qt::Key_V);
	actionCollection()->addAction("project", action);
	connect(action, SIGNAL(triggered()),SLOT(projectDialog()));

	// Edit
	//Undo/Redo-stuff
	m_undoAction = new KAction(KIcon("edit-undo"),i18n("Undo"),this);
	actionCollection()->addAction("undo", m_undoAction);
	connect(m_undoAction, SIGNAL(triggered()),SLOT(undo()));

	m_redoAction = new KAction(KIcon("edit-redo"),i18n("Redo"),this);
	actionCollection()->addAction("redo", m_redoAction);
	connect(m_redoAction, SIGNAL(triggered()),SLOT(redo()));

	m_historyAction = new KAction(i18n("Undo/Redo History"),this);
	actionCollection()->addAction("history", m_historyAction);
	connect(m_historyAction, SIGNAL(triggered()),SLOT(showHistory()));

	//New Folder/Spreasheet/Worksheet
	m_newSpreadsheetAction = new KAction(KIcon("insert-table"),i18n("New Spreadsheet"),this);
	m_newSpreadsheetAction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	actionCollection()->addAction("new spreadsheet", m_newSpreadsheetAction);
	connect(m_newSpreadsheetAction, SIGNAL(triggered()),SLOT(newSpreadsheet()));

	m_newFolderAction = new KAction(KIcon("folder-new"),i18n("New Folder"),this);
	actionCollection()->addAction("new folder", m_newFolderAction);
	connect(m_newFolderAction, SIGNAL(triggered()),SLOT(newFolder()));

	m_newWorksheetAction= new KAction(KIcon(QIcon(worksheet_xpm)),i18n("New Worksheet"),this);
	m_newWorksheetAction->setShortcut(Qt::ALT+Qt::Key_X);
	actionCollection()->addAction("new worksheet", m_newWorksheetAction);
	connect(m_newWorksheetAction, SIGNAL(triggered()), SLOT(newWorksheet()));

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
   QActionGroup* functionActions = new QActionGroup(this);
	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Function Plot"),this);
	actionCollection()->addAction("new_2D_function_plot", action);
	functionActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Surface Function Plot"),this);
	actionCollection()->addAction("new_2D_surface_function_plot", action);
	functionActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Polar Function Plot"),this);
	actionCollection()->addAction("new_2D_polar_function_plot", action);
	functionActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 3D Function Plot"),this);
	actionCollection()->addAction("new_3D_function_plot", action);
	functionActions->addAction(action);

	connect(functionActions, SIGNAL(triggered(QAction*)), this, SLOT(functionActionTriggered(QAction*)));


	//"New data plots"
	QActionGroup* newDataPlotActions = new QActionGroup(this);
	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Data Plot"),this);
	actionCollection()->addAction("new_2D_data_plot", action);
	newDataPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Surface Data Plot"),this);
	actionCollection()->addAction("new_2D_surface_data_plot", action);
	newDataPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 2D Polar Data Plot"),this);
	actionCollection()->addAction("new_2D_polar_data_plot", action);
	newDataPlotActions->addAction(action);

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New 3D Data Plot"),this);
	actionCollection()->addAction("new_3D_data_plot", action);
	newDataPlotActions->addAction(action);


	// Appearance
	action = new KAction(KIcon("draw-text"),i18n("Title Settings"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_T);
	actionCollection()->addAction("title settings", action);
	action->setWhatsThis(i18n("Changes the title settings of the active plot"));
	connect(action, SIGNAL(triggered()), SLOT(titleDialog()));

	action = new KAction(KIcon(QIcon(axes_xpm)),i18n("Axes Settings"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_A);
	actionCollection()->addAction("axes settings", action);
	action->setWhatsThis(i18n("Changes the axis settings of the active plot"));
	connect(action, SIGNAL(triggered()), SLOT(axesDialog()));

	action = new KAction(KIcon("format-list-unordered"),i18n("Legend Settings"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_L);
	actionCollection()->addAction("legend settings", action);
	action->setWhatsThis(i18n("Changes the legend settings of the active plot"));
	connect(action, SIGNAL(triggered()), SLOT(legendDialog()));

	action = new KAction(KIcon(QIcon(set_xpm)),i18n("Plot Settings"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_J);
	action->setWhatsThis(i18n("Changes the settings of the active plot"));
	actionCollection()->addAction("plot settings", action);
	connect(action, SIGNAL(triggered()), SLOT(plotDialog()));

	action = new KAction(KIcon(QIcon(worksheet_xpm)),i18n("Worksheet Settings"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_W);
	action->setWhatsThis(i18n("Changes the settings of the active worksheet"));
	actionCollection()->addAction("worksheet settings", action);
	connect(action, SIGNAL(triggered()), SLOT(worksheetDialog()));

	// Analysis
	// Drawing
	// Script

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
	if(m_mdi_area->subWindowList().size() > 0 && m_project->hasChanged()) {
		int want_save = KMessageBox::warningYesNoCancel( this,
			i18n("The current project has been modified.\nDo you want to save it?"),
			i18n("Save Project"));
		switch (want_save) {
		case KMessageBox::Yes:
			save();
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

void MainWin::updateSheetList() {
	// TODO
}

void MainWin::updateSetList() {
	// TODO
}

/*!
	disables/enables menu items etc. depending on the currently selected Aspect.
*/
void MainWin::updateGUI() {
	updateSheetList();
	updateSetList();
	KXMLGUIFactory* factory=this->guiFactory();
	KActionCollection* collection=this->actionCollection();

	//Handle the Worksheet-object
	Worksheet* w=this->activeWorksheet();
	if (w==0){
		//no workseet selected->deactivate the corresponding menus
		factory->container("appearance", this)->setEnabled(false);
		collection->action("worksheet settings")->setEnabled(false);
		factory->container("view", this)->setEnabled(false);
		factory->container("drawing", this)->setEnabled(false);

		//Handle the Table-object
		if (activeTable()){
			factory->container("analysis", this)->setEnabled(true);
		}else{
			factory->container("analysis", this)->setEnabled(false);
		}
	}else{
		//workseet selected->activate corresponding menus
		factory->container("appearance", this)->setEnabled(true);
		collection->action("worksheet settings")->setEnabled(true);
		factory->container("view", this)->setEnabled(true);
		factory->container("drawing", this)->setEnabled(true);
		factory->container("analysis", this)->setEnabled(true);

 		if (w->plotCount()==0){
 			//no plots available->deactivate "Plot appearance" actions
			collection->action("plot settings")->setEnabled(false);
			collection->action("title settings")->setEnabled(false);
			collection->action("axes settings")->setEnabled(false);
			collection->action("legend settings")->setEnabled(false);
 		}else{
 			//plots available->activate "Plot appearance" menus
			collection->action("plot settings")->setEnabled(true);
			collection->action("title settings")->setEnabled(true);
			collection->action("axes settings")->setEnabled(true);
			collection->action("legend settings")->setEnabled(true);
 		}
	}

	kDebug()<<"GUI updated"<<endl;
}

void MainWin::openNew() {
 	kDebug()<<"MainWin::New()"<<endl;
	if(warnModified()) return;

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
	m_mdi_area->closeAllSubWindows();
	delete m_project;
	m_current_aspect=0;
	m_current_folder=0;
	updateGUI();
 	m_project = new Project(this);
 	m_project->setChanged(true);
}

void MainWin::open(QString filename) {
	kDebug()<<filename<<endl;
	if (filename.isEmpty())
		filename = QFileDialog::getOpenFileName(this,i18n("Open project"),QString::null,
			i18n("LabPlot Projects (*.lml *.lml.gz *.lml.bz2 *.LML *.LML.GZ *.LML.BZ2)"));
	if(filename.isEmpty())
		return;

	QIODevice *file = KFilterDev::deviceForFile(filename,QString::null,true);
	if (file==0) file = new QFile(filename);
	if ( file->open( QIODevice::ReadOnly | QFile::Text) == 0) {
		KMessageBox::error(this, i18n("Sorry. Could not open file for reading!"));
		return;
	}
	openNew();
	openXML(file);
	m_project->setFileName(filename);
 	m_project->setChanged(false);
	//recent->addURL(fn);

	setCaption("LabPlot "LVERSION+i18n(" : ")+m_project->fileName());
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

void MainWin::save(QString filename) {
	if (filename.isEmpty() ) {
		if(m_project->fileName().isEmpty()) {
			saveAs();	// need a file name
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

	setCaption("LabPlot "LVERSION+i18n(" : ")+m_project->fileName());
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

void MainWin::saveAs() {
	kDebug()<<endl;
	QString fn = QFileDialog::getSaveFileName(this, i18n("Save project"),QString::null,
		i18n("LabPlot Projects (*.lml *.lml.gz *.lml.bz2 *.LML *.LML.GZ *.LML.BZ2)"));
	if(fn.isEmpty())	// "Cancel"
		return;

	if(fn.contains(QString(".lml"),Qt::CaseInsensitive) == false)
			fn.append(".lml");

	QFile file(fn);
	if ( file.exists() ) {
		int answer = KMessageBox::warningYesNoCancel( this,
				i18n( "Overwrite\n\'%1\'?" ).arg(fn), i18n("Save Project"));
		if (answer == KMessageBox::Cancel)
			return;
		else if (answer == KMessageBox::No)
			saveAs();
	}

	save(fn);
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

/************** sheet stuff *************************/
void MainWin::SpreadsheetMenu() {
	kDebug()<<endl;
	Spreadsheet *s = activeSpreadsheet();
	if(s) {
		s->Menu(spreadsheetmenu);
	} else {
		spreadsheetmenu->clear();
		spreadsheetmenu->addAction(m_newSpreadsheetAction);
	}
}

Table* MainWin::newSpreadsheet() {
	Table * table = new Table(0, 100, 2, i18n("Spreadsheet %1").arg(1));

	QModelIndex index = m_project_explorer->currentIndex();

	if(!index.isValid())
		m_project->addChild(table);
	else {
		AbstractAspect * parent_aspect = static_cast<AbstractAspect *>(index.internalPointer());
		Q_ASSERT(parent_aspect->folder()); // every aspect contained in the project should have a folder
		parent_aspect->folder()->addChild(table);
	}

	kDebug()<<"new spreadsheet created"<<endl;
    return table;
}

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
	Spreadsheet* s=0;
	if ( m_current_aspect )
  		s=qobject_cast<Spreadsheet*>(m_current_aspect);

	return s;
}

/*!
	returns a pointer to a Spreadsheet-object, if the currently active/selected Aspect is of type \a Spreadsheet.
	Otherwise returns \a 0.
*/
Table* MainWin::activeTable() const{
	Table* t=0;
	if ( m_current_aspect )
  		t=qobject_cast<Table*>(m_current_aspect);

	return t;
}

Spreadsheet* MainWin::getSpreadsheet(QString name) const{
// TODO: port to use aspects
	QList<QMdiSubWindow *> wlist = m_mdi_area->subWindowList();
	for (int i=0; i<wlist.size(); i++)
		if(wlist.at(i)->windowTitle() == name)
			return (Spreadsheet *)wlist.at(i);
	return 0;
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


//TODO remove
Worksheet* MainWin::getWorksheet(QString name) const {
// TODO: port to use aspects
// 	QList<QMdiSubWindow *> wlist = m_mdi_area->subWindowList();
// 	for (int i=0; i<wlist.size(); i++)
// 		if(wlist.at(i)->windowTitle() == name)
// 			return (Worksheet *)wlist.at(i);
//  	return 0;
}
/************** sheet stuff end *************************/

/******************** dialogs *****************************/
void MainWin::importDialog() { (new ImportDialog(this))->show(); }
void MainWin::projectDialog() { (new ProjectDialog(this))->show(); m_project->setChanged(true); }


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

	if (type!=Plot::PLOT2D){
			KMessageBox::error(this, i18n("Not yet implemented."));
			return;
	}else{
 		activeWorksheet()->createPlot(type);
	}
	this->updateGUI();
}

/*!
	shows the \a FunctionPlotDialog.
	Creates a data set from a function and adds it to the current Worksheet or Spreadsheet.
*/
void MainWin::functionActionTriggered(QAction* action){
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

	FunctionPlotDialog* dlg = new FunctionPlotDialog(this, type);
	AspectTreeModel* model=new AspectTreeModel(m_project, this);
 	model->stripDownToTopLevel();
	model->showTopLevelOnly(true);
	model->setFolderSelectable(false);

	dlg->setModel( model );
	if ( dlg->exec() == QDialog::Accepted ) {
		if (type!=Plot::PLOT2D){
			KMessageBox::error(this, i18n("Not yet implemented."));
			return;
		}

		Set set(Set::SET2D);
		dlg->saveSet(&set);
		//TODO
// 		QModelIndex index=dlg->currentModelIndex();
// 		this->addSet(set, i, type);
	}
}

void MainWin::titleDialog() {
	//TODO redesign: it shouldn't be possible to select this menu, if no worksheet is active.
	kDebug()<<endl;
	Worksheet *w = activeWorksheet();
	if(w == 0) {
		kDebug()<<"ERROR: no worksheet active!"<<endl;
		return;
	}
	if(w->plotCount() == 0) {
		kDebug()<<"ERROR: worksheet has no plot!"<<endl;
		return;
	}
	Plot *plot = w->activePlot();
	if(plot == 0) {
		kDebug()<<"ERROR: no active plot found!"<<endl;
		return;
	}

	TitleDialog* dlg = new TitleDialog(this);
	dlg->setWorksheet(w);
	dlg->exec();
}

void MainWin::axesDialog(){
	//TODO redesign: it shouldn't be possible to select this menu, if no worksheet is active.
	kDebug()<<endl;
	Worksheet *w = activeWorksheet();
	if(w == 0) {
		kDebug()<<"ERROR: no worksheet active!"<<endl;
		return;
	}
	if(w->plotCount() == 0) {
		kDebug()<<"ERROR: worksheet has no plot!"<<endl;
		return;
	}
	Plot *plot = w->activePlot();
	if(plot == 0) {
		kDebug()<<"ERROR: no active plot found!"<<endl;
		return;
	}

	AxesDialog* dlg = new AxesDialog(this);
	dlg->setWorksheet(w);
	dlg->exec();
}


void MainWin::legendDialog() {
	//TODO redesign: it shouldn't be possible to select this menu, if no worksheet is active.
	kDebug()<<endl;
	Worksheet *w = activeWorksheet();
	if(w == 0) {
		kDebug()<<"ERROR: no worksheet active!"<<endl;
		return;
	}
	if(w->plotCount() == 0) {
		kDebug()<<"ERROR: worksheet has no plot!"<<endl;
		return;
	}
	Plot *plot = w->activePlot();
	if(plot == 0) {
		kDebug()<<"ERROR: no active plot found!"<<endl;
		return;
	}

	LegendDialog* dlg = new LegendDialog(this);
	dlg->setWorksheet(w);
	dlg->exec();
}

/*!
	shows the dialog for editing the properties of the current workhseet
*/
void MainWin::worksheetDialog() {
	Worksheet *w = activeWorksheet();
	if(w == 0) {
		kDebug()<<"ERROR: no worksheet active! Should never happen."<<endl;
		return;
	}

// 	WorksheetDialog* dlg = new WorksheetDialog(this);
// 	dlg->setWorksheet(w);
// 	dlg->exec();
}

void MainWin::settingsDialog(){
	//TODO
	(new SettingsDialog(this))->show();
}
/******************** dialogs end *****************************/

void MainWin::addSet(Set set, const int sheet, const Plot::PlotType ptype) {
	kDebug()<<"sheet ="<<sheet<<endl;
// #if 0 // TODO: port this to use aspects
	int nr_sheets = m_mdi_area->subWindowList().size();
	kDebug()<<" Number of sheets :"<<nr_sheets<<endl;
	if(sheet == nr_sheets) {	// new worksheet
		kDebug()<<"Creating new worksheet"<<endl;
		Worksheet *w = this->newWorksheet();
 		w->addSet(set,ptype);
	}
	else if(sheet == nr_sheets+1) {	// new spreadsheet
// 		kDebug()<<"Creating new spreadsheet"<<endl;
// 		Spreadsheet *s = newSpreadsheet();
// 		//TODO pointer or reference
//  		s->addSet(&set);
	}
	else {
		kDebug()<<" Using sheet "<<sheet<<endl;
		QMdiSubWindow *subWindow = m_mdi_area->subWindowList()[sheet];
		if(subWindow != 0) {
			kDebug()<<" Subwindow exists"<<endl;
			/*
			Worksheet *w = (Worksheet *) subWindow->widget();
			if(w == 0) {
				kDebug()<<"ERROR: selected sheet found!"<<endl;
				return;
			}
			if (w->sheetType() == WORKSHEET)
				w->addSet(set,ptype);
			else {
				Spreadsheet *s = (Spreadsheet *) subWindow->widget();
				//TODO pointer or reference
 				s->addSet(&set);
			}
			*/
// 			Worksheet *w = dynamic_cast<Worksheet *>(subWindow->widget());
// // 			kDebug()<<"object Name "<<subWindow->widget()->objectName()<<endl;
// 			if (w!=0){
// 				w->addSet(set,ptype);
// 			}else{
//
// 			}
		}
	}
// #endif
	updateGUI();
}

void MainWin::handleCurrentSubWindowChanged(QMdiSubWindow* win)
{
	PartMdiView *view = qobject_cast<PartMdiView*>(win);
	if (!view) return;
	emit partActivated(view->part());
	m_project_explorer->setCurrentAspect(view->part());
	updateGUI();
}

void MainWin::handleAspectDescriptionChanged(const AbstractAspect *aspect)
{
	if (aspect != static_cast<AbstractAspect *>(m_project)) return;
	setCaption("LabPlot "LVERSION+i18n(" : ")+m_project->fileName());
}

void MainWin::handleAspectAdded(const AbstractAspect *parent, int index)
{
	handleAspectAddedInternal(parent->child(index));
	updateMdiWindowVisibility();
	handleCurrentSubWindowChanged(m_mdi_area->currentSubWindow());
}

void MainWin::handleAspectAddedInternal(AbstractAspect * aspect)
{
	int count = aspect->childCount();
	for (int i=0; i<count; i++)
		handleAspectAddedInternal(aspect->child(i));

	AbstractPart *part = qobject_cast<AbstractPart*>(aspect);
	if (part)
	{
		PartMdiView *win = part->mdiSubWindow();
		Q_ASSERT(win);
		m_mdi_area->addSubWindow(win);
		connect(win, SIGNAL(statusChanged(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)),
				this, SLOT(handleSubWindowStatusChange(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)));
	}
}

void MainWin::handleAspectRemoved(const AbstractAspect *parent, int index)
{
	Q_UNUSED(index);
	m_project_explorer->setCurrentAspect(parent);
}

void MainWin::handleAspectAboutToBeRemoved(const AbstractAspect *parent, int index)
{
	AbstractPart *part = qobject_cast<AbstractPart*>(parent->child(index));
	if (!part) return;
	PartMdiView *win = part->mdiSubWindow();
	Q_ASSERT(win);
	disconnect(win, SIGNAL(statusChanged(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)),
		this, SLOT(handleSubWindowStatusChange(PartMdiView *, PartMdiView::SubWindowStatus, PartMdiView::SubWindowStatus)));
	m_mdi_area->removeSubWindow(win);
	updateGUI();
}

void MainWin::initProjectExplorer()
{
	// project explorer
	m_project_explorer_dock = new QDockWidget(this);
	m_project_explorer_dock->setWindowTitle(tr("Project Explorer"));
	m_project_explorer = new ProjectExplorer(m_project_explorer_dock);
	m_project_explorer->setModel(new AspectTreeModel(m_project, this));
	m_project_explorer_dock->setWidget(m_project_explorer);
	addDockWidget(Qt::BottomDockWidgetArea, m_project_explorer_dock);
	connect(m_project_explorer, SIGNAL(currentAspectChanged(AbstractAspect *)),
		this, SLOT(handleCurrentAspectChanged(AbstractAspect *)));
	m_project_explorer->setCurrentAspect(m_project);
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

void MainWin::handleSubWindowStatusChange(PartMdiView * view, PartMdiView::SubWindowStatus from, PartMdiView::SubWindowStatus to)
{
	kDebug()<<""<<endl;
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

void MainWin::showHistory()
{   // TODO: this is Qt-only code, you may want to add KDE specific stuff here
	if (!m_project->undoStack()) return;
	QDialog dialog;
	QVBoxLayout layout(&dialog);

    QDialogButtonBox button_box;
	button_box.setOrientation(Qt::Horizontal);
    button_box.setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    QObject::connect(&button_box, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&button_box, SIGNAL(rejected()), &dialog, SLOT(reject()));

	int index = m_project->undoStack()->index();
	QUndoView undo_view(m_project->undoStack());

	layout.addWidget(&undo_view);
	layout.addWidget(&button_box);

	dialog.setWindowTitle(tr("Undo/Redo History"));
	if (dialog.exec() == QDialog::Accepted)
		return;

	m_project->undoStack()->setIndex(index);
}

/*!
	this is called on a right click on the root folder in the project explorer
*/
void MainWin::createContextMenu(QMenu * menu) const
{
	menu->addAction(m_newFolderAction);
	menu->addAction(m_newSpreadsheetAction);
	menu->addAction(m_newWorksheetAction);
}
/*!
	this is called on a right click on a non-root folder in the project explorer
*/
void MainWin::createFolderContextMenu(const Folder * folder, QMenu * menu) const
{
	//Folder provides it's own context menu. Add a separator befor adding additional actions.
	menu->addSeparator();
	menu->addAction(m_newFolderAction);
	menu->addAction(m_newSpreadsheetAction);
	menu->addAction(m_newWorksheetAction);
}

void MainWin::undo()
{
	WAIT_CURSOR;
	m_project->undoStack()->undo();
	RESET_CURSOR;
}

void MainWin::redo()
{
	WAIT_CURSOR;
	m_project->undoStack()->redo();
	RESET_CURSOR;
}

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
