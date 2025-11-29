/*
	File                 : GuiObserver.h
	Project              : LabPlot
	Description          : GUI observer
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GUIOBSERVER_H
#define GUIOBSERVER_H

#include <QObject>

class MainWin;
class AbstractAspect;

class BaseDock;
class AspectDock;
class AxisDock;
class InfoElementDock;
class NoteDock;
class CartesianPlotDock;
class HistogramDock;
class BarPlotDock;
class LollipopPlotDock;
class BoxPlotDock;
class KDEPlotDock;
class QQPlotDock;
class ProcessBehaviorChartDock;
class HypothesisTestDock;
class CartesianPlotLegendDock;
class CustomPointDock;
class ReferenceLineDock;
class ReferenceRangeDock;
class RunChartDock;
class ColumnDock;
class LiveDataDock;
class MatrixDock;
class PivotTableDock;
class ProjectDock;
class SpreadsheetDock;
class StatisticsSpreadsheetDock;
class XYCurveDock;
class XYEquationCurveDock;
class XYFunctionCurveDock;
class XYDataReductionCurveDock;
class XYDifferentiationCurveDock;
class XYIntegrationCurveDock;
class XYInterpolationCurveDock;
class XYSmoothCurveDock;
class XYFitCurveDock;
class XYFourierFilterCurveDock;
class XYFourierTransformCurveDock;
class XYHilbertTransformCurveDock;
class XYConvolutionCurveDock;
class XYCorrelationCurveDock;
class WorksheetDock;
class ImageDock;
class LabelWidget;
class DatapickerImageWidget;
class DatapickerCurveWidget;
#ifdef HAVE_SCRIPTING
class ScriptDock;
#endif
class SeasonalDecompositionDock;
#ifdef HAVE_CANTOR_LIBS
class NotebookDock;
#endif

class GuiObserver : public QObject {
	Q_OBJECT

public:
	explicit GuiObserver(MainWin*);
	~GuiObserver() override;

private:
	MainWin* m_mainWindow{nullptr};
	friend class MainWin;

	AspectDock* m_aspectDock{nullptr};
	ColumnDock* m_columnDock{nullptr};
	LiveDataDock* m_liveDataDock{nullptr};
	MatrixDock* m_matrixDock{nullptr};
	NoteDock* m_notesDock{nullptr};
	PivotTableDock* m_pivotTableDock{nullptr};
	ProjectDock* m_projectDock{nullptr};
	SpreadsheetDock* m_spreadsheetDock{nullptr};
	StatisticsSpreadsheetDock* m_statisticsSpreadsheetDock{nullptr};

	// data picker
	DatapickerImageWidget* m_datapickerImageDock{nullptr};
	DatapickerCurveWidget* m_datapickerCurveDock{nullptr};

	// worksheet
	AxisDock* m_axisDock{nullptr};
	CartesianPlotDock* m_cartesianPlotDock{nullptr};
	CartesianPlotLegendDock* m_cartesianPlotLegendDock{nullptr};
	CustomPointDock* m_customPointDock{nullptr};
	ImageDock* m_imageDock{nullptr};
	InfoElementDock* m_infoElementDock{nullptr};
	LabelWidget* m_textLabelDock{nullptr};
	ReferenceLineDock* m_referenceLineDock{nullptr};
	ReferenceRangeDock* m_referenceRangeDock{nullptr};
	WorksheetDock* m_worksheetDock{nullptr};

	XYCurveDock* m_xyCurveDock{nullptr};
	XYEquationCurveDock* m_xyEquationCurveDock{nullptr};
	XYFunctionCurveDock* m_xyFunctionCurveDock{nullptr};

	// bar plots
	BarPlotDock* m_barPlotDock{nullptr};
	LollipopPlotDock* m_lollipopPlotDock{nullptr};

	// statistical plots
	BoxPlotDock* m_boxPlotDock{nullptr};
	HistogramDock* m_histogramDock{nullptr};
	KDEPlotDock* m_kdePlotDock{nullptr};
	QQPlotDock* m_qqPlotDock{nullptr};

	// continuous improvement plots
	ProcessBehaviorChartDock* m_processBehaviorChartDock{nullptr};
	RunChartDock* m_runChartDock{nullptr};

	// analysis plots
	XYDataReductionCurveDock* m_xyDataReductionCurveDock{nullptr};
	XYDifferentiationCurveDock* m_xyDifferentiationCurveDock{nullptr};
	XYIntegrationCurveDock* m_xyIntegrationCurveDock{nullptr};
	XYInterpolationCurveDock* m_xyInterpolationCurveDock{nullptr};
	XYSmoothCurveDock* m_xySmoothCurveDock{nullptr};
	XYFitCurveDock* m_xyFitCurveDock{nullptr};
	XYFourierFilterCurveDock* m_xyFourierFilterCurveDock{nullptr};
	XYFourierTransformCurveDock* m_xyFourierTransformCurveDock{nullptr};
	XYHilbertTransformCurveDock* m_xyHilbertTransformCurveDock{nullptr};
	XYConvolutionCurveDock* m_xyConvolutionCurveDock{nullptr};
	XYCorrelationCurveDock* m_xyCorrelationCurveDock{nullptr};

	// statistical analysis
	HypothesisTestDock* m_hypothesisTestDock{nullptr};

	// time series analysis
	SeasonalDecompositionDock* m_seasonalDecompositionDock{nullptr};

#ifdef HAVE_SCRIPTING
	ScriptDock* m_scriptDock{nullptr};
#endif

#ifdef HAVE_CANTOR_LIBS
	NotebookDock* m_notebookDock{nullptr};
#endif

private Q_SLOTS:
	void selectedAspectsChanged(const QList<AbstractAspect*>&);
	void hiddenAspectSelected(const AbstractAspect*);
};

#endif
