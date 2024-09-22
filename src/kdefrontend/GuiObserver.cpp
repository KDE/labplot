/*
	File                 : GuiObserver.cpp
	Project              : LabPlot
	Description 	     : GUI observer
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2015-2018 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2016 Garvit Khatri <garvitdelhi@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdefrontend/GuiObserver.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/spreadsheet/StatisticsSpreadsheet.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/KDEPlot.h"
#include "backend/worksheet/plots/cartesian/QQPlot.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
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
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/datapicker/DatapickerImage.h"
#include "commonfrontend/ProjectExplorer.h"
#include "kdefrontend/MainWin.h"
#include "kdefrontend/dockwidgets/AspectDock.h"
#include "kdefrontend/dockwidgets/AxisDock.h"
#include "kdefrontend/dockwidgets/BarPlotDock.h"
#include "kdefrontend/dockwidgets/BoxPlotDock.h"
#include "kdefrontend/dockwidgets/CartesianPlotDock.h"
#include "kdefrontend/dockwidgets/CartesianPlotLegendDock.h"
#include "kdefrontend/dockwidgets/ColumnDock.h"
#include "kdefrontend/dockwidgets/CursorDock.h"
#include "kdefrontend/dockwidgets/CustomPointDock.h"
#include "kdefrontend/dockwidgets/HistogramDock.h"
#include "kdefrontend/dockwidgets/ImageDock.h"
#include "kdefrontend/dockwidgets/InfoElementDock.h"
#include "kdefrontend/dockwidgets/KDEPlotDock.h"
#include "kdefrontend/dockwidgets/LiveDataDock.h"
#include "kdefrontend/dockwidgets/LollipopPlotDock.h"
#include "kdefrontend/dockwidgets/MatrixDock.h"
#include "kdefrontend/dockwidgets/NoteDock.h"
#include "kdefrontend/dockwidgets/ProcessBehaviorChartDock.h"
#include "kdefrontend/dockwidgets/ProjectDock.h"
#include "kdefrontend/dockwidgets/QQPlotDock.h"
#include "kdefrontend/dockwidgets/ReferenceLineDock.h"
#include "kdefrontend/dockwidgets/ReferenceRangeDock.h"
#include "kdefrontend/dockwidgets/SpreadsheetDock.h"
#include "kdefrontend/dockwidgets/StatisticsSpreadsheetDock.h"
#include "kdefrontend/dockwidgets/WorksheetDock.h"
#include "kdefrontend/dockwidgets/XYConvolutionCurveDock.h"
#include "kdefrontend/dockwidgets/XYCorrelationCurveDock.h"
#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "kdefrontend/dockwidgets/XYDataReductionCurveDock.h"
#include "kdefrontend/dockwidgets/XYDifferentiationCurveDock.h"
#include "kdefrontend/dockwidgets/XYEquationCurveDock.h"
#include "kdefrontend/dockwidgets/XYFitCurveDock.h"
#include "kdefrontend/dockwidgets/XYFourierFilterCurveDock.h"
#include "kdefrontend/dockwidgets/XYFourierTransformCurveDock.h"
#include "kdefrontend/dockwidgets/XYFunctionCurveDock.h"
#include "kdefrontend/dockwidgets/XYHilbertTransformCurveDock.h"
#include "kdefrontend/dockwidgets/XYIntegrationCurveDock.h"
#include "kdefrontend/dockwidgets/XYInterpolationCurveDock.h"
#include "kdefrontend/dockwidgets/XYSmoothCurveDock.h"
#ifdef HAVE_CANTOR_LIBS
#include "kdefrontend/dockwidgets/CantorWorksheetDock.h"
#endif
#include "kdefrontend/widgets/DatapickerCurveWidget.h"
#include "kdefrontend/widgets/DatapickerImageWidget.h"
#include "kdefrontend/widgets/LabelWidget.h"

#include <DockWidget.h>
#include <KLocalizedString>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>

/*!
  \class GuiObserver
  \brief The GUI observer looks for the selection changes in the main window
  and shows/hides the correspondings dock widgets, toolbars etc.
  This class is intended to simplify (or not to overload) the code in MainWin.

  \ingroup kdefrontend
*/

QVector<BaseDock*> initializedDocks;

namespace GuiObserverHelper {

template<class T>
bool raiseDock(T*& dock, QStackedWidget* parent) {
	Q_ASSERT(parent);
	DEBUG(Q_FUNC_INFO << ", number of stacked widgets = " << parent->count())

	const bool generated = !dock;
	if (generated) {
		dock = new T(parent);
		initializedDocks << dock;
		parent->addWidget(dock);
	}

	// see https://wiki.qt.io/Technical_FAQ#How_can_I_get_a_QStackedWidget_to_automatically_switch_size_depending_on_the_content_of_the_page
	if (parent->currentWidget())
		parent->currentWidget()->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	parent->setCurrentWidget(dock);
	parent->currentWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// scroll the scroll area up to the top
	if (parent->parent() && parent->parent()->parent()) {
		auto* scrollArea = dynamic_cast<QScrollArea*>(parent->parent()->parent());
		if (scrollArea)
			scrollArea->ensureVisible(0, 0);
	}
	return generated;
}

template<class T>
void raiseDockConnect(T*& dock, QStatusBar* statusBar, QStackedWidget* parent) {
	if (raiseDock(dock, parent))
		QObject::connect(dock, &T::info, [=](const QString& text) {
			statusBar->showMessage(text);
		});
}

template<class T>
void raiseDockSetupConnect(T*& dock, QStatusBar* statusBar, QStackedWidget* parent) {
	if (raiseDock(dock, parent)) {
		dock->setupGeneral();
		QObject::connect(dock, &T::info, [=](const QString& text) {
			statusBar->showMessage(text);
		});
	}
}

template<class T>
QList<T*> castList(const QList<AbstractAspect*>& selectedAspects) {
	QList<T*> list;
	for (auto* aspect : selectedAspects)
		list << static_cast<T*>(aspect);
	return list;
}

} // namespace GuiObserverHelper

using namespace GuiObserverHelper;

GuiObserver::GuiObserver(MainWin* mainWin)
	: m_mainWindow(mainWin) {
	connect(mainWin->m_projectExplorer, &ProjectExplorer::selectedAspectsChanged, this, &GuiObserver::selectedAspectsChanged);
	connect(mainWin->m_projectExplorer, &ProjectExplorer::hiddenAspectSelected, this, &GuiObserver::hiddenAspectSelected);
}

GuiObserver::~GuiObserver() {
	while (!initializedDocks.isEmpty())
		delete initializedDocks.takeLast();
}

/*!
  called on selection changes in the project explorer.
  Determines the type of the currently selected objects (aspects)
  and activates the corresponding dockwidgets, toolbars etc.
*/
void GuiObserver::selectedAspectsChanged(const QList<AbstractAspect*>& selectedAspects) {
	auto clearDock = [&]() {
		if (m_mainWindow->stackedWidget->currentWidget())
			m_mainWindow->stackedWidget->currentWidget()->hide();

		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties"));
	};

	if (selectedAspects.isEmpty() || selectedAspects.front() == nullptr) {
		clearDock();
		return;
	}

	const AspectType type{selectedAspects.front()->type()};
	DEBUG(Q_FUNC_INFO << ", type: " << STDSTRING(AbstractAspect::typeName(type)))

	// update cursor dock
	if (m_mainWindow->cursorWidget) {
		if (type == AspectType::Worksheet) {
			auto* worksheet = static_cast<Worksheet*>(selectedAspects.front());
			m_mainWindow->cursorWidget->setWorksheet(worksheet);
		} else {
			auto* parent = selectedAspects.front()->parent(AspectType::Worksheet);
			if (parent) {
				auto* worksheet = static_cast<Worksheet*>(parent);
				m_mainWindow->cursorWidget->setWorksheet(worksheet);
			}
		}
	}

	// Check, whether objects of different types were selected.
	// Don't show any dock widgets in this case.
	for (const auto* aspect : selectedAspects) {
		if (aspect->type() != type) {
			clearDock();
			return;
		}
	}

	switch (type) {
	case AspectType::Spreadsheet:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Spreadsheet"));
		raiseDockConnect(m_spreadsheetDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_spreadsheetDock->setSpreadsheets(castList<Spreadsheet>(selectedAspects));
		break;
	case AspectType::StatisticsSpreadsheet:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Column Statistics"));
		raiseDockConnect(m_statisticsSpreadsheetDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_statisticsSpreadsheetDock->setSpreadsheets(castList<StatisticsSpreadsheet>(selectedAspects));
		break;
	case AspectType::Column: {
#ifdef HAVE_CANTOR_LIBS
		auto* casParent = dynamic_cast<CantorWorksheet*>(selectedAspects.first()->parentAspect());
		if (casParent) {
			// a column from a CAS-worksheets was selected, show the dock widget for the CAS worksheet
			raiseDockConnect(m_cantorWorksheetDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
			m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window %1 is a Cantor backend", "%1 Worksheet", casParent->backendName()));
			m_cantorWorksheetDock->setCantorWorksheets(QList<CantorWorksheet*>{casParent});
		} else
#endif
		{
			m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Column"));
			raiseDockConnect(m_columnDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
			m_columnDock->setColumns(castList<Column>(selectedAspects));
		}
		break;
	}
	case AspectType::Matrix:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Matrix"));
		raiseDockConnect(m_matrixDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_matrixDock->setMatrices(castList<Matrix>(selectedAspects));
		break;
	case AspectType::Worksheet: {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Worksheet"));
		raiseDockConnect(m_worksheetDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_worksheetDock->setWorksheets(castList<Worksheet>(selectedAspects));
		break;
	}
	case AspectType::CartesianPlot: {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Plot Area"));
		raiseDockConnect(m_cartesianPlotDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_cartesianPlotDock->setPlots(castList<CartesianPlot>(selectedAspects));
		break;
	}
	case AspectType::CartesianPlotLegend:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Legend"));
		raiseDockConnect(m_cartesianPlotLegendDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_cartesianPlotLegendDock->setLegends(castList<CartesianPlotLegend>(selectedAspects));
		break;
	case AspectType::Axis:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Axis"));
		raiseDockConnect(m_axisDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_axisDock->setAxes(castList<Axis>(selectedAspects));
		break;
	case AspectType::XYCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: XY-Curve"));
		raiseDockSetupConnect(m_xyCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYEquationCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: XY-Equation"));
		raiseDockSetupConnect(m_xyEquationCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyEquationCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYFunctionCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Function Curve"));
		raiseDockSetupConnect(m_xyFunctionCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyFunctionCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYDataReductionCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Data Reduction"));
		if (!m_xyDataReductionCurveDock) {
			m_xyDataReductionCurveDock = new XYDataReductionCurveDock(m_mainWindow->stackedWidget, m_mainWindow->statusBar());
			m_xyDataReductionCurveDock->setupGeneral();
			connect(m_xyDataReductionCurveDock, &XYDataReductionCurveDock::info, [&](const QString& text) {
				m_mainWindow->statusBar()->showMessage(text);
			});
			m_mainWindow->stackedWidget->addWidget(m_xyDataReductionCurveDock);
		}

		initializedDocks << m_xyDataReductionCurveDock;
		m_xyDataReductionCurveDock->setCurves(castList<XYCurve>(selectedAspects));

		m_mainWindow->stackedWidget->setCurrentWidget(m_xyDataReductionCurveDock);
		break;
	case AspectType::XYDifferentiationCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Differentiation"));
		raiseDockSetupConnect(m_xyDifferentiationCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyDifferentiationCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYIntegrationCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Integration"));
		raiseDockSetupConnect(m_xyIntegrationCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyIntegrationCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYInterpolationCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Interpolation"));
		raiseDockSetupConnect(m_xyInterpolationCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyInterpolationCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYSmoothCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Smoothing"));
		raiseDockSetupConnect(m_xySmoothCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xySmoothCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYFitCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Fit"));
		raiseDockSetupConnect(m_xyFitCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyFitCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYFourierTransformCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Fourier Transform"));
		raiseDockSetupConnect(m_xyFourierTransformCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyFourierTransformCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYHilbertTransformCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Hilbert Transform"));
		raiseDockSetupConnect(m_xyHilbertTransformCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyHilbertTransformCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYFourierFilterCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Fourier Filter"));
		raiseDockSetupConnect(m_xyFourierFilterCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyFourierFilterCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYConvolutionCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Convolution/Deconvolution"));
		raiseDockSetupConnect(m_xyConvolutionCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyConvolutionCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::XYCorrelationCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Auto-/Cross-Correlation"));
		raiseDockSetupConnect(m_xyCorrelationCurveDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_xyCorrelationCurveDock->setCurves(castList<XYCurve>(selectedAspects));
		break;
	case AspectType::Histogram:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Histogram"));
		raiseDockConnect(m_histogramDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		m_histogramDock->setCurves(castList<Histogram>(selectedAspects));
		break;
	// bar plots
	case AspectType::BarPlot:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Bar Plot"));
		raiseDock(m_barPlotDock, m_mainWindow->stackedWidget);
		m_barPlotDock->setBarPlots(castList<BarPlot>(selectedAspects));
		break;
	case AspectType::LollipopPlot:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Lollipop Plot"));
		raiseDock(m_lollipopPlotDock, m_mainWindow->stackedWidget);
		m_lollipopPlotDock->setPlots(castList<LollipopPlot>(selectedAspects));
		break;
	// statistical plots
	case AspectType::BoxPlot:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Box Plot"));
		raiseDock(m_boxPlotDock, m_mainWindow->stackedWidget);
		m_boxPlotDock->setBoxPlots(castList<BoxPlot>(selectedAspects));
		break;
	case AspectType::KDEPlot:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "KDE Plot"));
		raiseDock(m_kdePlotDock, m_mainWindow->stackedWidget);
		m_kdePlotDock->setPlots(castList<KDEPlot>(selectedAspects));
		break;
	case AspectType::QQPlot:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Q-Q Plot"));
		raiseDock(m_qqPlotDock, m_mainWindow->stackedWidget);
		m_qqPlotDock->setPlots(castList<QQPlot>(selectedAspects));
		break;
	// continious improvement plots
	case AspectType::ProcessBehaviorChart:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Process Behavior Chart"));
		raiseDock(m_processBehaviorChartDock, m_mainWindow->stackedWidget);
		m_processBehaviorChartDock->setPlots(castList<ProcessBehaviorChart>(selectedAspects));
		break;
	case AspectType::TextLabel:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Text Label"));
		raiseDock(m_textLabelDock, m_mainWindow->stackedWidget);
		m_textLabelDock->setLabels(castList<TextLabel>(selectedAspects));
		break;
	case AspectType::Image:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Image"));
		raiseDock(m_imageDock, m_mainWindow->stackedWidget);
		m_imageDock->setImages(castList<Image>(selectedAspects));
		break;
	case AspectType::CustomPoint:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Custom Point"));
		raiseDock(m_customPointDock, m_mainWindow->stackedWidget);
		m_customPointDock->setPoints(castList<CustomPoint>(selectedAspects));
		break;
	case AspectType::ReferenceLine:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Reference Line"));
		raiseDock(m_referenceLineDock, m_mainWindow->stackedWidget);
		m_referenceLineDock->setReferenceLines(castList<ReferenceLine>(selectedAspects));
		break;
	case AspectType::ReferenceRange:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Reference Range"));
		raiseDock(m_referenceRangeDock, m_mainWindow->stackedWidget);
		m_referenceRangeDock->setReferenceRanges(castList<ReferenceRange>(selectedAspects));
		break;
	case AspectType::DatapickerCurve:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Datapicker Curve"));
		raiseDock(m_datapickerCurveDock, m_mainWindow->stackedWidget);
		m_datapickerCurveDock->setCurves(castList<DatapickerCurve>(selectedAspects));
		break;
	case AspectType::Datapicker:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Data Extractor"));
		raiseDock(m_datapickerImageDock, m_mainWindow->stackedWidget);
		{
			QList<DatapickerImage*> list;
			for (auto* aspect : selectedAspects)
				list << static_cast<Datapicker*>(aspect)->image();
			m_datapickerImageDock->setImages(list);
		}
		break;
	case AspectType::Project:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Project"));
		raiseDock(m_projectDock, m_mainWindow->stackedWidget);
		m_projectDock->setProject(m_mainWindow->m_project);
		break;
	case AspectType::CantorWorksheet:
#ifdef HAVE_CANTOR_LIBS
		raiseDockConnect(m_cantorWorksheetDock, m_mainWindow->statusBar(), m_mainWindow->stackedWidget);
		{
			auto list = castList<CantorWorksheet>(selectedAspects);
			if (list.size() == 1)
				m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window %1 is a Cantor backend", "%1 Notebook", list.first()->backendName()));
			else
				m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Notebook"));
			m_cantorWorksheetDock->setCantorWorksheets(list);
		}
#endif
		break;
	case AspectType::Note:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Notes"));
		raiseDock(m_notesDock, m_mainWindow->stackedWidget);
		m_notesDock->setNotesList(castList<Note>(selectedAspects));
		break;
	case AspectType::InfoElement: {
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Info Element"));
		raiseDock(m_infoElementDock, m_mainWindow->stackedWidget);
		m_infoElementDock->setInfoElements(castList<InfoElement>(selectedAspects));
		break;
	}
	case AspectType::MQTTClient:
#ifdef HAVE_MQTT
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: MQTT Data Source"));
		raiseDock(m_liveDataDock, m_mainWindow->stackedWidget);
		m_liveDataDock->setMQTTClient(static_cast<MQTTClient*>(selectedAspects.first()));
#endif
		break;
	case AspectType::MQTTSubscription:
#ifdef HAVE_MQTT
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: MQTT Data Source"));
		raiseDock(m_liveDataDock, m_mainWindow->stackedWidget);
		m_liveDataDock->setMQTTClient(static_cast<MQTTSubscription*>(selectedAspects.first())->mqttClient());
#endif
		break;
	case AspectType::MQTTTopic:
#ifdef HAVE_MQTT
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: MQTT Data Source"));
		raiseDock(m_liveDataDock, m_mainWindow->stackedWidget);
		m_liveDataDock->setMQTTClient(static_cast<MQTTTopic*>(selectedAspects.first())->mqttClient());
#endif
		break;
	case AspectType::LiveDataSource:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Live Data Source"));
		raiseDock(m_liveDataDock, m_mainWindow->stackedWidget);
		m_liveDataDock->setLiveDataSource(static_cast<LiveDataSource*>(selectedAspects.first()));
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
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Folder"));
		raiseDock(m_aspectDock, m_mainWindow->stackedWidget);
		m_aspectDock->setAspects(selectedAspects);
		break;
	case AspectType::PlotArea:
	case AspectType::SimpleFilterColumn:
	case AspectType::Workbook:
		m_mainWindow->m_propertiesDock->setWindowTitle(i18nc("@title:window", "Properties: Workbook"));
		raiseDock(m_aspectDock, m_mainWindow->stackedWidget);
		m_aspectDock->setAspects(selectedAspects);
		break;
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
void GuiObserver::hiddenAspectSelected(const AbstractAspect* aspect) {
	const AbstractAspect* parent = aspect->parentAspect();
	if (!parent)
		return;

	switch (static_cast<quint64>(parent->type())) { // cast the enum to turn off warnings about unhandled cases
	case static_cast<quint64>(AspectType::Axis):
		if (!m_axisDock) {
			m_axisDock = new AxisDock(m_mainWindow->stackedWidget);
			initializedDocks << m_axisDock;
			m_mainWindow->stackedWidget->addWidget(m_axisDock);
		}
		m_axisDock->activateTitleTab();
		break;
	case static_cast<quint64>(AspectType::CartesianPlot):
		if (!m_cartesianPlotDock) {
			m_cartesianPlotDock = new CartesianPlotDock(m_mainWindow->stackedWidget);
			initializedDocks << m_cartesianPlotDock;
			m_mainWindow->stackedWidget->addWidget(m_cartesianPlotDock);
		}
		m_cartesianPlotDock->activateTitleTab();
		break;
	case static_cast<quint64>(AspectType::CartesianPlotLegend):
		if (!m_cartesianPlotLegendDock) {
			m_cartesianPlotLegendDock = new CartesianPlotLegendDock(m_mainWindow->stackedWidget);
			initializedDocks << m_cartesianPlotLegendDock;
			m_mainWindow->stackedWidget->addWidget(m_cartesianPlotLegendDock);
		}
		m_cartesianPlotLegendDock->activateTitleTab();
		break;
	default:
		break;
	}
}
