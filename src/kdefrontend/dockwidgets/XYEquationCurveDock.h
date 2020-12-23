/***************************************************************************
    File             : XYEquationCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Copyright        : (C) 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of equation curves

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef XYEQUATIONCURVEDOCK_H
#define XYEQUATIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "ui_xyequationcurvedockgeneraltab.h"

class XYEquationCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYEquationCurveDock(QWidget *parent);
	void setupGeneral() override;
	void setCurves(QList<XYCurve*>);

private:
	void initGeneralTab() override;
	void updatePlotRanges() const;

	Ui::XYEquationCurveDockGeneralTab uiGeneralTab;
	XYEquationCurve* m_equationCurve{nullptr};

private slots:
	//SLOTs for changes triggered in XYCurveDock
	void typeChanged(int);
	void recalculateClicked();
	void showConstants();
	void showFunctions();
	void insertFunction1(const QString&);
	void insertConstant1(const QString&);
	void insertFunction2(const QString&);
	void insertConstant2(const QString&);
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveEquationDataChanged(const XYEquationCurve::EquationData&);

	void plotRangeChanged(int);
};

#endif
