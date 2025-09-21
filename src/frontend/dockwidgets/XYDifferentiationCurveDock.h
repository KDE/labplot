/*
	File             : XYDifferentiationCurveDock.h
	Project          : LabPlot
	Description      : widget for editing properties of differentiation curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYDIFFERENTIATIONCURVEDOCK_H
#define XYDIFFERENTIATIONCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "frontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xydifferentiationcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYDifferentiationCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYDifferentiationCurveDock(QWidget*);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updateSettings(const AbstractColumn*) override;
	void showDifferentiationResult();

	Ui::XYDifferentiationCurveDockGeneralTab uiGeneralTab;
	XYDifferentiationCurve* m_differentiationCurve{nullptr};
	XYDifferentiationCurve::DifferentiationData m_differentiationData;
	bool m_dateTimeRange{false};

private Q_SLOTS:
	// SLOTs for changes triggered in XYDifferentiationCurveDock
	// general tab
	void dataSourceTypeChanged(int);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void xRangeMinDateTimeChanged(qint64);
	void xRangeMaxDateTimeChanged(qint64);
	void derivOrderChanged(int);
	void accOrderChanged(int);
	void recalculateClicked() override;

	// SLOTs for changes triggered in XYDifferentiationCurve
	// General-Tab
	void curveDifferentiationDataChanged(const XYDifferentiationCurve::DifferentiationData&);
};

#endif
