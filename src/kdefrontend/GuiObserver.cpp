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
#include "MainWin.h"

#include <QDockWidget>
#include <QStackedWidget>
#include "dockwidgets/ColumnDock.h"
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
  
	connect(mainWin-> m_aspectTreeModel, SIGNAL(indexSelected(const QModelIndex&)), this, SLOT(indexSelected(const QModelIndex&) ));
	connect(mainWin-> m_aspectTreeModel, SIGNAL(selectedItemsChanged(const QItemSelection&)), 
					this, SLOT(selectedItemsChanged(const QItemSelection&) ) );
	
	mainWindow=mainWin;
}


GuiObserver::~GuiObserver(){
}


void GuiObserver::indexSelected(const QModelIndex &  index){
//   qDebug()<<"GUIObserver selectIndex "<<index;
   	AbstractAspect* aspect = static_cast<AbstractAspect *>(index.internalPointer());
	if (!aspect)
	  return;
	
	if (aspect->inherits("Spreadsheet"))
	  mainWindow->stackedWidget->setCurrentWidget(mainWindow->spreadsheetDock);
	else if (aspect->inherits("Column"))
	  mainWindow->stackedWidget->setCurrentWidget(mainWindow->columnDock);
	else if (aspect->inherits("Worksheet"))
	 mainWindow-> stackedWidget->setCurrentWidget(mainWindow->worksheetDock);
	else
	 mainWindow-> stackedWidget->currentWidget()->hide();
}

/*!
  called on selection changes in the project explorer.
  Determines the type of the currently selected objects (aspects)
  and activates the corresponding dockwidgets, toolbars etc.
*/
 void GuiObserver::selectedItemsChanged(const QItemSelection &selected){
  QModelIndexList items= selected.indexes();
  
  if (items.size()==0){
	if (mainWindow->stackedWidget->currentWidget())
		mainWindow->stackedWidget->currentWidget()->hide();
	
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Properties"));
	return;
  }
	
  AbstractAspect* aspect=0;
  
  //determine the class name of the first aspect
  aspect = static_cast<AbstractAspect *>(items.first().internalPointer());
    if (!aspect){
	  if (mainWindow->stackedWidget->currentWidget())
		mainWindow->stackedWidget->currentWidget()->hide();
	  
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Properties"));
	return;
  }
  
  QString className=aspect->metaObject()->className();
  QModelIndex index;
  
  for (int i=0; i<items.size()/4; ++i){
	index=items.at(i*4);
	aspect = static_cast<AbstractAspect *>(index.internalPointer());
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

	//there are for model indeces in each row ->devide by 4 to obtain the number of selected aspects.
	//TODO find out a solution which is not explicitely dependent on the current number of columns.
	QList<Spreadsheet*> list;
	for (int i=0; i<items.size()/4; ++i){
	  index=items.at(i*4);
	  list<<static_cast<Spreadsheet *>(index.internalPointer());
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
	for (int i=0; i<items.size()/4; ++i){
	  index=items.at(i*4);
	  list<<static_cast<Column *>(index.internalPointer());
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
	for (int i=0; i<items.size()/4; ++i){
	  index=items.at(i*4);
	  list<<static_cast<Worksheet *>(index.internalPointer());
	}
	mainWindow->worksheetDock->setWorksheets(list);
	
	mainWindow->stackedWidget->setCurrentWidget(mainWindow->worksheetDock);
  }else{
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Properties"));
	if (mainWindow->stackedWidget->currentWidget())
	  mainWindow->stackedWidget->currentWidget()->hide();
  }
}
