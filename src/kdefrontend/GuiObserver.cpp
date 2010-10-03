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
//   qDebug()<<className;
  
  foreach(aspect, selectedAspects){
	  if (aspect->metaObject()->className() != className){
		if (mainWindow->stackedWidget->currentWidget())
		  mainWindow->stackedWidget->currentWidget()->hide();

		mainWindow->m_propertiesDock->setWindowTitle(i18n("Properties"));
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
  }else if (className=="LinearAxis"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Axis properties"));
	
	if (!mainWindow->axisDock){
	  mainWindow->axisDock = new AxisDock(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->axisDock);
	}
	mainWindow->stackedWidget->setCurrentWidget(mainWindow->axisDock);
  }else if (className=="LineSymbolCurve"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Line properties"));
	
	if (!mainWindow->lineSymbolCurveDock){
	  mainWindow->lineSymbolCurveDock = new LineSymbolCurveDock(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->lineSymbolCurveDock);
	}
	
	//TODO make onle the columns and their parents (spreadsheets and file data sources) available in the model 
// 	AspectTreeModel* model=new AspectTreeModel(mainWindow->m_aspectTreeModel);
// 	model->setFolderSelectable(false);
	mainWindow->lineSymbolCurveDock->setModel( mainWindow->m_aspectTreeModel );
	
		
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
}
