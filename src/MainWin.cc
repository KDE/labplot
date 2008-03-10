#include "MainWin.h"
#include "ImportDialog.h"
 
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
#include "ProjectDialog.h"
#include "FunctionDialog.h"
#include "TitleDialog.h"
#include "AxesDialog.h"
#include "LegendDialog.h"
#include "PlotDialog.h"
#include "WorksheetDialog.h"
 
#include "pixmaps/pixmap.h"

MainWin::MainWin(QWidget *parent)
 : KXmlGuiWindow(parent)
{
	mdi = new QMdiArea;
	setCentralWidget(mdi);

	setCaption("LabPlot "LVERSION);
	//setCaption("LabPlot "LVERSION+i18n(" : ")+project->Filename());
	setupActions();

	openNew();

	spreadsheetmenu = static_cast<QMenu*> (guiFactory()->container("spreadsheet",this));
	connect(spreadsheetmenu, SIGNAL(aboutToShow()), this, SLOT(SpreadsheetMenu()) );
}

void MainWin::setupActions() {
	// File
	KAction *action = KStandardAction::openNew(this, SLOT(openNew()),actionCollection());
	action = KStandardAction::open(this, SLOT(open()),actionCollection());
	action->setEnabled(false);
	action = KStandardAction::save(this, SLOT(save()),actionCollection());
	action->setEnabled(false);
	action = KStandardAction::saveAs(this, SLOT(saveAs()),actionCollection());
	action->setEnabled(false);
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
// TODO	:		saveXML();
			break;
		case KMessageBox::No:
			break;
		case KMessageBox::Cancel:
			return 1;
			break;
		}
	}

	project->setChanged(false);
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
	kdDebug()<<"MainWin::New()"<<endl;
//	if(warnModified()) return;

	mdi->closeAllSubWindows();
	updateGUI();
/*	gvpart=0;
	defining_region=0;
	defining_line=0;
	defining_rect=0;
	defining_ellipse=0;
	defining_label=false;
	defining_image=false;
	defining_baseline=false;
	defining_maglens=0;
	defining_panzoom=0;
*/
	project = new Project();
	project->setChanged(false);
}

void MainWin::print() {
	if (Worksheet *w = activeWorksheet()) w->print();
	statusBar()->showMessage(i18n("Printed worksheet"));
}

/************** sheet stuff *************************/
void MainWin::SpreadsheetMenu() {
	kdDebug()<<"MainWin::SpreadsheetMenu()"<<endl;
	Spreadsheet *s = activeSpreadsheet();
	if(s) {
		s->Menu(spreadsheetmenu);
	} else {
		spreadsheetmenu->clear();
		spreadsheetmenu->addAction(spreadaction);
	}
}

int MainWin::activeSheetIndex() {
	QList<QMdiSubWindow *> wlist = mdi->subWindowList();
	for (int i=0; i<wlist.size(); i++)
		if(wlist.at(i) == mdi->activeSubWindow())
			return i;
	return -1;
}

Spreadsheet* MainWin::newSpreadsheet() {
        kdDebug()<<"MainWin::newSpreadsheet()"<<endl;
        Spreadsheet *s = new Spreadsheet(this);
	s->setAttribute(Qt::WA_DeleteOnClose);
	mdi->addSubWindow(s);
	s->show();
	project->setChanged(true);

	updateGUI();

        return s;
}

Worksheet* MainWin::newWorksheet() {
        kdDebug()<<"MainWin::newWorksheet()"<<endl;
        Worksheet *w = new Worksheet(this);
	mdi->addSubWindow(w);
	w->show();
	project->setChanged(true);

	updateGUI();

        return w;
}

Spreadsheet* MainWin::activeSpreadsheet() {
	kdDebug()<<"MainWin::activeSpreadsheet()"<<endl;
	QMdiSubWindow *subWindow = mdi->activeSubWindow();
	if(subWindow != 0) {
		Spreadsheet *s = (Spreadsheet *) subWindow->widget();
		if (s && s->sheetType() == SPREADSHEET)
			return s;
	}
	return 0;
}

Worksheet* MainWin::activeWorksheet() {
	kdDebug()<<"MainWin::activeWorksheet()"<<endl;
	QMdiSubWindow *subWindow = mdi->activeSubWindow();
	if(subWindow != 0) {
		Worksheet *w = (Worksheet *) subWindow->widget();
		if (w && w->sheetType() == WORKSHEET)
			return w;
	}
	return 0;
}
/************** sheet stuff end *************************/

/******************** dialogs *****************************/
void MainWin::importDialog() { (new ImportDialog(this))->show(); }
void MainWin::projectDialog() { (new ProjectDialog(this))->show(); project->setChanged(true); }
void MainWin::functionDialog() { (new FunctionDialog(this))->show(); }
void MainWin::titleDialog() {
	kdDebug()<<"MainWin::titleDialog()"<<endl;
	Worksheet *w = activeWorksheet();
	if(w == 0) {
		kdDebug()<<"ERROR: no worksheet active!"<<endl;
		return;
	}
	if(w->plotCount() == 0) {
		kdDebug()<<"ERROR: worksheet has no plot!"<<endl;
		return;
	}
	Plot *plot = w->getActivePlot();
	if(plot == 0) {
		kdDebug()<<"ERROR: no active plot found!"<<endl;
		return;
	}

	(new TitleDialog(this,plot->Title()))->show();
}
void MainWin::axesDialog() { (new AxesDialog(this))->show(); }
void MainWin::plotDialog() { (new PlotDialog(this))->show(); }
void MainWin::worksheetDialog() { (new WorksheetDialog(this))->show(); }
void MainWin::legendDialog() { (new LegendDialog(this))->show(); }
/******************** dialogs end *****************************/

void MainWin::addSet(Set *set, int sheet, PlotType ptype) {
	kdDebug()<<"MainWin::addGraph2D() sheet ="<<sheet<<endl;

	int nr_sheets = mdi->subWindowList().size();
	kdDebug()<<" Number of sheets :"<<nr_sheets<<endl;
	if(sheet == nr_sheets) {	// new worksheet
		Worksheet *w = newWorksheet();
		w->addSet(set,ptype);
	}
	else if(sheet == nr_sheets+1) {	// new spreadsheet
		Spreadsheet *s = newSpreadsheet();
		s->addSet(set);
	}
	else {
		kdDebug()<<" Using sheet "<<sheet<<endl;
		QMdiSubWindow *subWindow = mdi->subWindowList()[sheet];
		if(subWindow != 0) {
			Worksheet *w = (Worksheet *) subWindow->widget();
			if(w == 0) {
				kdDebug()<<"ERROR: selected sheet found!"<<endl;
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

