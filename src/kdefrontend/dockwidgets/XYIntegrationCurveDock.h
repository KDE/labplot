/*
	File             : XYIntegrationCurveDock.h
	Project          : LabPlot
	Description      : widget for editing properties of integration curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYINTEGRATIONCURVEDOCK_H
#define XYINTEGRATIONCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "kdefrontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xyintegrationcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYIntegrationCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYIntegrationCurveDock(QWidget*);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updateSettings(const AbstractColumn*);
	void updatePlotRanges() override;
	void showIntegrationResult();

	virtual QString customText() const override;

	Ui::XYIntegrationCurveDockGeneralTab uiGeneralTab;

	XYIntegrationCurve* m_integrationCurve{nullptr};
	XYIntegrationCurve::IntegrationData m_integrationData;
	bool m_dateTimeRange{false};

protected:
	void setModel();

private Q_SLOTS:
	// SLOTs for changes triggered in XYIntegrationCurveDock
	// general tab
	void dataSourceTypeChanged(int);
	void dataSourceCurveChanged(const QModelIndex&);
	void xDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void xRangeMinDateTimeChanged(qint64);
	void xRangeMaxDateTimeChanged(qint64);
	void methodChanged(int);
	void absoluteChanged();

	void recalculateClicked();

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveIntegrationDataChanged(const XYIntegrationCurve::IntegrationData&);
	void dataChanged();
	void curveVisibilityChanged(bool);
};

#endif
