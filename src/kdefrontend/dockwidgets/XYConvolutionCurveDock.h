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

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"
#include "ui_xyconvolutioncurvedockgeneraltab.h"

class TreeViewComboBox;

class XYConvolutionCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYConvolutionCurveDock(QWidget*);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updatePlotRanges() const override;
	void showConvolutionResult();

	Ui::XYConvolutionCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};
	TreeViewComboBox* cbY2DataColumn{nullptr};

	XYConvolutionCurve* m_convolutionCurve{nullptr};
	XYConvolutionCurve::ConvolutionData m_convolutionData;

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYConvolutionCurveDock
	//general tab
	void dataSourceTypeChanged(int);
	void dataSourceCurveChanged(const QModelIndex&);
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void y2DataColumnChanged(const QModelIndex&);
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
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveY2DataColumnChanged(const AbstractColumn*);
	void curveConvolutionDataChanged(const XYConvolutionCurve::ConvolutionData&);
	void dataChanged();
	void curveVisibilityChanged(bool);
};

#endif
