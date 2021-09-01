/*
    File             : XYInterpolationCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of interpolation curves

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYINTERPOLATIONCURVEDOCK_H
#define XYINTERPOLATIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "ui_xyinterpolationcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYInterpolationCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYInterpolationCurveDock(QWidget *parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updateSettings(const AbstractColumn*);
	void updatePlotRanges() const override;
	void showInterpolationResult();

	Ui::XYInterpolationCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};

	XYInterpolationCurve* m_interpolationCurve{nullptr};
	XYInterpolationCurve::InterpolationData m_interpolationData;
	unsigned int dataPoints{0};	// number of data points in selected column
	bool m_dateTimeRange{false};

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYInterpolationCurveDock
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
	void variantChanged(int);
	void tensionChanged(double);
	void continuityChanged(double);
	void biasChanged(double);
	void evaluateChanged(int);
	void numberOfPointsChanged();
	void pointsModeChanged(int);

	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveInterpolationDataChanged(const XYInterpolationCurve::InterpolationData&);
	void dataChanged();
};

#endif
