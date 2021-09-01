/*
    File             : XYFitCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2017-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    Description      : widget for editing properties of equation curves

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYFITCURVEDOCK_H
#define XYFITCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "ui_xyfitcurvedockgeneraltab.h"

class TreeViewComboBox;
class FitParametersWidget;
class KMessageWidget;

class XYFitCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYFitCurveDock(QWidget* parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updatePlotRanges() const override;
	void updateSettings(const AbstractColumn*);
	void showFitResult();
	bool eventFilter(QObject*, QEvent*) override;

	Ui::XYFitCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};
	TreeViewComboBox* cbXErrorColumn{nullptr};
	TreeViewComboBox* cbYErrorColumn{nullptr};
	FitParametersWidget* fitParametersWidget{nullptr};

	XYFitCurve* m_fitCurve{nullptr};
	XYFitCurve::FitData m_fitData;
	QList<double> parameters;
	QList<double> parameterValues;
	bool m_parametersValid{true};
	KMessageWidget* m_messageWidget{nullptr};

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYFitCurveDock
	//general tab
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
	void showParameters(bool);
	void showResults(bool);

	void showConstants();
	void showFunctions();
	void updateParameterList();
	void parametersChanged(bool updateParameterWidget = true);
	void parametersValid(bool);
	void showOptions();
	void insertFunction(const QString&) const;
	void insertConstant(const QString&) const;
	void setPlotXRange();
	void recalculateClicked();
	void updateModelEquation();
	void expressionChanged();
	void enableRecalculate();
	void resultParametersContextMenuRequest(QPoint);
	void resultGoodnessContextMenuRequest(QPoint);
	void resultLogContextMenuRequest(QPoint);
	void resultCopy(bool copyAll = false);
	void resultCopyAll();

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveXErrorColumnChanged(const AbstractColumn*);
	void curveYErrorColumnChanged(const AbstractColumn*);
	void curveFitDataChanged(const XYFitCurve::FitData&);
	void dataChanged();
	void curveVisibilityChanged(bool);
};

#endif
