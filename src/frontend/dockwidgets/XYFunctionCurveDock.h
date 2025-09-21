/*
	File             : XYFunctionCurveDock.h
	Project          : LabPlot
	Description      : widget for editing properties of function curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYFUNCTIONCURVEDOCK_H
#define XYFUNCTIONCURVEDOCK_H

#include "backend/worksheet/plots/cartesian/XYFunctionCurve.h"
#include "frontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xyfunctioncurvedockgeneraltab.h"

class XYFunctionCurveDock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYFunctionCurveDock(QWidget*);
	void setupGeneral() override;
	void setCurves(QList<XYCurve*>);

private:
	void initGeneralTab() override;

	Ui::XYFunctionCurveDockGeneralTab uiGeneralTab;
	XYFunctionCurve* m_functionCurve{nullptr};

private Q_SLOTS:
	// SLOTs for changes triggered in XYCurveDock
	void recalculateClicked() override;
	void showConstants();
	void showFunctions();
	void insertFunction(const QString&);
	void insertConstant(const QString&);
	void enableRecalculate() const override;
	void addVariable();
	void removeAllVariableWidgets();
	void deleteVariable();

	void variableNameChanged();
	void variableColumnChanged(const QModelIndex& index);

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveFunctionDataChanged(const XYFunctionCurve::FunctionData&);

	// Everything related to function handling

private:
	// variable widgets
	QGridLayout* m_gridLayoutCurves{nullptr};
	QGridLayout* m_gridLayoutVariables{nullptr};
	QList<QLineEdit*> m_variableLineEdits;
	QList<TreeViewComboBox*> m_variableComboBoxes;
	QList<QToolButton*> m_variableDeleteButtons;
};

#endif
