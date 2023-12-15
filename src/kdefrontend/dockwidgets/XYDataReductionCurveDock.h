/*
	File             : XYDataReductionCurveDock.h
	Project          : LabPlot
	Description      : widget for editing properties of data reduction curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYDATAREDUCTIONCURVEDOCK_H
#define XYDATAREDUCTIONCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "kdefrontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xydatareductioncurvedockgeneraltab.h"

class XYAnalysisCurve;
class TreeViewComboBox;
class QStatusBar;

class XYDataReductionCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYDataReductionCurveDock(QWidget* parent, QStatusBar* sb);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updateTolerance();
	void updateTolerance2();
	void showDataReductionResult();
	virtual QString customText() const override;

	Ui::XYDataReductionCurveDockGeneralTab uiGeneralTab;
	QStatusBar* statusBar; // main status bar to display progress

	XYDataReductionCurve* m_dataReductionCurve{nullptr};
	XYDataReductionCurve::DataReductionData m_dataReductionData;
	bool m_dateTimeRange{false};

protected:
	void setModel();

private Q_SLOTS:
	// SLOTs for changes triggered in XYDataReductionCurveDock
	// general tab
	void dataSourceTypeChanged(int);
	void xDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void xRangeMinDateTimeChanged(qint64);
	void xRangeMaxDateTimeChanged(qint64);
	void typeChanged(int);
	void autoToleranceChanged();
	void toleranceChanged(double);
	void autoTolerance2Changed();
	void tolerance2Changed(double);

	void recalculateClicked();

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveDataReductionDataChanged(const XYDataReductionCurve::DataReductionData&);
	void curveVisibilityChanged(bool);
};

#endif
