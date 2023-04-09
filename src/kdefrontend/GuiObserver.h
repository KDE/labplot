/*
	File                 : GuiObserver.h
	Project              : LabPlot
	Description          : GUI observer
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2023 Alexander Semke <alexander.semke@web.de>
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
class BoxPlotDock;
class CartesianPlotLegendDock;
class CustomPointDock;
class ReferenceLineDock;
class ReferenceRangeDock;
class ColumnDock;
class LiveDataDock;
class MatrixDock;
class ProjectDock;
class SpreadsheetDock;
class XYCurveDock;
class XYEquationCurveDock;
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

#ifdef HAVE_CANTOR_LIBS
class CantorWorksheetDock;
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
	AxisDock* m_axisDock{nullptr};
	NoteDock* m_notesDock{nullptr};
	InfoElementDock* m_infoElementDock{nullptr};
	CartesianPlotDock* m_cartesianPlotDock{nullptr};
	CartesianPlotLegendDock* m_cartesianPlotLegendDock{nullptr};
	ColumnDock* m_columnDock{nullptr};
	LiveDataDock* m_liveDataDock{nullptr};
	MatrixDock* m_matrixDock{nullptr};
	SpreadsheetDock* m_spreadsheetDock{nullptr};
	ProjectDock* m_projectDock{nullptr};
	XYCurveDock* m_xyCurveDock{nullptr};
	XYEquationCurveDock* m_xyEquationCurveDock{nullptr};
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
	HistogramDock* m_histogramDock{nullptr};
	BarPlotDock* m_barPlotDock{nullptr};
	BoxPlotDock* m_boxPlotDock{nullptr};
	WorksheetDock* m_worksheetDock{nullptr};
	LabelWidget* m_textLabelDock{nullptr};
	ImageDock* m_imageDock{nullptr};
	CustomPointDock* m_customPointDock{nullptr};
	ReferenceLineDock* m_referenceLineDock{nullptr};
	ReferenceRangeDock* m_referenceRangeDock{nullptr};
	DatapickerImageWidget* m_datapickerImageDock{nullptr};
	DatapickerCurveWidget* m_datapickerCurveDock{nullptr};

#ifdef HAVE_CANTOR_LIBS
	CantorWorksheetDock* m_cantorWorksheetDock{nullptr};
#endif

private Q_SLOTS:
	void selectedAspectsChanged(QList<AbstractAspect*>&);
	void hiddenAspectSelected(const AbstractAspect*);
};

#endif
