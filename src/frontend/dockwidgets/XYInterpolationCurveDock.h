/*
	File             : XYInterpolationCurveDock.h
	Project          : LabPlot
	Description      : widget for editing properties of interpolation curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYINTERPOLATIONCURVEDOCK_H
#define XYINTERPOLATIONCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "frontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xyinterpolationcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYInterpolationCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYInterpolationCurveDock(QWidget* parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updateSettings(const AbstractColumn*) override;
	void showInterpolationResult();

	Ui::XYInterpolationCurveDockGeneralTab uiGeneralTab;
	XYInterpolationCurve* m_interpolationCurve{nullptr};
	XYInterpolationCurve::InterpolationData m_interpolationData;
	unsigned int dataPoints{0}; // number of data points in selected column
	bool m_dateTimeRange{false};

private Q_SLOTS:
	// SLOTs for changes triggered in XYInterpolationCurveDock
	// general tab
	void dataSourceTypeChanged(int);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void xRangeMinDateTimeChanged(qint64);
	void xRangeMaxDateTimeChanged(qint64);
	void typeChanged(int);
	void variantChanged(int);
	void tensionChanged(double);
	void continuityChanged(double);
	void biasChanged(double);
	void evaluateChanged(int);
	void numberOfPointsChanged();
	void pointsModeChanged(int);
	void recalculateClicked() override;

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveInterpolationDataChanged(const XYInterpolationCurve::InterpolationData&);
};

#endif
