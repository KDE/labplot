/***************************************************************************
    File             : XYFitCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Copyright        : (C) 2017-2018 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#ifndef XYFITCURVEDOCK_H
#define XYFITCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "ui_xyfitcurvedockgeneraltab.h"

class TreeViewComboBox;
class FitParametersWidget;

class XYFitCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYFitCurveDock(QWidget* parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void showFitResult();
	void updateSettings(const AbstractColumn*);

	Ui::XYFitCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve;
	TreeViewComboBox* cbXDataColumn;
	TreeViewComboBox* cbYDataColumn;
	TreeViewComboBox* cbXErrorColumn;
	TreeViewComboBox* cbYErrorColumn;
	FitParametersWidget* fitParametersWidget;

	XYFitCurve* m_fitCurve;
	XYFitCurve::FitData m_fitData;
	QList<double> parameters;
	QList<double> parameterValues;

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYFitCurveDock
	//general tab
	void nameChanged();
	void commentChanged();
	void dataSourceTypeChanged(int);
	void dataSourceCurveChanged(const QModelIndex&);
	void xWeightChanged(int);
	void yWeightChanged(int);
	void categoryChanged(int);
	void modelTypeChanged(int);
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void xErrorColumnChanged(const QModelIndex&);
	void yErrorColumnChanged(const QModelIndex&);

	void showDataOptions(bool);
	void showWeightsOptions(bool);
	void showFitOptions(bool);
	void showParametersOptions(bool);

	void showConstants();
	void showFunctions();
	void updateParameterList();
	void parametersChanged();
	void showOptions();
	void insertFunction(const QString&) const;
	void insertConstant(const QString&) const;
	void recalculateClicked();
	void updateModelEquation();
	void expressionChanged();
	void enableRecalculate() const;
	void resultParametersContextMenuRequest(QPoint);
	void resultGoodnessContextMenuRequest(QPoint);
	void resultLogContextMenuRequest(QPoint);
	void resultCopySelection();
	void resultCopyAll();

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveXErrorColumnChanged(const AbstractColumn*);
	void curveYErrorColumnChanged(const AbstractColumn*);
	void curveFitDataChanged(const XYFitCurve::FitData&);
	void dataChanged();
};

#endif
