/*
    File             : XYDataReductionCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
    Description      : widget for editing properties of data reduction curves

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYDATAREDUCTIONCURVEDOCK_H
#define XYDATAREDUCTIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "ui_xydatareductioncurvedockgeneraltab.h"

class XYAnalysisCurve;
class TreeViewComboBox;
class QStatusBar;

class XYDataReductionCurveDock : public XYCurveDock {
	Q_OBJECT

public:
	explicit XYDataReductionCurveDock(QWidget *parent, QStatusBar *sb);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updatePlotRanges() const override;
	void updateTolerance();
	void updateTolerance2();
	void showDataReductionResult();

	Ui::XYDataReductionCurveDockGeneralTab uiGeneralTab;
	QStatusBar* statusBar;	// main status bar to display progress
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};

	XYDataReductionCurve* m_dataReductionCurve{nullptr};
	XYDataReductionCurve::DataReductionData m_dataReductionData;
	bool m_dateTimeRange{false};

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYDataReductionCurveDock
	//general tab
	void dataSourceTypeChanged(int);
	void dataSourceCurveChanged(const QModelIndex&);
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void xRangeMinDateTimeChanged(const QDateTime&);
	void xRangeMaxDateTimeChanged(const QDateTime&);
	void typeChanged(int);
	void autoToleranceChanged();
	void toleranceChanged(double);
	void autoTolerance2Changed();
	void tolerance2Changed(double);

	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveDataReductionDataChanged(const XYDataReductionCurve::DataReductionData&);
	void dataChanged();
	void curveVisibilityChanged(bool);
};

#endif
