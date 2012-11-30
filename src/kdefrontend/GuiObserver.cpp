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

#include "kdefrontend/GuiObserver.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/AbstractAspect.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/core/Project.h"
#include "commonfrontend/ProjectExplorer.h"
#include "kdefrontend/MainWin.h"
#include "kdefrontend/dockwidgets/AxisDock.h"
#include "kdefrontend/dockwidgets/CartesianPlotDock.h"
#include "kdefrontend/dockwidgets/ColumnDock.h"
#include "kdefrontend/dockwidgets/ProjectDock.h"
#include "kdefrontend/dockwidgets/SpreadsheetDock.h"
#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "kdefrontend/dockwidgets/WorksheetDock.h"
#include "kdefrontend/widgets/LabelWidget.h"

#include <kxmlguifactory.h>

#include <QDockWidget>
#include <QStackedWidget>
#include <QToolBar>
#include <QDebug>

#include <memory>

/*!
  \class GuiObserver
  \brief The GUI observer looks for the selection changes in the main window and shows/hides the correspondings dock widgets, toolbars etc.
  This class is intended to simplify (or not to overload) the code in MainWin.

  \ingroup kdefrontend
*/

GuiObserver::GuiObserver(MainWin* mainWin){
	connect(mainWin->m_projectExplorer, SIGNAL(selectedAspectsChanged(QList<AbstractAspect*>&)), 
					this, SLOT(selectedAspectsChanged(QList<AbstractAspect*>&) ) );
	connect(mainWin->m_projectExplorer, SIGNAL(hiddenAspectSelected(const AbstractAspect*)), 
					this, SLOT(hiddenAspectSelected(const AbstractAspect*) ) );	
	mainWindow=mainWin;
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
  
  //check, whether objects of different  type where selected -> hide the dock in this case.
  foreach(aspect, selectedAspects){
	  if (aspect->metaObject()->className() != className){
		if (mainWindow->stackedWidget->currentWidget())
		  mainWindow->stackedWidget->currentWidget()->hide();

		mainWindow->m_propertiesDock->setWindowTitle(i18n("Properties"));
		this->updateGui("", 0);
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
  }else if (className=="CartesianPlot"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Cartesian plot properties"));
	
	if (!mainWindow->cartesianPlotDock){
	  mainWindow->cartesianPlotDock = new CartesianPlotDock(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->cartesianPlotDock);
	}
	
	QList<CartesianPlot*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<CartesianPlot *>(aspect);
	}
	mainWindow->cartesianPlotDock->setPlots(list);
	
	mainWindow->stackedWidget->setCurrentWidget(mainWindow->cartesianPlotDock);
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
  }else if (className=="XYCurve"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("xy-curve properties"));

	if (!mainWindow->lineSymbolCurveDock){
	  mainWindow->lineSymbolCurveDock = new XYCurveDock(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->lineSymbolCurveDock);
	}
	
	std::auto_ptr<AspectTreeModel> model( new AspectTreeModel(mainWindow->m_project) );
 	mainWindow->lineSymbolCurveDock->setModel( model );
	
	QList<XYCurve*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<XYCurve *>(aspect);
	}
	mainWindow->lineSymbolCurveDock->setCurves(list);
	
	mainWindow->stackedWidget->setCurrentWidget(mainWindow->lineSymbolCurveDock);
  }else if (className=="TextLabel"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Text label properties"));
	
	if (!mainWindow->axisDock){
	  mainWindow->textLabelDock = new LabelWidget(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->textLabelDock);
	}
	
	QList<TextLabel*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<TextLabel*>(aspect);
	}
	mainWindow->textLabelDock->setLabels(list);
	
	mainWindow->stackedWidget->setCurrentWidget(mainWindow->textLabelDock);
  }else if (className=="Project"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Project properties"));
	
	if (!mainWindow->projectDock){
	  mainWindow->projectDock = new ProjectDock(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->projectDock);
	}
	
	mainWindow->projectDock->setProject(mainWindow->m_project);
	
	mainWindow->stackedWidget->setCurrentWidget(mainWindow->projectDock);
  }else{
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Properties"));
	if (mainWindow->stackedWidget->currentWidget())
	  mainWindow->stackedWidget->currentWidget()->hide();
  }
  
  this->updateGui(className, aspect);
}

/*!
	handles the selection of a hidden aspect \c aspect in the view (relevant for WorksheetView only at the moment).
	Currently, a hidden aspect can only be a plot title lable or an axis label.
	-> Activate the corresponding DockWidget and make the title tab current.
 */
void GuiObserver::hiddenAspectSelected(const AbstractAspect* aspect){
	const AbstractAspect* parent = aspect->parentAspect();
	if (!parent)
		return;
	
	QString className = parent->metaObject()->className();
	if (className == "Axis")
		mainWindow->axisDock->activateTitleTab();
	else if (className == "CartesianPlot")
		mainWindow->cartesianPlotDock->activateTitleTab();
}

/*!
  udpates the GUI in MainWin.
  Depending on the currently selected object(s), identified by \c className, activates/diactivates the corresponding toolbars and menus.
*/
void GuiObserver::updateGui(const QString& className, const AbstractAspect* aspect){
  if (className == ""){
	//no object or objects of different kind (e.g. a spreadsheet and a worksheet) were selected.
	
  }else if (className=="CartesianPlot"){
	//populate worksheet-toolbar
	QToolBar* toolbar=qobject_cast<QToolBar*>(mainWindow->guiFactory()->container("cartesian_plot_toolbar", mainWindow));
	toolbar->show();
	toolbar->setEnabled(true);
	toolbar->clear();
	const CartesianPlot* plot = qobject_cast<const CartesianPlot*>(aspect);
	if (plot)
		plot->fillToolBar(toolbar);
	else
		toolbar->setEnabled(false);	
  }
}
