/*
	File                 : StatisticsColumnWidget.h
    Project              : LabPlot
	Description          : Widget showing statistics for column values
    --------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
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
	explicit StatisticsColumnWidget(const Column*, QWidget *parent = nullptr);
	~StatisticsColumnWidget() override;
	void showStatistics();

private:
	void showOverview();
	void showHistogram();
	void showKDEPlot();
	void showQQPlot();
	void showBoxPlot();
	CartesianPlot* addPlot(QWidget*);

	const QString isNanValue(const double) const;
	QString modeValue(const Column*, double) const;
	void copyValidData(QVector<double>&) const;

	const Column* m_column;
	Project* m_project;
	QTabWidget* m_tabWidget;
	QTextEdit* m_teOverview;
	QWidget m_histogramWidget;
	QWidget m_kdePlotWidget;
	QWidget m_qqPlotWidget;
	QWidget m_boxPlotWidget;

	QString m_htmlText;

	bool m_overviewInitialized{false};
	bool m_histogramInitialized{false};
	bool m_kdePlotInitialized{false};
	bool m_qqPlotInitialized{false};
	bool m_boxPlotInitialized{false};

private slots:
	void currentTabChanged(int);
};

#endif
