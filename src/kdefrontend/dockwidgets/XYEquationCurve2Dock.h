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

#include "backend/worksheet/plots/cartesian/XYEquationCurve2.h"
#include "kdefrontend/dockwidgets/XYAnalysisCurveDock.h"
#include "ui_xyequationcurve2dockgeneraltab.h"

class XYEquationCurve2Dock : public XYAnalysisCurveDock {
	Q_OBJECT

public:
	explicit XYEquationCurve2Dock(QWidget*);
	void setupGeneral() override;
	void setCurves(QList<XYCurve*>);

private:
	void initGeneralTab() override;

	Ui::XYEquationCurve2DockGeneralTab uiGeneralTab;
	XYEquationCurve2* m_equationCurve{nullptr};

private Q_SLOTS:
	// SLOTs for changes triggered in XYCurveDock
	void recalculateClicked();
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
	void curveEquationDataChanged(const XYEquationCurve2::EquationData&);

	// Everything related to equation handling
private:
	// variable widgets
	QGridLayout* m_gridLayoutCurves;
	QGridLayout* m_gridLayoutVariables;
	QList<QLineEdit*> m_variableLineEdits;
	QList<TreeViewComboBox*> m_variableComboBoxes;
	QList<QToolButton*> m_variableDeleteButtons;
};

#endif
