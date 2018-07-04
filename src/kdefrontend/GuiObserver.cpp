/***************************************************************************
	File                 : GuiObserver.cpp
	Project              : LabPlot
	Description 		 : GUI observer
--------------------------------------------------------------------
	Copyright            : (C) 2010-2015 Alexander Semke (alexander.semke@web.de)
	Copyright            : (C) 2015-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
	Copyright            : (C) 2016 Garvit Khatri (garvitdelhi@gmail.com)

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
#include "backend/datasources/LiveDataSource.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/BarChartPlot.h"
#include "backend/worksheet/TextLabel.h"
#ifdef HAVE_CANTOR_LIBS
#include "backend/cantorWorksheet/CantorWorksheet.h"
#endif
#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#include "backend/datasources/MQTTSubscriptions.h"
#include "backend/datasources/MQTTTopic.h"
#endif
#include "backend/core/Project.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/DatapickerImage.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "commonfrontend/ProjectExplorer.h"
#include "kdefrontend/MainWin.h"
#include "kdefrontend/dockwidgets/AxisDock.h"
#include "kdefrontend/dockwidgets/NoteDock.h"
#include "kdefrontend/dockwidgets/CartesianPlotDock.h"
#include "kdefrontend/dockwidgets/CartesianPlotLegendDock.h"
#include "kdefrontend/dockwidgets/ColumnDock.h"
#include "kdefrontend/dockwidgets/LiveDataDock.h"
#include "kdefrontend/dockwidgets/MatrixDock.h"
#include "kdefrontend/dockwidgets/ProjectDock.h"
#include "kdefrontend/dockwidgets/SpreadsheetDock.h"
#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "kdefrontend/dockwidgets/HistogramDock.h"
#include "kdefrontend/dockwidgets/XYEquationCurveDock.h"
#include "kdefrontend/dockwidgets/XYDataReductionCurveDock.h"
#include "kdefrontend/dockwidgets/XYDifferentiationCurveDock.h"
#include "kdefrontend/dockwidgets/XYIntegrationCurveDock.h"
#include "kdefrontend/dockwidgets/XYInterpolationCurveDock.h"
#include "kdefrontend/dockwidgets/XYFitCurveDock.h"
#include "kdefrontend/dockwidgets/XYFourierFilterCurveDock.h"
#include "kdefrontend/dockwidgets/XYFourierTransformCurveDock.h"
#include "kdefrontend/dockwidgets/XYSmoothCurveDock.h"
#include "kdefrontend/dockwidgets/CustomPointDock.h"
#include "kdefrontend/dockwidgets/WorksheetDock.h"
#ifdef HAVE_CANTOR_LIBS
#include "kdefrontend/dockwidgets/CantorWorksheetDock.h"
#endif
#include "kdefrontend/widgets/LabelWidget.h"
#include "kdefrontend/widgets/DatapickerImageWidget.h"
#include "kdefrontend/widgets/DatapickerCurveWidget.h"

#include <QDockWidget>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>
#include <KI18n/KLocalizedString>

/*!
  \class GuiObserver
  \brief The GUI observer looks for the selection changes in the main window
  and shows/hides the correspondings dock widgets, toolbars etc.
  This class is intended to simplify (or not to overload) the code in MainWin.

  \ingroup kdefrontend
*/

GuiObserver::GuiObserver(MainWin* mainWin) {
	connect(mainWin->m_projectExplorer, SIGNAL(selectedAspectsChanged(QList<AbstractAspect*>&)),
	        this, SLOT(selectedAspectsChanged(QList<AbstractAspect*>&)) );
	connect(mainWin->m_projectExplorer, SIGNAL(hiddenAspectSelected(const AbstractAspect*)),
	        this, SLOT(hiddenAspectSelected(const AbstractAspect*)) );
	m_mainWindow = mainWin;
}


/*!
  called on selection changes in the project explorer.
  Determines the type of the currently selected objects (aspects)
  and activates the corresponding dockwidgets, toolbars etc.
*/
void GuiObserver::selectedAspectsChanged(QList<AbstractAspect*>& selectedAspects) const {
	if (selectedAspects.isEmpty()) {
		if (m_mainWindow->stackedWidget->currentWidget())
			m_mainWindow->stackedWidget->currentWidget()->hide();

		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties"));
		return;
	}

	QString prevClassName, className;

	//check, whether objects of different types where selected
	//don't show any dock widgets in this case.
	for (auto* aspect: selectedAspects) {
		className = aspect->metaObject()->className();
		if (className != prevClassName && !prevClassName.isEmpty()) {
			if (m_mainWindow->stackedWidget->currentWidget())
				m_mainWindow->stackedWidget->currentWidget()->hide();

			m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties"));
			return;
		}
		prevClassName = className;
	}

	if (m_mainWindow->stackedWidget->currentWidget())
		m_mainWindow->stackedWidget->currentWidget()->show();

	if (className == "Spreadsheet") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Spreadsheet"));

		if (!m_mainWindow->spreadsheetDock) {
			m_mainWindow->spreadsheetDock = new SpreadsheetDock(m_mainWindow->stackedWidget);
			connect(m_mainWindow->spreadsheetDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->spreadsheetDock);
		}

		QList<Spreadsheet*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<Spreadsheet *>(aspect);
		m_mainWindow->spreadsheetDock->setSpreadsheets(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->spreadsheetDock);
	} else if (className == "Column") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Column"));

		if (!m_mainWindow->columnDock) {
			m_mainWindow->columnDock = new ColumnDock(m_mainWindow->stackedWidget);
			connect(m_mainWindow->columnDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->columnDock);
		}

		QList<Column*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<Column *>(aspect);
		m_mainWindow->columnDock->setColumns(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->columnDock);
	} else if (className == "Matrix") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Matrix"));

		if (!m_mainWindow->matrixDock) {
			m_mainWindow->matrixDock = new MatrixDock(m_mainWindow->stackedWidget);
			connect(m_mainWindow->matrixDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->matrixDock);
		}

		QList<Matrix*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<Matrix*>(aspect);
		m_mainWindow->matrixDock->setMatrices(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->matrixDock);
	} else if (className == "Worksheet") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Worksheet"));

		if (!m_mainWindow->worksheetDock) {
			m_mainWindow->worksheetDock = new WorksheetDock(m_mainWindow->stackedWidget);
			connect(m_mainWindow->worksheetDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->worksheetDock);
		}

		QList<Worksheet*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<Worksheet *>(aspect);
		m_mainWindow->worksheetDock->setWorksheets(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->worksheetDock);
	} else if (className == "CartesianPlot") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Cartesian Plot"));

		if (!m_mainWindow->cartesianPlotDock) {
			m_mainWindow->cartesianPlotDock = new CartesianPlotDock(m_mainWindow->stackedWidget);
			connect(m_mainWindow->cartesianPlotDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->cartesianPlotDock);
		}

		QList<CartesianPlot*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<CartesianPlot *>(aspect);
		m_mainWindow->cartesianPlotDock->setPlots(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->cartesianPlotDock);
	} else if (className == "CartesianPlotLegend") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Legend"));

		if (!m_mainWindow->cartesianPlotLegendDock) {
			m_mainWindow->cartesianPlotLegendDock = new CartesianPlotLegendDock(m_mainWindow->stackedWidget);
			connect(m_mainWindow->cartesianPlotLegendDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->cartesianPlotLegendDock);
		}

		QList<CartesianPlotLegend*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<CartesianPlotLegend*>(aspect);
		m_mainWindow->cartesianPlotLegendDock->setLegends(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->cartesianPlotLegendDock);
	} else if (className == "Axis") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Axis"));

		if (!m_mainWindow->axisDock) {
			m_mainWindow->axisDock = new AxisDock(m_mainWindow->stackedWidget);
			connect(m_mainWindow->axisDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->axisDock);
		}

		QList<Axis*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<Axis *>(aspect);
		m_mainWindow->axisDock->setAxes(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->axisDock);
	} else if (className == "XYCurve") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "xy-Curve"));

		if (!m_mainWindow->xyCurveDock) {
			m_mainWindow->xyCurveDock = new XYCurveDock(m_mainWindow->stackedWidget);
			m_mainWindow->xyCurveDock->setupGeneral();
			connect(m_mainWindow->xyCurveDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->xyCurveDock);
		}

		QList<XYCurve*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<XYCurve *>(aspect);
		m_mainWindow->xyCurveDock->setCurves(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->xyCurveDock);
	} else if (className == "XYEquationCurve") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "xy-Equation"));

		if (!m_mainWindow->xyEquationCurveDock) {
			m_mainWindow->xyEquationCurveDock = new XYEquationCurveDock(m_mainWindow->stackedWidget);
			m_mainWindow->xyEquationCurveDock->setupGeneral();
			connect(m_mainWindow->xyEquationCurveDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->xyEquationCurveDock);
		}

		QList<XYCurve*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<XYCurve *>(aspect);
		m_mainWindow->xyEquationCurveDock->setCurves(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->xyEquationCurveDock);
	} else if (className == "XYDataReductionCurve") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Data Reduction"));

		if (!m_mainWindow->xyDataReductionCurveDock) {
			m_mainWindow->xyDataReductionCurveDock = new XYDataReductionCurveDock(m_mainWindow->stackedWidget, m_mainWindow->statusBar());
			m_mainWindow->xyDataReductionCurveDock->setupGeneral();
			connect(m_mainWindow->xyDataReductionCurveDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->xyDataReductionCurveDock);
		}

		QList<XYCurve*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<XYCurve*>(aspect);
		m_mainWindow->xyDataReductionCurveDock->setCurves(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->xyDataReductionCurveDock);
	} else if (className == "XYDifferentiationCurve") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Differentiation"));

		if (!m_mainWindow->xyDifferentiationCurveDock) {
			m_mainWindow->xyDifferentiationCurveDock = new XYDifferentiationCurveDock(m_mainWindow->stackedWidget);
			m_mainWindow->xyDifferentiationCurveDock->setupGeneral();
			connect(m_mainWindow->xyDifferentiationCurveDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->xyDifferentiationCurveDock);
		}

		QList<XYCurve*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<XYCurve*>(aspect);
		m_mainWindow->xyDifferentiationCurveDock->setCurves(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->xyDifferentiationCurveDock);
	} else if (className == "XYIntegrationCurve") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Integration"));

		if (!m_mainWindow->xyIntegrationCurveDock) {
			m_mainWindow->xyIntegrationCurveDock = new XYIntegrationCurveDock(m_mainWindow->stackedWidget);
			m_mainWindow->xyIntegrationCurveDock->setupGeneral();
			connect(m_mainWindow->xyIntegrationCurveDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->xyIntegrationCurveDock);
		}

		QList<XYCurve*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<XYCurve*>(aspect);
		m_mainWindow->xyIntegrationCurveDock->setCurves(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->xyIntegrationCurveDock);
	} else if (className == "XYInterpolationCurve") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Interpolation"));

		if (!m_mainWindow->xyInterpolationCurveDock) {
			m_mainWindow->xyInterpolationCurveDock = new XYInterpolationCurveDock(m_mainWindow->stackedWidget);
			m_mainWindow->xyInterpolationCurveDock->setupGeneral();
			connect(m_mainWindow->xyInterpolationCurveDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->xyInterpolationCurveDock);
		}

		QList<XYCurve*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<XYCurve*>(aspect);
		m_mainWindow->xyInterpolationCurveDock->setCurves(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->xyInterpolationCurveDock);
	} else if (className == "XYFitCurve") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Fit"));

		if (!m_mainWindow->xyFitCurveDock) {
			m_mainWindow->xyFitCurveDock = new XYFitCurveDock(m_mainWindow->stackedWidget);
			m_mainWindow->xyFitCurveDock->setupGeneral();
			connect(m_mainWindow->xyFitCurveDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->xyFitCurveDock);
		}

		QList<XYCurve*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<XYCurve*>(aspect);
		m_mainWindow->xyFitCurveDock->setCurves(list);
		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->xyFitCurveDock);
	} else if (className == "XYFourierTransformCurve") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Fourier Transform"));

		if (!m_mainWindow->xyFourierTransformCurveDock) {
			m_mainWindow->xyFourierTransformCurveDock = new XYFourierTransformCurveDock(m_mainWindow->stackedWidget);
			m_mainWindow->xyFourierTransformCurveDock->setupGeneral();
			connect(m_mainWindow->xyFourierTransformCurveDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->xyFourierTransformCurveDock);
		}

		QList<XYCurve*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<XYCurve*>(aspect);

		m_mainWindow->xyFourierTransformCurveDock->setCurves(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->xyFourierTransformCurveDock);
	} else if (className == "XYFourierFilterCurve") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Fourier Filter"));

		if (!m_mainWindow->xyFourierFilterCurveDock) {
			m_mainWindow->xyFourierFilterCurveDock = new XYFourierFilterCurveDock(m_mainWindow->stackedWidget);
			m_mainWindow->xyFourierFilterCurveDock->setupGeneral();
			connect(m_mainWindow->xyFourierFilterCurveDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->xyFourierFilterCurveDock);
		}

		QList<XYCurve*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<XYCurve*>(aspect);
		m_mainWindow->xyFourierFilterCurveDock->setCurves(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->xyFourierFilterCurveDock);
	} else if (className == "XYSmoothCurve") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Smoothing"));

		if (!m_mainWindow->xySmoothCurveDock) {
			m_mainWindow->xySmoothCurveDock = new XYSmoothCurveDock(m_mainWindow->stackedWidget);
			m_mainWindow->xySmoothCurveDock->setupGeneral();
			connect(m_mainWindow->xySmoothCurveDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->xySmoothCurveDock);
		}

		QList<XYCurve*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<XYCurve*>(aspect);
		m_mainWindow->xySmoothCurveDock->setCurves(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->xySmoothCurveDock);
	} else if (className=="Histogram") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Histogram Properties"));

		if (!m_mainWindow->histogramDock) {
			m_mainWindow->histogramDock = new HistogramDock(m_mainWindow->stackedWidget);
			connect(m_mainWindow->histogramDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->histogramDock);
		}

		QList<Histogram*> list;
		for (auto* aspect: selectedAspects)
			list<<qobject_cast<Histogram *>(aspect);
		m_mainWindow->histogramDock->setCurves(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->histogramDock);
	} else if (className == "TextLabel") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Text Label"));

		if (!m_mainWindow->textLabelDock) {
			m_mainWindow->textLabelDock = new LabelWidget(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->textLabelDock);
		}

		QList<TextLabel*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<TextLabel*>(aspect);
		m_mainWindow->textLabelDock->setLabels(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->textLabelDock);
	} else if (className == "CustomPoint") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Custom Point"));

		if (!m_mainWindow->customPointDock) {
			m_mainWindow->customPointDock = new CustomPointDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->customPointDock);
		}

		QList<CustomPoint*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<CustomPoint*>(aspect);
		m_mainWindow->customPointDock->setPoints(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->customPointDock);
	} else if (className == "DatapickerCurve") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Datapicker Curve"));

		if (!m_mainWindow->datapickerCurveDock) {
			m_mainWindow->datapickerCurveDock = new DatapickerCurveWidget(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->datapickerCurveDock);
		}

		QList<DatapickerCurve*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<DatapickerCurve*>(aspect);
		m_mainWindow->datapickerCurveDock->setCurves(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->datapickerCurveDock);
	} else if (className == "Datapicker") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Datapicker"));

		if (!m_mainWindow->datapickerImageDock) {
			m_mainWindow->datapickerImageDock = new DatapickerImageWidget(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->datapickerImageDock);
		}

		QList<DatapickerImage*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<Datapicker*>(aspect)->image();
		m_mainWindow->datapickerImageDock->setImages(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->datapickerImageDock);
	} else if (className == "Project") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Project"));

		if (!m_mainWindow->projectDock) {
			m_mainWindow->projectDock = new ProjectDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->projectDock);
		}

		m_mainWindow->projectDock->setProject(m_mainWindow->m_project);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->projectDock);
	} else if (className=="CantorWorksheet") {
#ifdef HAVE_CANTOR_LIBS
		if (!m_mainWindow->cantorWorksheetDock) {
			m_mainWindow->cantorWorksheetDock = new CantorWorksheetDock(m_mainWindow->stackedWidget);
			connect(m_mainWindow->cantorWorksheetDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->cantorWorksheetDock);
		}

		QList<CantorWorksheet*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<CantorWorksheet *>(aspect);
		if (list.size() == 1)
			m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window %1 is a Cantor backend", "%1 Properties", list.first()->backendName()));
		else
			m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "CAS Properties"));
		m_mainWindow->cantorWorksheetDock->setCantorWorksheets(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->cantorWorksheetDock);
#endif
	} else if (className == "Notes") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Notes"));

		if (!m_mainWindow->notesDock) {
			m_mainWindow->notesDock = new NoteDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->notesDock);
		}

		QList<Note*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<Note*>(aspect);
		m_mainWindow->notesDock->setNotesList(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->notesDock);
	}
#ifdef HAVE_MQTT
	else if (className == "MQTTClient") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18n("MQTT Data Source"));

		if (!m_mainWindow->m_liveDataDock) {
			qDebug()<<"new live data dock";
			m_mainWindow->m_liveDataDock = new LiveDataDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->m_liveDataDock);
		}

		QList<MQTTClient*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<MQTTClient*>(aspect);
		m_mainWindow->m_liveDataDock->setMQTTClients(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->m_liveDataDock);
	} else if (className == "MQTTSubscriptions") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18n("MQTT Data Source"));

		if (!m_mainWindow->m_liveDataDock) {
			qDebug()<<"new live data dock";
			m_mainWindow->m_liveDataDock = new LiveDataDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->m_liveDataDock);
		}

		QList<MQTTClient*> list;
		for (auto* aspect: selectedAspects) {
			QString clientName = qobject_cast<MQTTClient*>(qobject_cast<MQTTSubscriptions*>(aspect)->mqttClient())->name();
			bool found = false;
			for (int i = 0; i < list.size(); ++i) {
				if(list.at(i)->name() == clientName) {
					found = true;
					break;
				}
			}
			if(!found)
				list << qobject_cast<MQTTClient*>(qobject_cast<MQTTSubscriptions*>(aspect)->mqttClient());
		}
		m_mainWindow->m_liveDataDock->setMQTTClients(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->m_liveDataDock);
	} else if (className == "MQTTTopic") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18n("MQTT Data Source"));

		if (!m_mainWindow->m_liveDataDock) {
			m_mainWindow->m_liveDataDock = new LiveDataDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->m_liveDataDock);
		}

		QList<MQTTClient*> list;
		for (auto* aspect: selectedAspects) {
			QString clientName = qobject_cast<MQTTClient*>(qobject_cast<MQTTTopic*>(aspect)->mqttClient())->name();
			bool found = false;
			for (int i = 0; i < list.size(); ++i) {
				if(list.at(i)->name() == clientName) {
					found = true;
					break;
				}
			}
			if(!found)
				list << qobject_cast<MQTTClient*>(qobject_cast<MQTTTopic*>(aspect)->mqttClient());
		}
		m_mainWindow->m_liveDataDock->setMQTTClients(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->m_liveDataDock);
	}
#endif
	else if (className == "LiveDataSource") {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18n("Live data source"));

		if (!m_mainWindow->m_liveDataDock) {
			m_mainWindow->m_liveDataDock = new LiveDataDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->m_liveDataDock);
		}

		QList<LiveDataSource*> list;
		for (auto* aspect: selectedAspects)
			list << qobject_cast<LiveDataSource*>(aspect);
		m_mainWindow->m_liveDataDock->setLiveDataSources(list);

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->m_liveDataDock);
	} else {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties"));
		if (m_mainWindow->stackedWidget->currentWidget())
			m_mainWindow->stackedWidget->currentWidget()->hide();
	}
}

/*!
	handles the selection of a hidden aspect \c aspect in the view (relevant for WorksheetView only at the moment).
	Currently, a hidden aspect can only be a plot title lable or an axis label.
	-> Activate the corresponding DockWidget and make the title tab current.
 */
void GuiObserver::hiddenAspectSelected(const AbstractAspect* aspect) const {
	const AbstractAspect* parent = aspect->parentAspect();
	if (!parent)
		return;

	QString className = parent->metaObject()->className();
	if (className == "Axis") {
		if (!m_mainWindow->axisDock) {
			m_mainWindow->axisDock = new AxisDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->axisDock);
		}
		m_mainWindow->axisDock->activateTitleTab();
	} else if (className == "CartesianPlot") {
		if (!m_mainWindow->cartesianPlotDock) {
			m_mainWindow->cartesianPlotDock = new CartesianPlotDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->cartesianPlotDock);
		}
		m_mainWindow->cartesianPlotDock->activateTitleTab();
	} else if (className == "CartesianPlotLegend") {
		if (!m_mainWindow->cartesianPlotLegendDock) {
			m_mainWindow->cartesianPlotLegendDock = new CartesianPlotLegendDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->cartesianPlotLegendDock);
		}
		m_mainWindow->cartesianPlotLegendDock->activateTitleTab();
	}
}

