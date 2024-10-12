/*
	File                 : ParetoChartDock.h
	Project              : LabPlot
	Description          : widget for properties of the Pareto chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARETOCHARTDOCK_H
#define PARETOCHARTDOCK_H

#include "backend/worksheet/plots/cartesian/ParetoChart.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_paretochartdock.h"

class BackgroundWidget;
class LineWidget;
class ParetoChart;
class SymbolWidget;
class TreeViewComboBox;
class ValueWidget;

class ParetoChartDock : public BaseDock {
	Q_OBJECT

public:
	explicit ParetoChartDock(QWidget*);
	~ParetoChartDock() override;

	void setPlots(QList<ParetoChart*>);
	void updateLocale() override;

private:
	TreeViewComboBox* cbDataColumn;
	void loadConfig(KConfig&);

protected:
	Ui::ParetoChartDock ui;
	BackgroundWidget* barBackgroundWidget{nullptr};
	LineWidget* barLineWidget{nullptr};
	ValueWidget* valueWidget{nullptr};
	LineWidget* lineWidget{nullptr};
	SymbolWidget* symbolWidget{nullptr};

	QList<ParetoChart*> m_plots;
	ParetoChart* m_plot{nullptr};

	virtual void setModel();

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in ParetoChartDock
	void dataColumnChanged(const QModelIndex&);

	// SLOTs for changes triggered in ParetoChart
	void plotDataColumnChanged(const AbstractColumn*);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
