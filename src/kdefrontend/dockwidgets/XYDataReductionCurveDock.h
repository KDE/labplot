/***************************************************************************
    File             : XYDataReductionCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of data reduction curves

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

#ifndef XYDATAREDUCTIONCURVEDOCK_H
#define XYDATAREDUCTIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "ui_xydatareductioncurvedockgeneraltab.h"

class TreeViewComboBox;

class XYDataReductionCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYDataReductionCurveDock(QWidget *parent);
	void setCurves(QList<XYCurve*>);
	virtual void setupGeneral();

private:
	virtual void initGeneralTab();
	void showDataReductionResult();
	void updateTolerance();

	Ui::XYDataReductionCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbXDataColumn;
	TreeViewComboBox* cbYDataColumn;

	XYDataReductionCurve* m_dataReductionCurve;
	XYDataReductionCurve::DataReductionData m_dataReductionData;

protected:
	virtual void setModel();

private slots:
	//SLOTs for changes triggered in XYDataReductionCurveDock
	//general tab
	void nameChanged();
	void commentChanged();
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void typeChanged();
	void autoToleranceChanged();
	void toleranceChanged();

//	void showOptions();
	void recalculateClicked();

	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveDataReductionDataChanged(const XYDataReductionCurve::DataReductionData&);
	void dataChanged();

};

#endif
