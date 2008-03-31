#include "MainWin.h"
#include "ImportDialog.h"

//****** GUI **************
#include "gui/SettingsDialog.h"
#include "gui/SettingsDialog.h"
#include "gui/AxesDialog.h"

#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KStandardAction>
#include <kxmlguifactory.h>
#include <KMessageBox>
#include <QMdiSubWindow>
#include <KStatusBar>
#include <KLocale>
#include <KDebug>
#include <KFilterDev>
#include "ProjectDialog.h"
#include "FunctionDialog.h"
#include "TitleDialog.h"
#include "LegendDialog.h"
#include "PlotDialog.h"
#include "WorksheetDialog.h"

#include "sheettype.h"
#include "pixmaps/pixmap.h"

MainWin::MainWin(QWidget *parent, QString filename)
 : KXmlGuiWindow(parent)
{
	mdi = new QMdiArea;
	setCentralWidget(mdi);

	setCaption("LabPlot "LVERSION);
	setupActions();

	openNew();

	// commandline filename
	if(!filename.isEmpty() && !QFile::exists(filename)) {
		int ret = KMessageBox::warningContinueCancel( this,
			i18n( "Could not open file \'%1\'!").arg(filename),i18n("Warning"));
		if (ret==KMessageBox::Cancel)
			exit(-1);
	}
	else if (filename.contains(".lml") || filename.contains(".xml")) {
		open(filename);
	}
	
	spreadsheetmenu = static_cast<QMenu*> (guiFactory()->container("spreadsheet",this));
	connect(spreadsheetmenu, SIGNAL(aboutToShow()), SLOT(SpreadsheetMenu()) );
	
	statusBar()->showMessage(i18n("Welcome to LabPlot ")+LVERSION);
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
	spreadaction = new KAction(KIcon(QIcon(spreadsheet_xpm)),i18n("New Spreadsheet"),this);
	spreadaction->setShortcut(Qt::CTRL+Qt::Key_Equal);
	actionCollection()->addAction("new spreadsheet", spreadaction);
	connect(spreadaction, SIGNAL(triggered()),SLOT(newSpreadsheet()));

	action = new KAction(KIcon(QIcon(worksheet_xpm)),i18n("New Worksheet"),this);
	action->setShortcut(Qt::ALT+Qt::Key_X);
	actionCollection()->addAction("new worksheet", action);
	connect(action, SIGNAL(triggered()), SLOT(newWorksheet()));

	action = new KAction(KIcon(QIcon(newFunction_xpm)),i18n("New Function"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_E);
	actionCollection()->addAction("new function", action);
	connect(action, SIGNAL(triggered()), SLOT(functionDialog()));

	// Appearance
	action = new KAction(KIcon(QIcon(title_xpm)),i18n("Title Settings"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_T);
	actionCollection()->addAction("title settings", action);
	connect(action, SIGNAL(triggered()), SLOT(titleDialog()));

	action = new KAction(KIcon(QIcon(axes_xpm)),i18n("Axes Settings"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_B);
	actionCollection()->addAction("axes settings", action);
	connect(action, SIGNAL(triggered()), SLOT(axesDialog()));

	action = new KAction(KIcon(QIcon(legend_xpm)),i18n("Legend Settings"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_L);
	actionCollection()->addAction("legend settings", action);
	connect(action, SIGNAL(triggered()), SLOT(legendDialog()));

	action = new KAction(KIcon(QIcon(set_xpm)),i18n("Plot Settings"),this);
	action->setShortcut(Qt::CTRL+Qt::Key_J);
	actionCollection()->addAction("plot settings", action);
	connect(action, SIGNAL(triggered()), SLOT(plotDialog()));
	//plot_action->setWhatsThis(i18n("This lets you change the settings of the active plot"));

	action = new KAction(KIcon(QIcon(worksheet_xpm)),i18n("Worksheet Settings"),this);
	action->setShortcut(Qt::ALT+Qt::Key_W);
	actionCollection()->addAction("worksheet settings", action);
	connect(action, SIGNAL(triggered()), SLOT(worksheetDialog()));
	//worksheet_action->setWhatsThis(i18n("This lets you change the settings of the active worksheet"));

	// Analysis
	// Drawing
	// Script

	//Windows
	action  = new KAction(i18n("Cl&ose"), this);
	action->setShortcut(tr("Ctrl+F4"));
	action->setStatusTip(i18n("Close the active window"));
	actionCollection()->addAction("close window", action);
	connect(action, SIGNAL(triggered()), mdi, SLOT(closeActiveSubWindow()));

	action = new KAction(i18n("Close &All"), this);
	action->setStatusTip(i18n("Close all the windows"));
	actionCollection()->addAction("close all windows", action);
	connect(action, SIGNAL(triggered()), mdi, SLOT(closeAllSubWindows()));

	action = new KAction(i18n("&Tile"), this);
	action->setStatusTip(i18n("Tile the windows"));
	actionCollection()->addAction("tile windows", action);
	connect(action, SIGNAL(triggered()), mdi, SLOT(tileSubWindows()));

	action = new KAction(i18n("&Cascade"), this);
	action->setStatusTip(i18n("Cascade the windows"));
	actionCollection()->addAction("cascade windows", action);
	connect(action, SIGNAL(triggered()), mdi, SLOT(cascadeSubWindows()));

	action = new KAction(i18n("Ne&xt"), this);
	action->setStatusTip(i18n("Move the focus to the next window"));
	actionCollection()->addAction("next window", action);
	connect(action, SIGNAL(triggered()), mdi, SLOT(activateNextSubWindow()));

	action = new KAction(i18n("Pre&vious"), this);
	action->setStatusTip(i18n("Move the focus to the previous window"));
	actionCollection()->addAction("previous window", action);
	connect(action, SIGNAL(triggered()), mdi, SLOT(activatePreviousSubWindow()));

	//"Standard actions"
	KStandardAction::preferences(this, SLOT(settingsDialog()), actionCollection());
	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

	setupGUI();
}

bool MainWin::warnModified() {
	if(mdi->subWindowList().size() > 0 && project->Changed()) {
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

void MainWin::updateGUI() {
	updateSheetList();
	updateSetList();

	// TODO : disable/enable menu items, etc.
	if(activeWorksheet() == 0) {
		(static_cast<QMenu*> (guiFactory()->container("appearance",this)))->setEnabled(false);
		(static_cast<QMenu*> (guiFactory()->container("viewmenu",this)))->setEnabled(false);
		(static_cast<QMenu*> (guiFactory()->container("drawing",this)))->setEnabled(false);
		if(activeSpreadsheet() == 0)
			(static_cast<QMenu*> (guiFactory()->container("analysis",this)))->setEnabled(false);
		else
			(static_cast<QMenu*> (guiFactory()->container("analysis",this)))->setEnabled(true);
	}
	else {
		(static_cast<QMenu*> (guiFactory()->container("appearance",this)))->setEnabled(true);
		(static_cast<QMenu*> (guiFactory()->container("viewmenu",this)))->setEnabled(true);
		(static_cast<QMenu*> (guiFactory()->container("drawing",this)))->setEnabled(true);
		(static_cast<QMenu*> (guiFactory()->container("analysis",this)))->setEnabled(true);
	}
}

void MainWin::openNew() {
 	kDebug()<<"MainWin::New()"<<endl;
	if(warnModified()) return;

 	mdi->closeAllSubWindows();
 	updateGUI();
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
 	project = new Project();
 	project->setChanged(true);
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
	project->setFilename(filename);
 	project->setChanged(false);
	//recent->addURL(fn);

	setCaption("LabPlot "LVERSION+i18n(" : ")+project->Filename());
}

void MainWin::openXML(QIODevice *file) {
	QDomDocument doc;
	kDebug()<<"	reading ..."<<endl;
	if(doc.setContent(file) != true ) {
		kDebug()<<"ERROR: reading file content"<<endl;
		return;
	}
	file->close();

	QDomElement root = doc.documentElement();
	kDebug()<<"ROOT TAG = "<<root.tagName()<<endl;
	kDebug()<<"ROOT ATTR version = "<<root.attribute("version")<<endl;
	Q_ASSERT(root.tagName() == "LabPlot");

//Unused : project->setFilename(fn);
	project->setLabPlot(root.attribute("version"));

	QDomNode node = root.firstChild();
	while(!node.isNull()) {
		QDomElement e = node.toElement();
//		kDebug()<<"TAG = "<<e.tagName()<<endl;
//		kDebug()<<"TEXT = "<<e.text()<<endl;

		if(e.tagName() == "SpeedMode")
			kDebug()<<"Speedmode ="<<(bool)e.text().toInt()<<endl;
//			speedmode = (bool)e.text().toInt();
		else if(e.tagName() == "Project")
			project->open(e.firstChild());
		else if(e.tagName() == "Worksheet") {
			Worksheet *p = newWorksheet();
			p->open(e.firstChild());
		}
		else if(e.tagName() == "Spreadsheet") {
			Spreadsheet *s = newSpreadsheet();
			s->open(e.firstChild());
		}
		node = node.nextSibling();
	}
}

void MainWin::save(QString filename) {
	if (filename.isEmpty() ) {
		if(project->Filename().isEmpty()) {
			saveAs();	// need a file name
			return;
		}
		else
			filename = project->Filename();
	}

	kDebug()<<filename<<endl;
	if(project->Filename() == filename && project->Changed() == false) {
		kDebug()<<"no changes to be saved"<<endl;
		return;
	}

	QIODevice *xmlfile = KFilterDev::deviceForFile(filename,QString::null,true);
	if (xmlfile==0) xmlfile = new QFile(filename);
	saveXML(xmlfile);
	project->setFilename(filename);

	setCaption("LabPlot "LVERSION+i18n(" : ")+project->Filename());
}

void MainWin::saveXML(QIODevice *file) {
	kDebug()<<endl;
	QDomDocument doc("LabPlot");
	QString header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	QString doctype = "<!DOCTYPE LabPlotXML>\n";
	doc.setContent(header+doctype);

	QDomElement root = doc.createElement( "LabPlot" );
	root.setAttribute("version",QString(LVERSION));
	doc.appendChild( root );

	QDomElement tag = project->save(doc);
    	root.appendChild( tag );
//	tag = doc.createElement( "SpeedMode" );
//	root.appendChild( tag );
//	QDomText t = doc.createTextNode( QString::number(speedmode));
//	tag.appendChild( t );

	QList<QMdiSubWindow *> list = mdi->subWindowList();
	for (int i = 0; i < list.size(); i++ ) {
		QMdiSubWindow *subWindow = list[i];
		kDebug()<<"	Saving sheet "<<i<<endl;
		Worksheet *w = (Worksheet *) subWindow->widget();
		if(w && w->sheetType() == WORKSHEET) {
			kDebug()<<"	Saving worksheet "<<i<<endl;
			QDomElement wtag = w->save(doc);
    			root.appendChild(wtag);
		}
		else {
			Spreadsheet *s = (Spreadsheet *) subWindow->widget();
			if( s==0 || s->sheetType() != SPREADSHEET) {
				kDebug()<<"ERROR: unknown sheet found!"<<endl;
				return;
			}
			kDebug()<<"	Saving spreadsheet "<<i<<endl;
			QDomElement stag = s->save(doc);
    			root.appendChild(stag);
		}
	}

	if(file->open(QIODevice::WriteOnly | QFile::Text)) {
		QTextStream ts(file);
		doc.save(ts,4);
		ts<<endl;
		statusBar()->showMessage(i18n("Project saved"));
		file->close();

		project->setChanged(false);
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

void MainWin::print() {
	if (Worksheet *w = activeWorksheet()) w->print();
	statusBar()->showMessage(i18n("Printed worksheet"));
}

/************** sheet stuff *************************/
void MainWin::SpreadsheetMenu() {
	kDebug()<<"MainWin::SpreadsheetMenu()"<<endl;
	Spreadsheet *s = activeSpreadsheet();
	if(s) {
		s->Menu(spreadsheetmenu);
	} else {
		spreadsheetmenu->clear();
		spreadsheetmenu->addAction(spreadaction);
	}
}

int MainWin::activeSheetIndex() const {
	QList<QMdiSubWindow *> wlist = mdi->subWindowList();
	for (int i=0; i<wlist.size(); i++)
		if(wlist.at(i) == mdi->activeSubWindow())
			return i;
	return -1;
}

Spreadsheet* MainWin::newSpreadsheet() {
	kDebug()<<"MainWin::newSpreadsheet()"<<endl;
        Spreadsheet *s = new Spreadsheet(this);
	s->setAttribute(Qt::WA_DeleteOnClose);
	mdi->addSubWindow(s);
	s->show();
	project->setChanged(true);

	updateGUI();

        return s;
}

Worksheet* MainWin::newWorksheet() {
        kDebug()<<"()"<<endl;
        Worksheet *w = new Worksheet(this);
	mdi->addSubWindow(w);
	w->show();
	project->setChanged(true);

	updateGUI();

        kDebug()<<"MainWin::newWorksheet() DONE"<<endl;
        return w;
}

Spreadsheet* MainWin::activeSpreadsheet() const {
	kDebug()<<"()"<<endl;
	QMdiSubWindow *subWindow = mdi->activeSubWindow();
	if(subWindow != 0) {
		Spreadsheet *s = (Spreadsheet *) subWindow->widget();
		if (s && s->sheetType() == SPREADSHEET)
			return s;
	}
	return 0;
}

Spreadsheet* MainWin::getSpreadsheet(QString name) const {
	QList<QMdiSubWindow *> wlist = mdi->subWindowList();
	for (int i=0; i<wlist.size(); i++)
		if(wlist.at(i)->windowTitle() == name)
			return (Spreadsheet *)wlist.at(i);
	return 0;
}

Worksheet* MainWin::activeWorksheet() const {
	kDebug()<<"()"<<endl;
	QMdiSubWindow *subWindow = mdi->activeSubWindow();
	if(subWindow != 0) {
		Worksheet *w = (Worksheet *) subWindow->widget();
		if (w && w->sheetType() == WORKSHEET)
			return w;
	}
	return 0;
}

Worksheet* MainWin::getWorksheet(QString name) const {
	QList<QMdiSubWindow *> wlist = mdi->subWindowList();
	for (int i=0; i<wlist.size(); i++)
		if(wlist.at(i)->windowTitle() == name)
			return (Worksheet *)wlist.at(i);
	return 0;
}
/************** sheet stuff end *************************/

/******************** dialogs *****************************/
void MainWin::importDialog() { (new ImportDialog(this))->show(); }
void MainWin::projectDialog() { (new ProjectDialog(this))->show(); project->setChanged(true); }
void MainWin::functionDialog() { (new FunctionDialog(this))->show(); }
void MainWin::titleDialog() {
	kDebug()<<"MainWin::titleDialog()"<<endl;
	Worksheet *w = activeWorksheet();
	if(w == 0) {
		kDebug()<<"ERROR: no worksheet active!"<<endl;
		return;
	}
	if(w->plotCount() == 0) {
		kDebug()<<"ERROR: worksheet has no plot!"<<endl;
		return;
	}
	Plot *plot = w->getActivePlot();
	if(plot == 0) {
		kDebug()<<"ERROR: no active plot found!"<<endl;
		return;
	}

	(new TitleDialog(this,plot->Title()))->show();
}
void MainWin::axesDialog() { (new AxesDialog(this))->show(); }
void MainWin::plotDialog() { (new PlotDialog(this))->show(); }
void MainWin::worksheetDialog() { (new WorksheetDialog(this))->show(); }
void MainWin::legendDialog() { (new LegendDialog(this))->show(); }
void MainWin::settingsDialog(){ (new SettingsDialog(this))->show(); }
/******************** dialogs end *****************************/

void MainWin::addSet(Set *set, int sheet, PlotType ptype) {
	kDebug()<<"MainWin::addGraph2D() sheet ="<<sheet<<endl;

	int nr_sheets = mdi->subWindowList().size();
	kDebug()<<" Number of sheets :"<<nr_sheets<<endl;
	if(sheet == nr_sheets) {	// new worksheet
		Worksheet *w = newWorksheet();
		w->addSet(set,ptype);
	}
	else if(sheet == nr_sheets+1) {	// new spreadsheet
		Spreadsheet *s = newSpreadsheet();
		s->addSet(set);
	}
	else {
		kDebug()<<" Using sheet "<<sheet<<endl;
		QMdiSubWindow *subWindow = mdi->subWindowList()[sheet];
		if(subWindow != 0) {
			Worksheet *w = (Worksheet *) subWindow->widget();
			if(w == 0) {
				kDebug()<<"ERROR: selected sheet found!"<<endl;
				return;
			}
			if (w->sheetType() == WORKSHEET)
				w->addSet(set,ptype);
			else {
				Spreadsheet *s = (Spreadsheet *) subWindow->widget();
				s->addSet(set);
			}
		}
	}

	updateGUI();
}
