/*
	File             : XYLineSimplificationCurveDock.h
	Project          : LabPlot
	Description      : widget for editing properties of line simplification curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYLINESIMPLIFICATIONCURVEDOCK_H
#define XYLINESIMPLIFICATIONCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYLineSimplificationCurve.h"
#include "frontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xylinesimplificationcurvedockgeneraltab.h"

class XYAnalysisCurve;
class TreeViewComboBox;
class QStatusBar;

class XYLineSimplificationCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYLineSimplificationCurveDock(QWidget* parent, QStatusBar* sb);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;
	void updateLocale() override;
	void retranslateUi() override;

private:
	void initGeneralTab() override;
	void updateSettings(const AbstractColumn*) override;
	void updateTolerance();
	void updateTolerance2();
	void showLineSimplificationResult();
	void updateOptionsTexts();
	virtual QString customText() const override;

	Ui::XYLineSimplificationCurveDockGeneralTab uiGeneralTab;
	QStatusBar* statusBar; // main status bar to display progress

	XYLineSimplificationCurve* m_lineSimplificationCurve{nullptr};
	XYLineSimplificationCurve::LineSimplificationData m_lineSimplificationData;
	bool m_dateTimeRange{false};

private Q_SLOTS:
	// SLOTs for changes triggered in XYLineSimplificationCurveDock
	// general tab
	void dataSourceTypeChanged(int);
	void xDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void xRangeMinDateTimeChanged(qint64);
	void xRangeMaxDateTimeChanged(qint64);
	void methodChanged(int);
	void autoToleranceChanged();
	void toleranceChanged(double);
	void autoTolerance2Changed();
	void tolerance2Changed(double);

	void recalculateClicked() override;

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveLineSimplificationDataChanged(const XYLineSimplificationCurve::LineSimplificationData&);
};

#endif
