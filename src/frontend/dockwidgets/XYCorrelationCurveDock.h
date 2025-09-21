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

#include "backend/worksheet/plots/cartesian/XYCorrelationCurve.h"
#include "frontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xycorrelationcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYCorrelationCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYCorrelationCurveDock(QWidget*);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void showCorrelationResult();

	Ui::XYCorrelationCurveDockGeneralTab uiGeneralTab;
	XYCorrelationCurve* m_correlationCurve{nullptr};
	XYCorrelationCurve::CorrelationData m_correlationData;

private Q_SLOTS:
	// SLOTs for changes triggered in XYCorrelationCurveDock
	// general tab
	void dataSourceTypeChanged(int);
	void xDataColumnChanged(const QModelIndex&);
	void samplingIntervalChanged();
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void typeChanged();
	void normChanged();
	void recalculateClicked() override;

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveY2DataColumnChanged(const AbstractColumn*);
	void curveCorrelationDataChanged(const XYCorrelationCurve::CorrelationData&);
};

#endif
