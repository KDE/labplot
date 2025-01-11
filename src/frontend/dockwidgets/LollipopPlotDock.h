/*
	File                 : LollipopPlotDock.h
	Project              : LabPlot
	Description          : Dock widget for the lollipop plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LOLLIPOPPLOTDOCK_H
#define LOLLIPOPPLOTDOCK_H

#include "backend/worksheet/plots/cartesian/LollipopPlot.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_lollipopplotdock.h"

class AbstractAspect;
class LineWidget;
class SymbolWidget;
class ValueWidget;
class LollipopPlot;
class TreeViewComboBox;
class KConfig;

class LollipopPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit LollipopPlotDock(QWidget*);
	void setPlots(QList<LollipopPlot*>);
	void updateLocale() override;

private:
	Ui::LollipopPlotDock ui;
	LineWidget* lineWidget{nullptr};
	SymbolWidget* symbolWidget{nullptr};
	ValueWidget* valueWidget{nullptr};
	QList<LollipopPlot*> m_plots;
	LollipopPlot* m_plot{nullptr};
	TreeViewComboBox* cbXColumn{nullptr};

	QGridLayout* m_gridLayout;
	QPushButton* m_buttonNew;
	QVector<TreeViewComboBox*> m_dataComboBoxes;
	QVector<QPushButton*> m_removeButtons;

	void setModel();
	void load();
	void loadConfig(KConfig&);
	void setDataColumns() const;
	void loadDataColumns();

private Q_SLOTS:
	// SLOTs for changes triggered in LollipopPlotDock

	//"General"-tab
	void xColumnChanged(const QModelIndex&);
	void removeXColumn();
	void addDataColumn();
	void removeDataColumn();
	void dataColumnChanged(const QModelIndex&);
	void orientationChanged(int);

	//"Line"-tab
	void currentBarLineChanged(int);

	//"Line"-tab
	void currentBarSymbolChanged(int);

	// SLOTs for changes triggered in Lollipop
	// general
	void plotXColumnChanged(const AbstractColumn*);
	void plotDataColumnsChanged(const QVector<const AbstractColumn*>&);
	void plotOrientationChanged(LollipopPlot::Orientation);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
