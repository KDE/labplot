/***************************************************************************
	File                 : GuiObserver.cpp
	Project              : LabPlot
	Description 		 : GUI observer
--------------------------------------------------------------------
	Copyright            : (C) 2010-2015 Alexander Semke (alexander.semke@web.de)

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
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/core/Project.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/DatapickerImage.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "commonfrontend/ProjectExplorer.h"
#include "kdefrontend/MainWin.h"
#include "kdefrontend/dockwidgets/AxisDock.h"
#include "kdefrontend/dockwidgets/CartesianPlotDock.h"
#include "kdefrontend/dockwidgets/CartesianPlotLegendDock.h"
#include "kdefrontend/dockwidgets/ColumnDock.h"
#include "kdefrontend/dockwidgets/MatrixDock.h"
#include "kdefrontend/dockwidgets/ProjectDock.h"
#include "kdefrontend/dockwidgets/SpreadsheetDock.h"
#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "kdefrontend/dockwidgets/XYEquationCurveDock.h"
#include "kdefrontend/dockwidgets/XYFitCurveDock.h"
#include "kdefrontend/dockwidgets/XYFourierFilterCurveDock.h"
#include "kdefrontend/dockwidgets/XYInterpolationCurveDock.h"
#include "kdefrontend/dockwidgets/CustomPointDock.h"
#include "kdefrontend/dockwidgets/WorksheetDock.h"
#include "kdefrontend/widgets/LabelWidget.h"
#include "kdefrontend/widgets/DatapickerImageWidget.h"
#include "kdefrontend/widgets/DatapickerCurveWidget.h"

#include <kstatusbar.h>

#include <QDockWidget>
#include <QStackedWidget>
#include <QToolBar>

/*!
  \class GuiObserver
  \brief The GUI observer looks for the selection changes in the main window
  and shows/hides the correspondings dock widgets, toolbars etc.
  This class is intended to simplify (or not to overload) the code in MainWin.

  \ingroup kdefrontend
*/

GuiObserver::GuiObserver(MainWin* mainWin) : m_lastCartesianPlot(0){
	connect(mainWin->m_projectExplorer, SIGNAL(selectedAspectsChanged(QList<AbstractAspect*>&)),
					this, SLOT(selectedAspectsChanged(QList<AbstractAspect*>&)) );
	connect(mainWin->m_projectExplorer, SIGNAL(hiddenAspectSelected(const AbstractAspect*)),
					this, SLOT(hiddenAspectSelected(const AbstractAspect*)) );
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
  QString prevClassName, className;

  //check, whether objects of different types where selected
  //don't show any dock widgets in this case.
  foreach(aspect, selectedAspects){
	  className= aspect->metaObject()->className();
	  if ( className != prevClassName && !prevClassName.isEmpty() ){
		if (mainWindow->stackedWidget->currentWidget())
		  mainWindow->stackedWidget->currentWidget()->hide();

		mainWindow->m_propertiesDock->setWindowTitle(i18n("Properties"));
		return;
	  }
	  prevClassName = className;
  }

  if (mainWindow->stackedWidget->currentWidget())
	mainWindow->stackedWidget->currentWidget()->show();

  if (className=="Spreadsheet"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Spreadsheet properties"));

	if (!mainWindow->spreadsheetDock){
	  mainWindow->spreadsheetDock = new SpreadsheetDock(mainWindow->stackedWidget);
	  connect(mainWindow->spreadsheetDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
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
	  connect(mainWindow->columnDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
	  mainWindow->stackedWidget->addWidget(mainWindow->columnDock);
	}

	QList<Column*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<Column *>(aspect);
	}
	mainWindow->columnDock->setColumns(list);

	mainWindow->stackedWidget->setCurrentWidget(mainWindow->columnDock);
  }else if (className=="Matrix"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Matrix properties"));

	if (!mainWindow->matrixDock){
	  mainWindow->matrixDock = new MatrixDock(mainWindow->stackedWidget);
	  connect(mainWindow->matrixDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
	  mainWindow->stackedWidget->addWidget(mainWindow->matrixDock);
	}

	QList<Matrix*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<Matrix*>(aspect);
	}
	mainWindow->matrixDock->setMatrices(list);

	mainWindow->stackedWidget->setCurrentWidget(mainWindow->matrixDock);
  }else if (className=="Worksheet"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Worksheet properties"));

	if (!mainWindow->worksheetDock){
	  mainWindow->worksheetDock = new WorksheetDock(mainWindow->stackedWidget);
	  connect(mainWindow->worksheetDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
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
	  connect(mainWindow->cartesianPlotDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
	  mainWindow->stackedWidget->addWidget(mainWindow->cartesianPlotDock);
	}

	QList<CartesianPlot*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<CartesianPlot *>(aspect);
	}
	mainWindow->cartesianPlotDock->setPlots(list);

	mainWindow->stackedWidget->setCurrentWidget(mainWindow->cartesianPlotDock);
  }else if (className=="CartesianPlotLegend"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Cartesian plot legend properties"));

	if (!mainWindow->cartesianPlotLegendDock){
	  mainWindow->cartesianPlotLegendDock = new CartesianPlotLegendDock(mainWindow->stackedWidget);
	  connect(mainWindow->cartesianPlotLegendDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
	  mainWindow->stackedWidget->addWidget(mainWindow->cartesianPlotLegendDock);
	}

	QList<CartesianPlotLegend*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<CartesianPlotLegend*>(aspect);
	}
	mainWindow->cartesianPlotLegendDock->setLegends(list);

	mainWindow->stackedWidget->setCurrentWidget(mainWindow->cartesianPlotLegendDock);
  }else if (className=="Axis"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Axis properties"));

	if (!mainWindow->axisDock){
	  mainWindow->axisDock = new AxisDock(mainWindow->stackedWidget);
	  connect(mainWindow->axisDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
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

	if (!mainWindow->xyCurveDock){
	  mainWindow->xyCurveDock = new XYCurveDock(mainWindow->stackedWidget);
	  mainWindow->xyCurveDock->setupGeneral();
	  connect(mainWindow->xyCurveDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
	  mainWindow->stackedWidget->addWidget(mainWindow->xyCurveDock);
	}

	QList<XYCurve*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<XYCurve *>(aspect);
	}
	mainWindow->xyCurveDock->setCurves(list);

	mainWindow->stackedWidget->setCurrentWidget(mainWindow->xyCurveDock);
  }else if (className=="XYEquationCurve"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("xy-equation-curve properties"));

	if (!mainWindow->xyEquationCurveDock){
	  mainWindow->xyEquationCurveDock = new XYEquationCurveDock(mainWindow->stackedWidget);
	  mainWindow->xyEquationCurveDock->setupGeneral();
	  connect(mainWindow->xyEquationCurveDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
	  mainWindow->stackedWidget->addWidget(mainWindow->xyEquationCurveDock);
	}

	QList<XYCurve*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<XYCurve *>(aspect);
	}
	mainWindow->xyEquationCurveDock->setCurves(list);

	mainWindow->stackedWidget->setCurrentWidget(mainWindow->xyEquationCurveDock);
  }else if (className=="XYFitCurve"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("xy-fit-curve properties"));

	if (!mainWindow->xyFitCurveDock){
	  mainWindow->xyFitCurveDock = new XYFitCurveDock(mainWindow->stackedWidget);
	  mainWindow->xyFitCurveDock->setupGeneral();
	  connect(mainWindow->xyFitCurveDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
	  mainWindow->stackedWidget->addWidget(mainWindow->xyFitCurveDock);
	}

	QList<XYCurve*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<XYCurve*>(aspect);
	}
	mainWindow->xyFitCurveDock->setCurves(list);

	mainWindow->stackedWidget->setCurrentWidget(mainWindow->xyFitCurveDock);
  }else if (className=="XYFourierFilterCurve"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("xy-fourier_filter-curve properties"));

	if (!mainWindow->xyFourierFilterCurveDock){
	  mainWindow->xyFourierFilterCurveDock = new XYFourierFilterCurveDock(mainWindow->stackedWidget);
	  mainWindow->xyFourierFilterCurveDock->setupGeneral();
	  connect(mainWindow->xyFourierFilterCurveDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
	  mainWindow->stackedWidget->addWidget(mainWindow->xyFourierFilterCurveDock);
	}

	QList<XYCurve*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<XYCurve*>(aspect);
	}
	mainWindow->xyFourierFilterCurveDock->setCurves(list);

	mainWindow->stackedWidget->setCurrentWidget(mainWindow->xyFourierFilterCurveDock);
  }else if (className=="XYInterpolationCurve"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("xy-interpolation-curve properties"));

	if (!mainWindow->xyInterpolationCurveDock){
	  mainWindow->xyInterpolationCurveDock = new XYInterpolationCurveDock(mainWindow->stackedWidget);
	  mainWindow->xyInterpolationCurveDock->setupGeneral();
	  connect(mainWindow->xyInterpolationCurveDock, SIGNAL(info(QString)), mainWindow->statusBar(), SLOT(showMessage(QString)));
	  mainWindow->stackedWidget->addWidget(mainWindow->xyInterpolationCurveDock);
	}

	QList<XYCurve*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<XYCurve*>(aspect);
	}
	mainWindow->xyInterpolationCurveDock->setCurves(list);

	mainWindow->stackedWidget->setCurrentWidget(mainWindow->xyInterpolationCurveDock);
  }else if (className=="TextLabel"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Text label properties"));

	if (!mainWindow->textLabelDock){
	  mainWindow->textLabelDock = new LabelWidget(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->textLabelDock);
	}

	QList<TextLabel*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<TextLabel*>(aspect);
	}
	mainWindow->textLabelDock->setLabels(list);

	mainWindow->stackedWidget->setCurrentWidget(mainWindow->textLabelDock);
  }else if (className=="CustomPoint"){
	mainWindow->m_propertiesDock->setWindowTitle(i18n("Custom point properties"));

	if (!mainWindow->customPointDock){
	  mainWindow->customPointDock = new CustomPointDock(mainWindow->stackedWidget);
	  mainWindow->stackedWidget->addWidget(mainWindow->customPointDock);
	}

	QList<CustomPoint*> list;
	foreach(aspect, selectedAspects){
	  list<<qobject_cast<CustomPoint*>(aspect);
	}
	mainWindow->customPointDock->setPoints(list);

	mainWindow->stackedWidget->setCurrentWidget(mainWindow->customPointDock);
  }else if (className=="DatapickerCurve"){
      mainWindow->m_propertiesDock->setWindowTitle(i18n("DatapickerCurve properties"));

      if (!mainWindow->datapickerCurveDock){
        mainWindow->datapickerCurveDock = new DatapickerCurveWidget(mainWindow->stackedWidget);
        mainWindow->stackedWidget->addWidget(mainWindow->datapickerCurveDock);
      }

      QList<DatapickerCurve*> list;
      foreach(aspect, selectedAspects){
        list<<qobject_cast<DatapickerCurve*>(aspect);
      }
      mainWindow->datapickerCurveDock->setCurves(list);

      mainWindow->stackedWidget->setCurrentWidget(mainWindow->datapickerCurveDock);
  }else if (className=="Datapicker"){
      mainWindow->m_propertiesDock->setWindowTitle(i18n("Datapicker properties"));

      if (!mainWindow->datapickerImageDock){
        mainWindow->datapickerImageDock = new DatapickerImageWidget(mainWindow->stackedWidget);
        mainWindow->stackedWidget->addWidget(mainWindow->datapickerImageDock);
      }

      QList<DatapickerImage*> list;
      foreach(aspect, selectedAspects){
        list<<qobject_cast<Datapicker*>(aspect)->image();
      }
      mainWindow->datapickerImageDock->setImages(list);

      mainWindow->stackedWidget->setCurrentWidget(mainWindow->datapickerImageDock);
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
	if (className == "Axis") {
		if (!mainWindow->axisDock) {
			mainWindow->axisDock = new AxisDock(mainWindow->stackedWidget);
			mainWindow->stackedWidget->addWidget(mainWindow->axisDock);
		}
		mainWindow->axisDock->activateTitleTab();
	} else if (className == "CartesianPlot") {
		if (!mainWindow->cartesianPlotDock) {
			mainWindow->cartesianPlotDock = new CartesianPlotDock(mainWindow->stackedWidget);
			mainWindow->stackedWidget->addWidget(mainWindow->cartesianPlotDock);
		}
		mainWindow->cartesianPlotDock->activateTitleTab();
	} else if (className=="CartesianPlotLegend") {
		if (!mainWindow->cartesianPlotLegendDock){
			mainWindow->cartesianPlotLegendDock = new CartesianPlotLegendDock(mainWindow->stackedWidget);
			mainWindow->stackedWidget->addWidget(mainWindow->cartesianPlotLegendDock);
		}
		mainWindow->cartesianPlotLegendDock->activateTitleTab();
	}
}
