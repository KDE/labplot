/*
    File             : XYCorrelationCurveDock.h
    Project          : LabPlot
    Description      : widget for editing properties of correlation curves
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYCORRELATIONCURVEDOCK_H
#define XYCORRELATIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYCorrelationCurve.h"
#include "ui_xycorrelationcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYCorrelationCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYCorrelationCurveDock(QWidget*);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updatePlotRanges() override;
	void showCorrelationResult();

	Ui::XYCorrelationCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};
	TreeViewComboBox* cbY2DataColumn{nullptr};

	XYCorrelationCurve* m_correlationCurve{nullptr};
	XYCorrelationCurve::CorrelationData m_correlationData;

protected:
	void setModel() override;

private Q_SLOTS:
	//SLOTs for changes triggered in XYCorrelationCurveDock
	//general tab
	void dataSourceTypeChanged(int);
	void dataSourceCurveChanged(const QModelIndex&);
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void y2DataColumnChanged(const QModelIndex&);
	void samplingIntervalChanged();
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void typeChanged();
	void normChanged();

	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveY2DataColumnChanged(const AbstractColumn*);
	void curveCorrelationDataChanged(const XYCorrelationCurve::CorrelationData&);
	void dataChanged();
	void curveVisibilityChanged(bool);
};

#endif
