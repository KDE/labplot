/*
	File             : XYHilbertTransformCurveDock.h
	Project          : LabPlot
	Description      : widget for editing properties of Hilbert transform curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYHILBERTTRANSFORMCURVEDOCK_H
#define XYHILBERTTRANSFORMCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYHilbertTransformCurve.h"
#include "kdefrontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xyhilberttransformcurvedockgeneraltab.h"

class XYHilbertTransformCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYHilbertTransformCurveDock(QWidget* parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void showTransformResult();

	Ui::XYHilbertTransformCurveDockGeneralTab uiGeneralTab;
	XYHilbertTransformCurve* m_transformCurve{nullptr};
	XYHilbertTransformCurve::TransformData m_transformData;

private Q_SLOTS:
	// SLOTs for changes triggered in XYHilbertTransformCurveDock
	// general tab
	void xDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void typeChanged();
	void recalculateClicked();

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveTransformDataChanged(const XYHilbertTransformCurve::TransformData&);
};

#endif
