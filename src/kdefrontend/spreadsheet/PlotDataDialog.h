/*
	File                 : PlotDataDialog.h
	Project              : LabPlot
	Description          : Dialog for generating plots for the spreadsheet data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLOTDATADIALOG_H
#define PLOTDATADIALOG_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

namespace Ui {
class PlotDataWidget;
}

#include <QDialog>

class QComboBox;
class AbstractAspect;
class AspectTreeModel;
class CartesianPlot;
class AbstractColumn;
class Column;
class Spreadsheet;
class TreeViewComboBox;
class Worksheet;

class PlotDataDialog : public QDialog {
	Q_OBJECT

public:
	enum class PlotType { XYCurve, Histogram, BoxPlot, KDEPlot, QQPlot, BarPlot, LollipopPlot };

	explicit PlotDataDialog(Spreadsheet*, PlotType = PlotType::XYCurve, QWidget* parent = nullptr);
	~PlotDataDialog() override;

	void setAnalysisAction(XYAnalysisCurve::AnalysisAction);

private:
	Ui::PlotDataWidget* ui;
	QPushButton* m_okButton;
	Spreadsheet* m_spreadsheet;
	TreeViewComboBox* cbExistingPlots;
	TreeViewComboBox* cbExistingWorksheets;
	QVector<Column*> m_columns;
	QVector<QComboBox*> m_columnComboBoxes;
	AspectTreeModel* m_plotsModel;
	AspectTreeModel* m_worksheetsModel;
	PlotType m_plotType;
	XYAnalysisCurve::AnalysisAction m_analysisAction{XYAnalysisCurve::AnalysisAction::Differentiation};
	bool m_analysisMode{false};
	AbstractAspect* m_lastAddedCurve{nullptr};

	void processColumns();
	void processColumnsForXYCurve(const QStringList& columnNames, const QString& xColumnName);
	void processColumnsForHistogram(const QStringList&);

	void addCurvesToPlot(CartesianPlot*);
	void addCurvesToPlots(Worksheet*);

	void addCurve(const QString& name, Column* xColumn, Column* yColumn, CartesianPlot*);
	void addSingleSourceColumnPlot(const Column* column, CartesianPlot*);
	void addMultiSourceColumnsPlot(const QVector<const AbstractColumn*>&, CartesianPlot*);

	Column* columnFromName(const QString&) const;
	void adjustWorksheetSize(Worksheet*) const;
	void setAxesTitles(CartesianPlot*, const QString& yColumnName = QString()) const;

	void setAxesColumnLabels(CartesianPlot*, const QString& columnName);
	void setAxesColumnLabels(CartesianPlot*, const Column*);

private Q_SLOTS:
	void plot();
	void curvePlacementChanged();
	void plotPlacementChanged();
	void checkOkButton();
};

#endif
