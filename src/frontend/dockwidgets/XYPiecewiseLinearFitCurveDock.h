/*
	File                 : XYPiecewiseLinearFitCurveDock.h
	Project              : LabPlot
	Description          : widget for editing properties of piecewise linear fit curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYPIECEWISELINEARFITCURVEDOCK_H
#define XYPIECEWISELINEARFITCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYPiecewiseLinearFitCurve.h"
#include "frontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xypiecewiselinearfitcurvedockgeneraltab.h"

class XYPiecewiseLinearFitCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYPiecewiseLinearFitCurveDock(QWidget* parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void showFitResult();
	bool eventFilter(QObject*, QEvent*) override;

	Ui::XYPiecewiseLinearFitCurveDockGeneralTab uiGeneralTab;
	XYPiecewiseLinearFitCurve* m_fitCurve{nullptr};
	XYPiecewiseLinearFitCurve::FitData m_fitData;

private Q_SLOTS:
	// SLOTs for changes triggered in XYPiecewiseLinearFitCurveDock
	void dataSourceCurveChanged(const QModelIndex&);
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void methodChanged(int);
	void penaltyChanged();
	void minSegmentSizeChanged();
	void maxChangepointsChanged();
	void autoRangeChanged();
	void fitRangeMinChanged();
	void fitRangeMaxChanged();
	void autoEvalRangeChanged();
	void evalRangeMinChanged();
	void evalRangeMaxChanged();
	void evaluatedPointsChanged();
	void recalculateClicked() override;

	void showOptions(bool);
	void showResults(bool);

	// SLOTs for changes triggered in XYPiecewiseLinearFitCurve
	void curveFitDataChanged(const XYPiecewiseLinearFitCurve::FitData&);
};

#endif
