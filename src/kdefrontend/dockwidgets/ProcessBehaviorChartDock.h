/*
	File                 : ProcessBehaviorChartDock.h
	Project              : LabPlot
	Description          : widget for properties of the process behavior chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESSBEHAVIORCHARTDOCK_H
#define PROCESSBEHAVIORCHARTDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "backend/worksheet/plots/cartesian/ProcessBehaviorChart.h"
#include "ui_processbehaviorchartdock.h"

class LineWidget;
class ProcessBehaviorChart;
class SymbolWidget;
class TreeViewComboBox;

class ProcessBehaviorChartDock : public BaseDock {
	Q_OBJECT

public:
	explicit ProcessBehaviorChartDock(QWidget*);
	~ProcessBehaviorChartDock() override;

	void setPlots(QList<ProcessBehaviorChart*>);
	void updateLocale() override;

private:
	TreeViewComboBox* cbXDataColumn;
	TreeViewComboBox* cbYDataColumn;

	void load();
	void loadConfig(KConfig&);

protected:
	Ui::ProcessBehaviorChartDock ui;
	LineWidget* dataLineWidget{nullptr};
	SymbolWidget* dataSymbolWidget{nullptr};
	LineWidget* centerLineWidget{nullptr};
	LineWidget* upperLimitLineWidget{nullptr};
	LineWidget* lowerLimitLineWidget{nullptr};

	QList<ProcessBehaviorChart*> m_plots;
	ProcessBehaviorChart* m_plot{nullptr};

	virtual void setModel();

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in ProcessBehaviorChartDock

	// General-Tab
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void typeChanged(int);

	// SLOTs for changes triggered in ProcessBehaviorChart
	// General-Tab
	void plotXDataColumnChanged(const AbstractColumn*);
	void plotYDataColumnChanged(const AbstractColumn*);
	void plotTypeChanged(ProcessBehaviorChart::Type);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
