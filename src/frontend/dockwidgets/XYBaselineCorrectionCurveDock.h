/*
	File             : XYBaselineCorrectionCurveDock.h
	Project          : LabPlot
	Description      : widget for editing properties of baseline correction curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYBASELINECORRECTIONCURVEDOCK_H
#define XYBASELINECORRECTIONCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYBaselineCorrectionCurve.h"
#include "frontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xybaselinecorrectioncurvedockgeneraltab.h"

class XYBaselineCorrectionCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYBaselineCorrectionCurveDock(QWidget*);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;
	void updateLocale() override;
	void retranslateUi() override;

private:
	void initGeneralTab() override;
	// void updateSettings(const AbstractColumn*) override;
	void showBaselineCorrectionResult();

	Ui::XYBaselineCorrectionCurveDockGeneralTab uiGeneralTab;
	XYBaselineCorrectionCurve* m_baselineCurve{nullptr};
	XYBaselineCorrectionCurve::BaselineData m_data;
	bool m_dateTimeRange{false};

private Q_SLOTS:
	// SLOTs for changes triggered in XYBaselineCorrectionCurveDock
	// general tab
	void dataSourceTypeChanged(int);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void xRangeMinDateTimeChanged(qint64);
	void xRangeMaxDateTimeChanged(qint64);
	void methodChanged(int);
	void recalculateClicked() override;

	// SLOTs for changes triggered in XYBaselineCorrectionCurve
	// General-Tab
	void curveBaselineDataChanged(const XYBaselineCorrectionCurve::BaselineData&);
};

#endif
