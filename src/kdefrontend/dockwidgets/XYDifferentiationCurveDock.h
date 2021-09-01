/*
    File             : XYDifferentiationCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
    Description      : widget for editing properties of differentiation curves

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYDIFFERENTIATIONCURVEDOCK_H
#define XYDIFFERENTIATIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "ui_xydifferentiationcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYDifferentiationCurveDock : public XYCurveDock {
	Q_OBJECT

public:
	explicit XYDifferentiationCurveDock(QWidget*);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updatePlotRanges() const override;
	void updateSettings(const AbstractColumn*);
	void showDifferentiationResult();

	Ui::XYDifferentiationCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};

	XYDifferentiationCurve* m_differentiationCurve{nullptr};
	XYDifferentiationCurve::DifferentiationData m_differentiationData;
	bool m_dateTimeRange{false};

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYDifferentiationCurveDock
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
	void derivOrderChanged(int);
	void accOrderChanged(int);

	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYDifferentiationCurve
	//General-Tab
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveDifferentiationDataChanged(const XYDifferentiationCurve::DifferentiationData&);
	void dataChanged();
	void curveVisibilityChanged(bool);
};

#endif
