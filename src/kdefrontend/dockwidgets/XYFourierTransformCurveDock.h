/*
	File             : XYFourierTransformCurveDock.h
	Project          : LabPlot
	Description      : widget for editing properties of Fourier transform curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYFOURIERTRANSFORMCURVEDOCK_H
#define XYFOURIERTRANSFORMCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"
#include "kdefrontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xyfouriertransformcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYFourierTransformCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYFourierTransformCurveDock(QWidget* parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updatePlotRanges() override;
	void showTransformResult();

	Ui::XYFourierTransformCurveDockGeneralTab uiGeneralTab;

	XYFourierTransformCurve* m_transformCurve{nullptr};
	XYFourierTransformCurve::TransformData m_transformData;

protected:
	void setModel();

private Q_SLOTS:
	// SLOTs for changes triggered in XYFourierTransformCurveDock
	// general tab
	void xDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void windowTypeChanged();
	void typeChanged();
	void twoSidedChanged();
	void shiftedChanged();
	void xScaleChanged();

	//	void showOptions();
	void recalculateClicked();

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveTransformDataChanged(const XYFourierTransformCurve::TransformData&);
	void dataChanged();
	void curveVisibilityChanged(bool);
};

#endif
