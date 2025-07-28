/*
	File                 : ProcessBehaviorChartDock.h
	Project              : LabPlot
	Description          : widget for properties of the process behavior chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESSBEHAVIORCHARTDOCK_H
#define PROCESSBEHAVIORCHARTDOCK_H

#include "backend/worksheet/plots/cartesian/ProcessBehaviorChart.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_processbehaviorchartdock.h"

class KMessageWidget;
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
	void retranslateUi() override;

private:
	TreeViewComboBox* cbDataColumn{nullptr};
	TreeViewComboBox* cbData2Column{nullptr};

	void load();
	void loadConfig(KConfig&);

protected:
	Ui::ProcessBehaviorChartDock ui;
	LineWidget* dataLineWidget{nullptr};
	SymbolWidget* dataSymbolWidget{nullptr};
	LineWidget* centerLineWidget{nullptr};
	LineWidget* upperLimitLineWidget{nullptr};
	LineWidget* lowerLimitLineWidget{nullptr};
	LineWidget* labelsBorderLineWidget{nullptr};
	KMessageWidget* m_messageWidget{nullptr};

	QList<ProcessBehaviorChart*> m_plots;
	ProcessBehaviorChart* m_plot{nullptr};

	virtual void setModel();

private Q_SLOTS:
	// SLOTs for changes triggered in ProcessBehaviorChartDock
	// General-Tab
	void dataColumnChanged(const QModelIndex&);
	void data2ColumnChanged(const QModelIndex&);
	void typeChanged(int);
	void limitsTypeChanged(int);
	void limitsMetricChanged(int);
	void sampleSizeChanged(int);
	void maxUpperLimitChanged(const QString&);
	void minLowerLimitChanged(const QString&);
	void exactLimitsEnabledChanged(bool);
	void updateLowerLimitWidgets();
	void centerSpecificationChanged(const QString&);
	void lowerLimitSpecificationChanged(const QString&);
	void upperLimitSpecificationChanged(const QString&);

	// Labels-tab
	void labelsEnabledChanged(bool);
	void labelsPrecisionChanged(int);
	void labelsAutoPrecisionChanged(bool);
	void labelsFontChanged(const QFont&);
	void labelsFontColorChanged(const QColor&);
	void labelsBackgroundColorChanged(const QColor&);
	void labelsBorderShapeChanged(int);

	// SLOTs for changes triggered in ProcessBehaviorChart
	// General-Tab
	void plotDataColumnChanged(const AbstractColumn*);
	void plotData2ColumnChanged(const AbstractColumn*);
	void plotTypeChanged(ProcessBehaviorChart::Type);
	void plotLimitsTypeChanged(ProcessBehaviorChart::LimitsType);
	void plotLimitsMetricChanged(ProcessBehaviorChart::LimitsMetric);
	void plotSampleSizeChanged(int);
	void plotMaxUpperLimitChanged(double);
	void plotMinLowerLimitChanged(double);
	void plotExactLimitsEnabledChanged(bool);
	void plotCenterSpecificationChanged(double);
	void plotLowerLimitSpecificationChanged(double);
	void plotUpperLimitSpecificationChanged(double);

	// Labels-tab
	void plotLabelsEnabledChanged(bool);
	void plotLabelsAutoPrecisionChanged(bool);
	void plotLabelsPrecisionChanged(int);
	void plotLabelsFontChanged(const QFont&);
	void plotLabelsFontColorChanged(const QColor&);
	void plotLabelsBackgroundColorChanged(const QColor&);
	void plotLabelsBorderShapeChanged(TextLabel::BorderShape);

	void showStatusInfo(const QString&);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
