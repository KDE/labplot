/*
	File                 : KDEPlotDock.h
	Project              : LabPlot
	Description          : widget for KDE-plot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEPLOTDOCK_H
#define KDEPLOTDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_kdeplotdock.h"

class AspectTreeModel;
class BackgroundWidget;
class KDEPlot;
class LineWidget;
class TreeViewComboBox;

class KDEPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit KDEPlotDock(QWidget*);
	~KDEPlotDock() override;

	void setPlots(QList<KDEPlot*>);
	void updateLocale() override;

private:
	TreeViewComboBox* cbDataColumn;

	void updatePlotRanges() override;
	void load();
	void loadConfig(KConfig&);

protected:
	Ui::KDEPlotDock ui;
	LineWidget* estimationLineWidget{nullptr};
	BackgroundWidget* estimationBackgroundWidget{nullptr};
	LineWidget* histogramLineWidget{nullptr};
	BackgroundWidget* histogramBackgroundWidget{nullptr};

	QList<KDEPlot*> m_plots;
	KDEPlot* m_plot{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};

	virtual void setModel();

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in KDEPlotDock

	// General-Tab
	void dataColumnChanged(const QModelIndex&);
	void visibilityChanged(bool);

	// SLOTs for changes triggered in KDEPlot
	// General-Tab
	void plotDataColumnChanged(const AbstractColumn*);
	void plotVisibilityChanged(bool);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
