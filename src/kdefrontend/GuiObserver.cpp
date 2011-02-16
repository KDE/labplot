/***************************************************************************
File                 : GuiObserver.cpp
Project              : LabPlot/SciDAVis
Description 		: GUI observer
--------------------------------------------------------------------
Copyright            		: (C) 2010 Alexander Semke
Email (use @ for *)  	: alexander.semke*web.de

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

#include "GuiObserver.h"
#include "core/AspectTreeModel.h"
#include "core/AbstractAspect.h"
#include "spreadsheet/Spreadsheet.h"
#include "worksheet/Worksheet.h"
#include "worksheet/LineSymbolCurve.h"
#include "worksheet/Axis.h"
#include "core/Project.h"
#include "core/ProjectExplorer.h"
#include "MainWin.h"

#include <QDockWidget>
#include <QStackedWidget>
#include "dockwidgets/AxisDock.h"
#include "dockwidgets/ColumnDock.h"
#include "dockwidgets/LineSymbolCurveDock.h"
#include "dockwidgets/SpreadsheetDock.h"
#include "dockwidgets/WorksheetDock.h"
#include <QDebug>
/*!
  \class GuiObserver
  \brief The GUI observer looks for the selection changes in the main window and shows/hides the correspondings dock widgets, toolbars etc.
  This class is intended to simplify (or not to overload) the code in MainWin.

  \ingroup kdefrontend
*/

GuiObserver::GuiObserver(MainWin* mainWin){
	connect(mainWin-> m_project_explorer, SIGNAL(selectedAspectsChanged(QList<AbstractAspect*>&)), 
					this, SLOT(selectedAspectsChanged(QList<AbstractAspect*>&) ) );
	
	mainWindow=mainWin;
}


GuiObserver::~GuiObserver(){
}


/*!
  called on selection changes in the project explorer.
  Determines the type of the currently selected objects (aspects)
  and activates the corresponding dockwidgets, toolbars etc.
*/
 void GuiObserver::selectedAspectsChanged(QList<AbstractAspect*>& selectedAspects){
  if (selectedAspects.size()==0){
	if (mainWindow->stackedWidget->currentWidget())
		mainWindow->stackedWidget->currentWidget()->hide();
	
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Properties"));
	return;
  }
	
  AbstractAspect* aspect=0;
  
  //determine the class name of the first aspect
  aspect = static_cast<AbstractAspect *>(selectedAspects.first());
    if (!aspect){
	  if (mainWindow->stackedWidget->currentWidget())
		mainWindow->stackedWidget->currentWidget()->hide();
	  
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Properties"));
	return;
  }
  
  QString className=aspect->metaObject()->className();
//    qDebug()<<className;
  
  //check, whether objects of different  type where selected -> hide the dock in this case.
  foreach(aspect, selectedAspects){
	  if (aspect->metaObject()->className() != className){
		if (mainWindow->stackedWidget->currentWidget())
		  mainWindow->stackedWidget->currentWidget()->hide();

		mainWindow->m_propertiesDock->setWindowTitle(i18n("Properties"));
		this->updateGui("");
		return;
	  }
	}
	
  if (mainWindow->stackedWidget->currentWidget())
	mainWindow->stackedWidget->currentWidget()->show();

  if (className=="Spreadsheet"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Spreadsheet properties"));
	
	if (!mainWindow->spreadsheetDock){
	  mainWindow->spreadsheetDock = new SpreadsheetDock(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->spreadsheetDock);
	}

	QList<Spreadsheet*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<Spreadsheet *>(aspect);
	}
	mainWindow->spreadsheetDock->setSpreadsheets(list);
	
	mainWindow->stackedWidget->setCurrentWidget(mainWindow->spreadsheetDock);
  }else if (className=="Column"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Column properties"));
	
	if (!mainWindow->columnDock){
	  mainWindow->columnDock = new ColumnDock(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->columnDock);
	}
	
	QList<Column*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<Column *>(aspect);
	}
	mainWindow->columnDock->setColumns(list);
	
	mainWindow->stackedWidget->setCurrentWidget(mainWindow->columnDock);
  }else if (className=="Worksheet"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Worksheet properties"));
	
	if (!mainWindow->worksheetDock){
	  mainWindow->worksheetDock = new WorksheetDock(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->worksheetDock);
	}
	
	QList<Worksheet*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<Worksheet *>(aspect);
	}
	mainWindow->worksheetDock->setWorksheets(list);
	
	mainWindow->stackedWidget->setCurrentWidget(mainWindow->worksheetDock);
  }else if (className=="Axis"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Axis properties"));
	
	if (!mainWindow->axisDock){
	  mainWindow->axisDock = new AxisDock(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->axisDock);
	}
	
	QList<Axis*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<Axis *>(aspect);
	}
	mainWindow->axisDock->setAxes(list);
	
	mainWindow->stackedWidget->setCurrentWidget(mainWindow->axisDock);
  }else if (className=="LineSymbolCurve"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Line properties"));
	
	if (!mainWindow->lineSymbolCurveDock){
	  mainWindow->lineSymbolCurveDock = new LineSymbolCurveDock(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->lineSymbolCurveDock);
	}
	
	//TODO who deletes the new model?
	AspectTreeModel* model=new AspectTreeModel(mainWindow->m_project);
 	mainWindow->lineSymbolCurveDock->setModel( model );
	
	QList<LineSymbolCurve*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<LineSymbolCurve *>(aspect);
	}
	mainWindow->lineSymbolCurveDock->setCurves(list);
	
	mainWindow->stackedWidget->setCurrentWidget(mainWindow->lineSymbolCurveDock);
  }else{
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Properties"));
	if (mainWindow->stackedWidget->currentWidget())
	  mainWindow->stackedWidget->currentWidget()->hide();
  }
  
  this->updateGui(className);
}

/*!
  udpates the GUI in MainWin.
  Depending on the currently selected object(s), identified by \c className, activates/diactivates the corresponding toolbars and menus.
*/
void GuiObserver::updateGui(const QString& className ){
/*
  if (className == ""){
	//no object or objects of different kind (e.g. a spreadsheet and a worksheet) were selected.
	
  }else if (className=="Spreadsheet"){
	
  }else if (className == "Worksheet"){
	if (!worksheetActionCreated)
	  this->createWorksheetActions();
	
	//TODO connect the worksheet actions with the first (or with all?) selected worksheet
  }
 */ 
}

/*
void GuiObserver::createWoksheetActions(){
    //Zoom actions
  worksheetZoomInAction = new KAction(KIcon("zoom-in"), i18n("Zoom in"), this);
  worksheetZoomInAction->setShortcut(Qt::CTRL+Qt::Key_Plus);
//   connect(zoomInAction, SIGNAL(triggered()), SLOT(zoomIn()));

  zoomOutAction = new KAction(KIcon("zoom-out"), i18n("Zoom out"), this);
  zoomOutAction->setShortcut(Qt::CTRL+Qt::Key_Minus);
  connect(zoomOutAction, SIGNAL(triggered()), SLOT(zoomOut()));

  zoomOriginAction = new KAction(KIcon("zoom-original"), i18n("Original size"), this);
  zoomOriginAction->setShortcut(Qt::CTRL+Qt::Key_1);
  connect(zoomOriginAction, SIGNAL(triggered()), SLOT(zoomOrigin()));

  zoomFitPageHeightAction = new KAction(KIcon("zoom-fit-height"), i18n("Fit to height"), this);
  connect(zoomFitPageHeightAction, SIGNAL(triggered()), SLOT(zoomFitPageHeight()));

  zoomFitPageWidthAction = new KAction(KIcon("zoom-fit-width"), i18n("Fit to width"), this);
  connect(zoomFitPageWidthAction, SIGNAL(triggered()), SLOT(zoomFitPageWidth()));
  
  zoomFitSelectionAction = new KAction(i18n("Fit to selection"), this);
  connect(zoomFitSelectionAction, SIGNAL(triggered()), SLOT(zoomFitSelection()));

  // Mouse mode actions 
  QActionGroup* mouseModeActionGroup = new QActionGroup(this);
  navigationModeAction = new KAction(KIcon("input-mouse"), i18n("Navigation"), this);
  navigationModeAction->setCheckable(true);
  mouseModeActionGroup->addAction(navigationModeAction);
  connect(navigationModeAction, SIGNAL(triggered()), SLOT(enableNavigationMode()));

  zoomModeAction = new KAction(KIcon("page-zoom"), i18n("Zoom"), this);
  zoomModeAction->setCheckable(true);
  mouseModeActionGroup->addAction(zoomModeAction);
  connect(zoomModeAction, SIGNAL(triggered()), SLOT(enableZoomMode()));

  selectionModeAction = new KAction(KIcon("select-rectangular"), i18n("Selection"), this);
  selectionModeAction->setCheckable(true);
  mouseModeActionGroup->addAction(selectionModeAction);
  connect(selectionModeAction, SIGNAL(triggered()), SLOT(enableSelectionMode()));

  //Layout actions
  QActionGroup* layoutActionGroup = new QActionGroup(this);

  verticalLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Vertical layout"), this);
  verticalLayoutAction->setObjectName("verticalLayoutAction");
  verticalLayoutAction->setCheckable(true);
  layoutActionGroup->addAction(verticalLayoutAction);

  horizontalLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Horizontal layout"), this);
  horizontalLayoutAction->setObjectName("horizontalLayoutAction");
  horizontalLayoutAction->setCheckable(true);
  layoutActionGroup->addAction(horizontalLayoutAction);

  gridLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Grid layout"), this);
  gridLayoutAction->setObjectName("gridLayoutAction");
  gridLayoutAction->setCheckable(true);
  layoutActionGroup->addAction(gridLayoutAction);

  breakLayoutAction = new KAction(KIcon("select-rectangular"), i18n("Break layout"), this);
  breakLayoutAction->setObjectName("breakLayoutAction");
  breakLayoutAction->setEnabled(false);
  layoutActionGroup->addAction(breakLayoutAction);
}
*/