/*
	File                 : StatisticsColumnWidget.h
	Project              : LabPlot
	Description          : Widget showing statistics for column values
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATISTICSCOLUMNWIDGET_H
#define STATISTICSCOLUMNWIDGET_H

#include <QWidget>

class CartesianPlot;
class Column;
class Project;

class QTabWidget;
class QTextEdit;

class StatisticsColumnWidget : public QWidget {
	Q_OBJECT

public:
	explicit StatisticsColumnWidget(const Column*, QWidget* parent = nullptr);
	~StatisticsColumnWidget() override;
	void setCurrentTab(int);

private:
	void showOverview();
	void showOverviewPlot();
	void showHistogram();
	void showKDEPlot();
	void showQQPlot();
	void showBoxPlot();
	void showBarPlot();
	void showParetoPlot();

	CartesianPlot* addPlot(QWidget*);

	const QString isNanValue(const double) const;
	QString modeValue(const Column*, double) const;
	void copyValidData(QVector<double>&) const;

	const Column* m_column{nullptr}; // external column that the statistics has to be shown for
	Project* m_project{nullptr};
	QTabWidget* m_tabWidget{nullptr};
	QTextEdit* m_teOverview{nullptr};
	QWidget m_overviewWidget;
	QWidget m_overviewPlotWidget;
	QWidget m_histogramWidget;
	QWidget m_kdePlotWidget;
	QWidget m_qqPlotWidget;
	QWidget m_boxPlotWidget;
	QWidget m_barPlotWidget;
	QWidget m_paretoPlotWidget;

	QString m_htmlOverview;

	bool m_overviewInitialized{false};
	bool m_histogramInitialized{false};
	bool m_kdePlotInitialized{false};
	bool m_qqPlotInitialized{false};
	bool m_boxPlotInitialized{false};
	bool m_barPlotInitialized{false};
	bool m_paretoPlotInitialized{false};

private Q_SLOTS:
	void currentTabChanged(int);

Q_SIGNALS:
	void tabChanged(int);
};

#endif
