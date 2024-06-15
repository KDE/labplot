/*
	File             : XYEquationCurve2Dock.h
	Project          : LabPlot
	Description      : widget for editing properties of equation curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYEQUATIONCURVE2DOCK_H
#define XYEQUATIONCURVE2DOCK_H

#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "ui_xyequationcurve2dockgeneraltab.h"

class XYEquationCurve2Dock : public XYCurveDock {
	Q_OBJECT

public:
	explicit XYEquationCurve2Dock(QWidget*);
	void setupGeneral() override;
	void setCurves(QList<XYCurve*>);

private:
	void initGeneralTab() override;

	Ui::XYEquationCurve2DockGeneralTab uiGeneralTab;
	XYEquationCurve* m_equationCurve{nullptr};

private Q_SLOTS:
	// SLOTs for changes triggered in XYCurveDock
	void typeChanged(int);
	void recalculateClicked();
	void showConstants();
	void showFunctions();
	void insertFunction1(const QString&);
	void insertConstant1(const QString&);
	void insertFunction2(const QString&);
	void insertConstant2(const QString&);
	void enableRecalculate();

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveEquationDataChanged(const XYEquationCurve::EquationData&);
};

#endif
