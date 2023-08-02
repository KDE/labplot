/*
	File                 : ClevelandDotPlotDock.h
	Project              : LabPlot
	Description          : Dock widget for the Cleveland dot plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLEVELANDDOTPLOTDOCK_H
#define CLEVELANDDOTPLOTDOCK_H

#include "backend/worksheet/plots/cartesian/ClevelandDotPlot.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_clevelanddotplotdock.h"

class AbstractAspect;
class AspectTreeModel;
class SymbolWidget;
class ValueWidget;
class ClevelandDotPlot;
class TreeViewComboBox;
class KConfig;

class ClevelandDotPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit ClevelandDotPlotDock(QWidget*);
	void setPlots(QList<ClevelandDotPlot*>);
	void updateLocale() override;

private:
	Ui::ClevelandDotPlotDock ui;
	SymbolWidget* symbolWidget{nullptr};
	ValueWidget* valueWidget{nullptr};
	QList<ClevelandDotPlot*> m_plots;
	ClevelandDotPlot* m_plot{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
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
	// SLOTs for changes triggered in ClevelandDotPlotDock

	//"General"-tab
	void xColumnChanged(const QModelIndex&);
	void removeXColumn();
	void addDataColumn();
	void removeDataColumn();
	void dataColumnChanged(const QModelIndex&);
	void orientationChanged(int);
	void visibilityChanged(bool);

	//"Symbol"-tab
	void currentBarSymbolChanged(int);

	// SLOTs for changes triggered in Lollipop
	// general
	void updatePlotRanges() override;
	void plotXColumnChanged(const AbstractColumn*);
	void plotDataColumnsChanged(const QVector<const AbstractColumn*>&);
	void plotOrientationChanged(ClevelandDotPlot::Orientation);
	void plotVisibilityChanged(bool);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
