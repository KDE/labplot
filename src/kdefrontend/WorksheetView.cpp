/***************************************************************************
    File                 : WorksheetView.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : provides a view for a worksheet object

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
 #include "WorksheetView.h"
#include "Worksheet.h"
#include "AxesDialog.h"
#include "FunctionPlotDialog.h"
#include "ImportDialog.h"
#include "LegendDialog.h"
#include "ProjectDialog.h"
#include "SettingsDialog.h"
#include "TitleDialog.h"
#include "plots/Plot.h"
#include <KDebug>
#include <KAction>
#include <KLocale>
#include "pixmaps/pixmap.h" //TODO remove this. Use Qt's resource system instead.

WorksheetView::WorksheetView(QWidget* parent, Worksheet* w)
	:QWidget(parent){

// 	KConfigGroup conf(KSharedConfig::openConfig(),"Worksheet");
// 	// TODO : hardcoded :-(
// 	setMinimumSize(300,200);
// 	int w = conf.readEntry("Width",600);
// 	int h = conf.readEntry("Height",400);
// 	resize(w,h);
	createActions();
	worksheet=w;
	setMinimumSize(300,200);
	resize(600,400);

	QPixmap pm(20, 20);
	QPainter pmp(&pm);
	pmp.fillRect(0, 0, 10, 10, Qt::lightGray);
	pmp.fillRect(10, 10, 10, 10, Qt::lightGray);
	pmp.fillRect(0, 10, 10, 10, Qt::darkGray);
	pmp.fillRect(10, 0, 10, 10, Qt::darkGray);
	pmp.end();
	QPalette pal = palette();
	pal.setBrush(backgroundRole(), QBrush(pm));
	setAutoFillBackground(true);
	setPalette(pal);
}


WorksheetView::~WorksheetView(){

}

/*!
	initializes KActions used in the context menu of WorksheetView and in the menu of MainWin.
*/
void WorksheetView::createActions(){
	titleSettingsAction = new KAction(KIcon("draw-text"),i18n("Title Settings"),this);
	titleSettingsAction->setShortcut(Qt::CTRL+Qt::Key_T);
	titleSettingsAction->setWhatsThis(i18n("Changes the title settings of the active plot"));
	connect(titleSettingsAction, SIGNAL(triggered()), SLOT(titleDialog()));

	axesSettingsAction= new KAction(KIcon(QIcon(axes_xpm)),i18n("Axes Settings"),this);
	axesSettingsAction->setShortcut(Qt::CTRL+Qt::Key_A);
	axesSettingsAction->setWhatsThis(i18n("Changes the axis settings of the active plot"));
	connect(axesSettingsAction, SIGNAL(triggered()), SLOT(axesDialog()));

	legendSettingsAction= new KAction(KIcon("format-list-unordered"),i18n("Legend Settings"),this);
	legendSettingsAction->setShortcut(Qt::CTRL+Qt::Key_L);
	legendSettingsAction->setWhatsThis(i18n("Changes the legend settings of the active plot"));
	connect(legendSettingsAction, SIGNAL(triggered()), SLOT(legendDialog()));

	plotSettingsAction= new KAction(KIcon(QIcon(set_xpm)),i18n("Plot Settings"),this);
	plotSettingsAction->setShortcut(Qt::CTRL+Qt::Key_J);
	plotSettingsAction->setWhatsThis(i18n("Changes the settings of the active plot"));
	connect(plotSettingsAction, SIGNAL(triggered()), SLOT(plotDialog()));

	arrangePlotsAction= new KAction(KIcon("view-list-icons"),i18n("Arrange plots"),this);
// 	arrangePlotsAction->setShortcut(Qt::CTRL+Qt::Key_L);
	arrangePlotsAction->setWhatsThis(i18n("Arranges plots in the active worksheet"));
// 	connect(arrangePlotsAction, SIGNAL(triggered()), SLOT(legendDialog()));

	overlayPlotsAction= new KAction(KIcon("window-duplicate"),i18n("Overlay plots"),this);
// overlayPlotsAction	->setShortcut(Qt::CTRL+Qt::Key_L);
	overlayPlotsAction->setWhatsThis(i18n("Overlays plots in the active worksheet"));
// 	connect(overlayPlotsAction, SIGNAL(triggered()), SLOT(legendDialog()));

	worksheetSettingsAction= new KAction(KIcon(QIcon(worksheet_xpm)),i18n("Worksheet Settings"),this);
	worksheetSettingsAction->setShortcut(Qt::CTRL+Qt::Key_W);
	worksheetSettingsAction->setWhatsThis(i18n("Changes the settings of the active worksheet"));
	connect(worksheetSettingsAction, SIGNAL(triggered()), SLOT(worksheetDialog()));
}



void WorksheetView::createMenu(QMenu* menu) const{
	if (!menu)
		menu = new QMenu();

	menu->addAction(titleSettingsAction);
	menu->addAction(axesSettingsAction);
	menu->addAction(legendSettingsAction);
	menu->addAction(plotSettingsAction);
	menu->addSeparator();
	menu->addAction(arrangePlotsAction);
	menu->addAction(overlayPlotsAction);
	menu->addAction(worksheetSettingsAction);

	if (worksheet->plotCount()==0){
		//no plots available->deactivate "Plot appearance" actions
		plotSettingsAction->setEnabled(false);
		titleSettingsAction->setEnabled(false);
		axesSettingsAction->setEnabled(false);
		legendSettingsAction->setEnabled(false);
	}else{
		//plots available->activate "Plot appearance" menus
		plotSettingsAction->setEnabled(true);
		titleSettingsAction->setEnabled(true);
		axesSettingsAction->setEnabled(true);
		legendSettingsAction->setEnabled(true);
	}

	if (worksheet->plotCount()>1){
		arrangePlotsAction->setEnabled(true);
		overlayPlotsAction->setEnabled(true);
	}else{
		arrangePlotsAction->setEnabled(false);
		overlayPlotsAction->setEnabled(false);
	}
}

void WorksheetView::paintEvent(QPaintEvent *) {
	this->draw();
}

void WorksheetView::draw() {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	int width=this->width();
	int height=this->height();

	Q_ASSERT(worksheet);
	QList<Plot*>* listPlots=worksheet->listPlots();
	for (int i=0; i<listPlots->size(); i++) {
 		listPlots->at(i)->draw(&painter, width, height);
		kDebug()<<"Plot "<<i<<" drawn."<<endl;
	}
}

void WorksheetView::contextMenuEvent(QContextMenuEvent *) {
	QMenu *menu = new QMenu(this);
	this->createMenu(menu);
	menu->exec(QCursor::pos());
}


//SLOTS

void WorksheetView::titleDialog() {
	kDebug()<<endl;
	if(worksheet == 0) {
		kDebug()<<"ERROR: no worksheet active!"<<endl;
		return;
	}

	if(worksheet->plotCount() == 0) {
		kDebug()<<"ERROR: worksheet has no plot!"<<endl;
		return;
	}

	Plot *plot = worksheet->activePlot();
	if(plot == 0) {
		kDebug()<<"ERROR: no active plot found!"<<endl;
		return;
	}

	TitleDialog* dlg = new TitleDialog(this);
	dlg->setWorksheet(worksheet);
	dlg->exec();
}

void WorksheetView::axesDialog(){
	kDebug()<<endl;
	if(worksheet == 0) {
		kDebug()<<"ERROR: no worksheet active!"<<endl;
		return;
	}

	if(worksheet->plotCount() == 0) {
		kDebug()<<"ERROR: worksheet has no plot!"<<endl;
		return;
	}

	Plot *plot = worksheet->activePlot();
	if(plot == 0) {
		kDebug()<<"ERROR: no active plot found!"<<endl;
		return;
	}

	AxesDialog* dlg = new AxesDialog(this);
	dlg->setWorksheet(worksheet);
	dlg->exec();
}


void WorksheetView::legendDialog() {
	kDebug()<<endl;
	if(worksheet == 0) {
		kDebug()<<"ERROR: no worksheet active!"<<endl;
		return;
	}

	if(worksheet->plotCount() == 0) {
		kDebug()<<"ERROR: worksheet has no plot!"<<endl;
		return;
	}

	Plot *plot = worksheet->activePlot();
	if(plot == 0) {
		kDebug()<<"ERROR: no active plot found!"<<endl;
		return;
	}

	LegendDialog* dlg = new LegendDialog(this);
	dlg->setWorksheet(worksheet);
	dlg->exec();
}

void WorksheetView::plotDialog(){

}

void WorksheetView::arrangeDialog(){

}

void WorksheetView::overlayDialog(){

}


/*!
	shows the dialog for editing the properties of the current workhseet
*/
void WorksheetView::worksheetDialog(){
	if(worksheet == 0) {
		kDebug()<<"ERROR: no worksheet active! Should never happen."<<endl;
		return;
	}

// 	WorksheetDialog* dlg = new WorksheetDialog(this);
// 	dlg->setWorksheet(w);
// 	dlg->exec();
}
