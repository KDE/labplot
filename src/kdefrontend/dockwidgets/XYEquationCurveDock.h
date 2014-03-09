/***************************************************************************
    File             : XYEquationCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014 Alexander Semke (alexander.semke@web.de)
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
	virtual void setupGeneral();
	void setCurves(QList<XYCurve*>);

private:
	Ui::XYEquationCurveDockGeneralTab uiGeneralTab;
	virtual void initGeneralTab();
	XYEquationCurve* m_equationCurve;

private slots:
	//SLOTs for changes triggered in XYCurveDock
	void nameChanged();
	void commentChanged();	
	void typeChanged(int);
	void recalculateClicked();
	void validateExpression();
	void validateExpression(const QString&);

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveEquationDataChanged(const XYEquationCurve::EquationData&);
};

#endif
