/*
	File                 : BarPlotDock.h
	Project              : LabPlot
	Description          : Dock widget for the bar plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BARPLOTDOCK_H
#define BARPLOTDOCK_H

#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_barplotdock.h"

class BackgroundWidget;
class DataColumnsWidget;
class ErrorBarWidget;
class LineWidget;
class TreeViewComboBox;
class ValueWidget;

class KConfig;

class BarPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit BarPlotDock(QWidget*);
	void setBarPlots(QList<BarPlot*>);
	void updateLocale() override;
	void retranslateUi() override;

private:
	Ui::BarPlotDock ui;
	BackgroundWidget* backgroundWidget{nullptr};
	LineWidget* lineWidget{nullptr};
	ValueWidget* valueWidget{nullptr};
	ErrorBarWidget* errorBarWidget{nullptr};
	QList<BarPlot*> m_barPlots;
	BarPlot* m_barPlot{nullptr};
	TreeViewComboBox* cbXColumn{nullptr};
	DataColumnsWidget* m_dataColumnsWidget{nullptr};

	void setModel();
	void load();
	void loadConfig(KConfig&);
	void loadDataColumns();

private Q_SLOTS:
	// SLOTs for changes triggered in BarPlotDock
	//"General"-tab
	void xColumnChanged(const QModelIndex&);
	void removeXColumn();
	void dataColumnsChanged(QVector<const AbstractColumn*>);
	void typeChanged(int);
	void orientationChanged(int);

	//"Bars"-tab
	void barNumberChanged(int);
	void widthFactorChanged(int);

	//"Error Bars"-tab
	void errorNumberChanged(int);

	// SLOTs for changes triggered in BarPlot
	// general
	void plotXColumnChanged(const AbstractColumn*);
	void plotDataColumnsChanged(const QVector<const AbstractColumn*>&);
	void plotTypeChanged(BarPlot::Type);
	void plotOrientationChanged(BarPlot::Orientation);
	void plotWidthFactorChanged(double);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
