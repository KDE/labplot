/*
	File             : XYFourierFilterCurveDock.h
	Project          : LabPlot
	Description      : widget for editing properties of Fourier filter curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYFOURIERFILTERCURVEDOCK_H
#define XYFOURIERFILTERCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "frontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xyfourierfiltercurvedockgeneraltab.h"

class XYFourierFilterCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYFourierFilterCurveDock(QWidget* parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updateSettings(const AbstractColumn*) override;
	void showFilterResult();
	void updateCutoffSpinBoxes(NumberSpinBox* sb, nsl_filter_cutoff_unit newUnit, nsl_filter_cutoff_unit oldUnit, double oldValue);

	Ui::XYFourierFilterCurveDockGeneralTab uiGeneralTab;
	XYFourierFilterCurve* m_filterCurve{nullptr};
	XYFourierFilterCurve::FilterData m_filterData;
	bool m_dateTimeRange{false};

private Q_SLOTS:
	// SLOTs for changes triggered in XYFourierFilterCurveDock
	// general tab
	void dataSourceTypeChanged(int);
	void xDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void xRangeMinDateTimeChanged(qint64);
	void xRangeMaxDateTimeChanged(qint64);
	void typeChanged();
	void formChanged();
	void orderChanged();
	void unitChanged();
	void unit2Changed();
	void recalculateClicked() override;

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveFilterDataChanged(const XYFourierFilterCurve::FilterData&);
};

#endif
