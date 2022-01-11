/*
    File             : XYEquationCurveDock.h
    Project          : LabPlot
    Description      : widget for editing properties of equation curves
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2020-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYEQUATIONCURVEDOCK_H
#define XYEQUATIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "ui_xyequationcurvedockgeneraltab.h"

class XYEquationCurveDock : public XYCurveDock {
	Q_OBJECT

public:
	explicit XYEquationCurveDock(QWidget*);
	void setupGeneral() override;
	void setCurves(QList<XYCurve*>);

private:
	void initGeneralTab() override;
	void updatePlotRanges() override;

	Ui::XYEquationCurveDockGeneralTab uiGeneralTab;
	XYEquationCurve* m_equationCurve{nullptr};

private Q_SLOTS:
	//SLOTs for changes triggered in XYCurveDock
	void typeChanged(int);
	void recalculateClicked();
	void showConstants();
	void showFunctions();
	void insertFunction1(const QString&);
	void insertConstant1(const QString&);
	void insertFunction2(const QString&);
	void insertConstant2(const QString&);
	void enableRecalculate();

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveEquationDataChanged(const XYEquationCurve::EquationData&);
};

#endif
