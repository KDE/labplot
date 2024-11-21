/*
	File             : XYConvolutionCurveDock.h
	Project          : LabPlot
	Description      : widget for editing properties of convolution curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYCONVOLUTIONCURVEDOCK_H
#define XYCONVOLUTIONCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"
#include "frontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xyconvolutioncurvedockgeneraltab.h"

class TreeViewComboBox;

class XYConvolutionCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYConvolutionCurveDock(QWidget*);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void showConvolutionResult();

	Ui::XYConvolutionCurveDockGeneralTab uiGeneralTab;
	XYConvolutionCurve* m_convolutionCurve{nullptr};
	XYConvolutionCurve::ConvolutionData m_convolutionData;

private Q_SLOTS:
	// SLOTs for changes triggered in XYConvolutionCurveDock
	// general tab
	void dataSourceTypeChanged(int);
	void xDataColumnChanged(const QModelIndex&);
	void samplingIntervalChanged();
	void kernelChanged();
	void kernelSizeChanged();
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void directionChanged();
	void typeChanged();
	void normChanged();
	void wrapChanged();
	void recalculateClicked();

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveY2DataColumnChanged(const AbstractColumn*);
	void curveConvolutionDataChanged(const XYConvolutionCurve::ConvolutionData&);
};

#endif
