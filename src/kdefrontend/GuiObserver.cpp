/***************************************************************************
File                 : GuiObserver.cpp
Project              : LabPlot
Description 	     : GUI observer
--------------------------------------------------------------------
Copyright            : (C) 2010-2015 Alexander Semke (alexander.semke@web.de)
Copyright            : (C) 2015-2018 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/TextLabel.h"
#ifdef HAVE_CANTOR_LIBS
#include "backend/cantorWorksheet/CantorWorksheet.h"
#endif
#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#include "backend/datasources/MQTTSubscription.h"
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
#include "kdefrontend/dockwidgets/CursorDock.h"
#include "kdefrontend/dockwidgets/CartesianPlotDock.h"
#include "kdefrontend/dockwidgets/CartesianPlotLegendDock.h"
#include "kdefrontend/dockwidgets/ColumnDock.h"
#include "kdefrontend/dockwidgets/ImageDock.h"
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
#include "kdefrontend/dockwidgets/XYSmoothCurveDock.h"
#include "kdefrontend/dockwidgets/XYFitCurveDock.h"
#include "kdefrontend/dockwidgets/XYFourierFilterCurveDock.h"
#include "kdefrontend/dockwidgets/XYFourierTransformCurveDock.h"
#include "kdefrontend/dockwidgets/XYConvolutionCurveDock.h"
#include "kdefrontend/dockwidgets/XYCorrelationCurveDock.h"
#include "kdefrontend/dockwidgets/CustomPointDock.h"
#include "kdefrontend/dockwidgets/ReferenceLineDock.h"
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

namespace GuiObserverHelper
{

template<class T>
bool raiseDock(T*& dock, QStackedWidget* parent)
{
	const bool generated = !dock;
	if (generated) {
		dock = new T(parent);
		parent->addWidget(dock);
	}
	parent->setCurrentWidget(dock);

	return generated;
}

template<class T>
void raiseDockConnect(T*& dock, QStatusBar* statusBar, QStackedWidget* parent)
{
	if (raiseDock(dock, parent))
		QObject::connect(dock, &T::info, [=](const QString& text){ statusBar->showMessage(text); });
}

template<class T>
void raiseDockSetupConnect(T*& dock, QStatusBar* statusBar, QStackedWidget* parent)
{
	if (raiseDock(dock, parent)) {
		dock->setupGeneral();
		QObject::connect(dock, &T::info, [=](const QString& text){ statusBar->showMessage(text); });
	}
}

template<class R>
QList<R*> castList(QList<AbstractAspect*>& selectedAspects)
{
	QList<R*> list;
	for (auto* aspect : selectedAspects)
		list << static_cast<R*>(aspect);
	return list;
}

}

using namespace GuiObserverHelper;

GuiObserver::GuiObserver(MainWin* mainWin) {
	connect(mainWin->m_projectExplorer, &ProjectExplorer::selectedAspectsChanged, this, &GuiObserver::selectedAspectsChanged);
	connect(mainWin->m_projectExplorer, &ProjectExplorer::hiddenAspectSelected, this, &GuiObserver::hiddenAspectSelected);
	m_mainWindow = mainWin;
}

/*!
  called on selection changes in the project explorer.
  Determines the type of the currently selected objects (aspects)
  and activates the corresponding dockwidgets, toolbars etc.
*/
void GuiObserver::selectedAspectsChanged(QList<AbstractAspect*>& selectedAspects) const {
	auto clearDock = [&](){
		if (m_mainWindow->stackedWidget->currentWidget())
			m_mainWindow->stackedWidget->currentWidget()->hide();

		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties"));
	};

	if (selectedAspects.isEmpty()) {
		clearDock();
		return;
	}

	// update cursor dock
	AbstractAspect* parent = selectedAspects[0]->parent(AspectType::Worksheet);
	if (selectedAspects[0]->inherits(AspectType::Worksheet)) {
		if (m_mainWindow->cursorWidget) {
			Worksheet* worksheet = static_cast<Worksheet*>(selectedAspects[0]);
			m_mainWindow->cursorWidget->setWorksheet(worksheet);
		}
	} else if (parent) {
		if (m_mainWindow->cursorWidget) {
			Worksheet* worksheet = static_cast<Worksheet*>(parent);
			m_mainWindow->cursorWidget->setWorksheet(worksheet);
		}
	}

	const AspectType type = selectedAspects.front()->type();

	// Check, whether objects of different types were selected.
	// Don't show any dock widgets in this case.
	for (auto* aspect : selectedAspects) {
		if (aspect->type() != type) {
			clearDock();
			return;
		}
	}

	switch (type) {
	case AspectType::Spreadsheet:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Spreadsheet"));
		raiseDockConnect(m_mainWindow->spreadsheetDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->spreadsheetDock->setSpreadsheets(castList<Spreadsheet>(selectedAspects));
		break;
	case AspectType::Column:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Column"));
		raiseDockConnect(m_mainWindow->columnDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->columnDock->setColumns(castList<Column>(selectedAspects));
		break;
	case AspectType::Matrix:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Matrix"));
		raiseDockConnect(m_mainWindow->matrixDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->matrixDock->setMatrices(castList<Matrix>(selectedAspects));
		break;
	case AspectType::Worksheet: {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Worksheet"));
		if (!m_mainWindow->worksheetDock) {
			m_mainWindow->worksheetDock = new WorksheetDock(m_mainWindow->stackedWidget);
			connect(m_mainWindow->worksheetDock, SIGNAL(info(QString)), m_mainWindow->statusBar(), SLOT(showMessage(QString)));
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->worksheetDock);
		}
		raiseDockConnect(m_mainWindow->worksheetDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->worksheetDock->setWorksheets(castList<Worksheet>(selectedAspects));
		break;
	} case AspectType::CartesianPlot: {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Cartesian Plot"));
		raiseDockConnect(m_mainWindow->cartesianPlotDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->cartesianPlotDock->setPlots(castList<CartesianPlot>(selectedAspects));
		break;
	} case AspectType::CartesianPlotLegend:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Legend"));
		raiseDockConnect(m_mainWindow->cartesianPlotLegendDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->cartesianPlotLegendDock->setLegends(castList<CartesianPlotLegend>(selectedAspects));
		break;
	case AspectType::Axis:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Axis"));
		raiseDockConnect(m_mainWindow->axisDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->axisDock->setAxes(castList<Axis>(selectedAspects));
		break;
	case AspectType::XYCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "xy-Curve"));
		raiseDockSetupConnect(m_mainWindow->xyCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->xyCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYEquationCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "xy-Equation"));
		raiseDockSetupConnect(m_mainWindow->xyEquationCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->xyEquationCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYDataReductionCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Data Reduction"));

		if (!m_mainWindow->xyDataReductionCurveDock) {
			m_mainWindow->xyDataReductionCurveDock = new XYDataReductionCurveDock(m_mainWindow->stackedWidget, m_mainWindow->statusBar());
			m_mainWindow->xyDataReductionCurveDock->setupGeneral();
			connect(m_mainWindow->xyDataReductionCurveDock, &XYDataReductionCurveDock::info,
			        [&](const QString& text){ m_mainWindow->statusBar()->showMessage(text); });
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->xyDataReductionCurveDock);
		}

		m_mainWindow->xyDataReductionCurveDock->setCurves(castList<XYCurve>(selectedAspects));

		m_mainWindow->stackedWidget->setCurrentWidget(m_mainWindow->xyDataReductionCurveDock);
		break;
	case AspectType::XYDifferentiationCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Differentiation"));
		raiseDockSetupConnect(m_mainWindow->xyDifferentiationCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->xyDifferentiationCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYIntegrationCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Integration"));
		raiseDockSetupConnect(m_mainWindow->xyIntegrationCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->xyIntegrationCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYInterpolationCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Interpolation"));
		raiseDockSetupConnect(m_mainWindow->xyInterpolationCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->xyInterpolationCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYSmoothCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Smoothing"));
		raiseDockSetupConnect(m_mainWindow->xySmoothCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->xySmoothCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYFitCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Fit"));
		raiseDockSetupConnect(m_mainWindow->xyFitCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->xyFitCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYFourierTransformCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Fourier Transform"));
		raiseDockSetupConnect(m_mainWindow->xyFourierTransformCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->xyFourierTransformCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYFourierFilterCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Fourier Filter"));
		raiseDockSetupConnect(m_mainWindow->xyFourierFilterCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->xyFourierFilterCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYConvolution:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Convolution/Deconvolution"));
		raiseDockSetupConnect(m_mainWindow->xyConvolutionCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->xyConvolutionCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYCorrelationCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Auto-/Cross-Correlation"));
		raiseDockSetupConnect(m_mainWindow->xyCorrelationCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->xyCorrelationCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::Histogram:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Histogram Properties"));
		raiseDockConnect(m_mainWindow->histogramDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_mainWindow->histogramDock->setCurves(castList<Histogram>(selectedAspects));
		break;
	case AspectType::TextLabel:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Text Label"));
		raiseDock(m_mainWindow->textLabelDock, m_mainWindow->stackedWidget);
		m_mainWindow->textLabelDock->setLabels(castList<TextLabel>(selectedAspects));
		break;
	case AspectType::Image:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Image"));
		raiseDock(m_mainWindow->imageDock, m_mainWindow->stackedWidget);
		m_mainWindow->imageDock->setImages(castList<Image>(selectedAspects));
		break;
	case AspectType::CustomPoint:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Custom Point"));
		raiseDock(m_mainWindow->customPointDock, m_mainWindow->stackedWidget);
		m_mainWindow->customPointDock->setPoints(castList<CustomPoint>(selectedAspects));
		break;
	case AspectType::ReferenceLine:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Reference Line"));
		raiseDock(m_mainWindow->referenceLineDock, m_mainWindow->stackedWidget);
		m_mainWindow->referenceLineDock->setReferenceLines(castList<ReferenceLine>(selectedAspects));
		break;
	case AspectType::DatapickerCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Datapicker Curve"));
		raiseDock(m_mainWindow->datapickerCurveDock, m_mainWindow->stackedWidget);
		m_mainWindow->datapickerCurveDock->setCurves(castList<DatapickerCurve>(selectedAspects));
		break;
	case AspectType::Datapicker:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Datapicker"));
		raiseDock(m_mainWindow->datapickerImageDock, m_mainWindow->stackedWidget);

		{
			QList<DatapickerImage*> list;
			for (auto* aspect : selectedAspects)
				list << qobject_cast<Datapicker*>(aspect)->image();
			m_mainWindow->datapickerImageDock->setImages(list);
		}
		break;
	case AspectType::Project:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Project"));
		raiseDock(m_mainWindow->projectDock, m_mainWindow->stackedWidget);
		m_mainWindow->projectDock->setProject(m_mainWindow->m_project);
		break;
	case AspectType::CantorWorksheet:
#ifdef HAVE_CANTOR_LIBS
		raiseDockConnect(m_mainWindow->cantorWorksheetDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);

		{
			QList<CantorWorksheet*> list = castList<CantorWorksheet>(selectedAspects);
			if (list.size() == 1)
				m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window %1 is a Cantor backend", "%1 Properties", list.first()->backendName()));
			else
				m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "CAS Properties"));
			m_mainWindow->cantorWorksheetDock->setCantorWorksheets(list);
		}
#endif
		break;
	case AspectType::Note:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Notes"));
		raiseDock(m_mainWindow->notesDock, m_mainWindow->stackedWidget);
		m_mainWindow->notesDock->setNotesList(castList<Note>(selectedAspects));
		break;
	case AspectType::MQTTClient:
#ifdef HAVE_MQTT
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "MQTT Data Source"));
		raiseDock(m_mainWindow->m_liveDataDock, m_mainWindow->stackedWidget);
		m_mainWindow->m_liveDataDock->setMQTTClient(static_cast<MQTTClient*>(selectedAspects.first()));
#endif
		break;
	case AspectType::MQTTSubscription:
#ifdef HAVE_MQTT
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "MQTT Data Source"));
		raiseDock(m_mainWindow->m_liveDataDock, m_mainWindow->stackedWidget);
		m_mainWindow->m_liveDataDock->setMQTTClient(static_cast<MQTTSubscription*>(selectedAspects.first())->mqttClient());
#endif
		break;
	case AspectType::MQTTTopic:
#ifdef HAVE_MQTT
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "MQTT Data Source"));
		raiseDock(m_mainWindow->m_liveDataDock, m_mainWindow->stackedWidget);
		m_mainWindow->m_liveDataDock->setMQTTClient(static_cast<MQTTTopic*>(selectedAspects.first())->mqttClient());
#endif
		break;
	case AspectType::LiveDataSource:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Live Data Source"));
		raiseDock(m_mainWindow->m_liveDataDock, m_mainWindow->stackedWidget);
		m_mainWindow->m_liveDataDock->setLiveDataSource(static_cast<LiveDataSource*>(selectedAspects.first()));
		break;
	case AspectType::AbstractAspect:
	case AspectType::AbstractColumn:
	case AspectType::AbstractDataSource:
	case AspectType::AbstractFilter:
	case AspectType::AbstractPart:
	case AspectType::AbstractPlot:
	case AspectType::ColumnStringIO:
	case AspectType::DatapickerImage:
	case AspectType::DatapickerPoint:
	case AspectType::Folder:
	case AspectType::PlotArea:
	case AspectType::SimpleFilterColumn:
	case AspectType::Workbook:
	case AspectType::WorksheetElement:
	case AspectType::WorksheetElementContainer:
	case AspectType::WorksheetElementGroup:
	case AspectType::XYAnalysisCurve:
		clearDock();
		return;
	}

	m_mainWindow->stackedWidget->currentWidget()->show();
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

	switch (static_cast<quint64>(parent->type())) {  // cast the enum to turn off warnings about unhandled cases
	case static_cast<quint64>(AspectType::Axis):
		if (!m_mainWindow->axisDock) {
			m_mainWindow->axisDock = new AxisDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->axisDock);
		}
		m_mainWindow->axisDock->activateTitleTab();
		break;
	case static_cast<quint64>(AspectType::CartesianPlot):
		if (!m_mainWindow->cartesianPlotDock) {
			m_mainWindow->cartesianPlotDock = new CartesianPlotDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->cartesianPlotDock);
		}
		m_mainWindow->cartesianPlotDock->activateTitleTab();
		break;
	case static_cast<quint64>(AspectType::CartesianPlotLegend):
		if (!m_mainWindow->cartesianPlotLegendDock) {
			m_mainWindow->cartesianPlotLegendDock = new CartesianPlotLegendDock(m_mainWindow->stackedWidget);
			m_mainWindow->stackedWidget->addWidget(m_mainWindow->cartesianPlotLegendDock);
		}
		m_mainWindow->cartesianPlotLegendDock->activateTitleTab();
		break;
	default:
		break;
	}
}

