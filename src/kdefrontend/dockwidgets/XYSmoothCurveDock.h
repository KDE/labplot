/*
    File             : XYSmoothCurveDock.h
    Project          : LabPlot
    Description      : widget for editing properties of smooth curves
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYSMOOTHCURVEDOCK_H
#define XYSMOOTHCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"
#include "ui_xysmoothcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYSmoothCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYSmoothCurveDock(QWidget *parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updatePlotRanges() override;
	void showSmoothResult();

	Ui::XYSmoothCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};

	XYSmoothCurve* m_smoothCurve{nullptr};
	XYSmoothCurve::SmoothData m_smoothData;
	bool m_dateTimeRange{false};

protected:
	void setModel() override;

private Q_SLOTS:
	//SLOTs for changes triggered in XYSmoothCurveDock
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
	void pointsChanged(int);
	void weightChanged(int);
	void percentileChanged(double);
	void orderChanged(int);
	void modeChanged(int);
	void valueChanged();

	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveSmoothDataChanged(const XYSmoothCurve::SmoothData&);
	void dataChanged();
	void curveVisibilityChanged(bool);
};

#endif
