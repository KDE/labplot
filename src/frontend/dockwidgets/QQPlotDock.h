/*
	File                 : QQPlotDock.h
	Project              : LabPlot
	Description          : widget for QQ-plot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QQPLOTDOCK_H
#define QQPLOTDOCK_H

#include "frontend/dockwidgets/BaseDock.h"
#include "ui_qqplotdock.h"

class LineWidget;
class QQPlot;
class SymbolWidget;
class TreeViewComboBox;

class QQPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit QQPlotDock(QWidget*);
	~QQPlotDock() override;

	void setPlots(QList<QQPlot*>);
	void updateLocale() override;

private:
	TreeViewComboBox* cbDataColumn;

	void load();
	void loadConfig(KConfig&);

protected:
	Ui::QQPlotDock ui;
	LineWidget* lineWidget{nullptr};
	SymbolWidget* symbolWidget{nullptr};

	QList<QQPlot*> m_plots;
	QQPlot* m_plot{nullptr};

	virtual void setModel();

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in QQPlotDock

	// General-Tab
	void dataColumnChanged(const QModelIndex&);
	void distributionChanged(int);

	// SLOTs for changes triggered in QQPlot
	// General-Tab
	void plotDataColumnChanged(const AbstractColumn*);
	void plotDistributionChanged(nsl_sf_stats_distribution);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
