/*
	File                 : RunChartDock.h
	Project              : LabPlot
	Description          : widget for properties of the run chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RUNCHARTDOCK_H
#define RUNCHARTDOCK_H

#include "backend/worksheet/plots/cartesian/RunChart.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_runchartdock.h"

class LineWidget;
class RunChart;
class SymbolWidget;
class TreeViewComboBox;

class RunChartDock : public BaseDock {
	Q_OBJECT

public:
	explicit RunChartDock(QWidget*);
	~RunChartDock() override;

	void setPlots(QList<RunChart*>);
	void updateLocale() override;
	void retranslateUi() override;

private:
	TreeViewComboBox* cbDataColumn;
	void loadConfig(KConfig&);

protected:
	Ui::RunChartDock ui;
	LineWidget* dataLineWidget{nullptr};
	SymbolWidget* dataSymbolWidget{nullptr};
	LineWidget* centerLineWidget{nullptr};

	QList<RunChart*> m_plots;
	RunChart* m_plot{nullptr};

	virtual void setModel();

private Q_SLOTS:
	// SLOTs for changes triggered in RunChartDock
	void dataColumnChanged(const QModelIndex&);
	void centerMetricChanged(int);

	// SLOTs for changes triggered in RunChart
	void plotDataColumnChanged(const AbstractColumn*);
	void plotCenterMetricChanged(RunChart::CenterMetric);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
