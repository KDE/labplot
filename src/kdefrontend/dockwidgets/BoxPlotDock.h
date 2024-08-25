/*
	File                 : BoxPlotDock.h
	Project              : LabPlot
	Description          : Dock widget for the box plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BOXPLOTDOCK_H
#define BOXPLOTDOCK_H

#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_boxplotdock.h"

class AspectTreeModel;
class BoxPlot;
class BackgroundWidget;
class LineWidget;
class SymbolWidget;
class TreeViewComboBox;
class QPushButton;
class KConfig;

class BoxPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit BoxPlotDock(QWidget*);
	void setBoxPlots(QList<BoxPlot*>);
	void updateLocale() override;

private:
	Ui::BoxPlotDock ui;
	BackgroundWidget* backgroundWidget{nullptr};
	SymbolWidget* symbolWidget{nullptr};
	LineWidget* borderLineWidget{nullptr};
	LineWidget* medianLineWidget{nullptr};
	LineWidget* whiskersLineWidget{nullptr};
	LineWidget* whiskersCapLineWidget{nullptr};

	QList<BoxPlot*> m_boxPlots;
	BoxPlot* m_boxPlot{nullptr};

	QGridLayout* m_gridLayout;
	QPushButton* m_buttonNew;
	QVector<TreeViewComboBox*> m_dataComboBoxes;
	QVector<QPushButton*> m_removeButtons;

	void load();
	void loadConfig(KConfig&);
	void setModel();
	void setDataColumns() const;
	void loadDataColumns();
	void updateSymbolWidgets();

private Q_SLOTS:
	// SLOTs for changes triggered in BoxPlotDock

	//"General"-tab
	void addDataColumn();
	void removeDataColumn();
	void dataColumnChanged(const QModelIndex&);
	void orderingChanged(int);
	void orientationChanged(int);
	void variableWidthChanged(bool);
	void notchesEnabledChanged(bool);

	//"Box"-tab
	void currentBoxChanged(int);
	void widthFactorChanged(int);

	// symbols
	void symbolCategoryChanged();
	void jitteringEnabledChanged(bool);

	// whiskers
	void whiskersTypeChanged(int);
	void whiskersRangeParameterChanged(const QString&);
	void whiskersCapSizeChanged(double) const;

	//"Margin Plots"-Tab
	void rugEnabledChanged(bool);
	void rugLengthChanged(double) const;
	void rugWidthChanged(double) const;
	void rugOffsetChanged(double) const;

	// SLOTs for changes triggered in BoxPlot
	// general
	void plotDescriptionChanged(const AbstractAspect*);
	void plotDataColumnsChanged(const QVector<const AbstractColumn*>&);
	void plotOrderingChanged(BoxPlot::Ordering);
	void plotOrientationChanged(BoxPlot::Orientation);
	void plotVariableWidthChanged(bool);
	void plotWidthFactorChanged(double);
	void plotNotchesEnabledChanged(bool);

	// symbols
	void plotJitteringEnabledChanged(bool);

	// whiskers
	void plotWhiskersTypeChanged(BoxPlot::WhiskersType);
	void plotWhiskersRangeParameterChanged(double);
	void plotWhiskersCapSizeChanged(double);

	//"Margin Plots"-Tab
	void plotRugEnabledChanged(bool);
	void plotRugLengthChanged(double);
	void plotRugWidthChanged(double);
	void plotRugOffsetChanged(double);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
