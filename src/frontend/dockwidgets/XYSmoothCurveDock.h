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

#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"
#include "frontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xysmoothcurvedockgeneraltab.h"

class XYSmoothCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYSmoothCurveDock(QWidget* parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updateSettings(const AbstractColumn*) override;
	void showSmoothResult();

	Ui::XYSmoothCurveDockGeneralTab uiGeneralTab;
	XYSmoothCurve* m_smoothCurve{nullptr};
	XYSmoothCurve::SmoothData m_smoothData;
	bool m_dateTimeRange{false};

private Q_SLOTS:
	// SLOTs for changes triggered in XYSmoothCurveDock
	// general tab
	void dataSourceTypeChanged(int);
	void xDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void xRangeMinDateTimeChanged(qint64);
	void xRangeMaxDateTimeChanged(qint64);
	void typeChanged(int);
	void pointsChanged(int);
	void weightChanged(int);
	void percentileChanged(double);
	void orderChanged(int);
	void modeChanged(int);
	void valueChanged();
	void recalculateClicked() override;

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveSmoothDataChanged(const XYSmoothCurve::SmoothData&);
};
#endif
