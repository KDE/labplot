/*
	File                 : BarPlotDock.h
	Project              : LabPlot
	Description          : Dock widget for the bar plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BARPLOTDOCK_H
#define BARPLOTDOCK_H

#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_barplotdock.h"

class AbstractAspect;
class AspectTreeModel;
class BackgroundWidget;
class LineWidget;
class ValueWidget;
class BarPlot;
class TreeViewComboBox;
class KConfig;

class BarPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit BarPlotDock(QWidget*);
	void setBarPlots(QList<BarPlot*>);
	void updateLocale() override;

private:
	Ui::BarPlotDock ui;
	BackgroundWidget* backgroundWidget{nullptr};
	LineWidget* lineWidget{nullptr};
	ValueWidget* valueWidget{nullptr};
	QList<BarPlot*> m_barPlots;
	BarPlot* m_barPlot{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	TreeViewComboBox* cbXColumn{nullptr};

	QGridLayout* m_gridLayout;
	QPushButton* m_buttonNew;
	QVector<TreeViewComboBox*> m_dataComboBoxes;
	QVector<QPushButton*> m_removeButtons;

	void setModel();
	void loadConfig(KConfig&);
	void setDataColumns() const;
	void loadDataColumns();

private Q_SLOTS:
	// SLOTs for changes triggered in BarPlotDock

	//"General"-tab
	void xColumnChanged(const QModelIndex&);
	void removeXColumn();
	void addDataColumn();
	void removeDataColumn();
	void dataColumnChanged(const QModelIndex&) const;
	void typeChanged(int) const;
	void orientationChanged(int) const;
	void visibilityChanged(bool) const;

	//"Box"-tab
	void currentBarChanged(int) const;
	void widthFactorChanged(int) const;

	// SLOTs for changes triggered in BarPlot
	// general
	void updatePlotRanges() override;
	void plotXColumnChanged(const AbstractColumn*);
	void plotDataColumnsChanged(const QVector<const AbstractColumn*>&);
	void plotTypeChanged(BarPlot::Type);
	void plotOrientationChanged(BarPlot::Orientation);
	void plotWidthFactorChanged(double);
	void plotVisibilityChanged(bool);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
