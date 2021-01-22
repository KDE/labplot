/***************************************************************************
	File                 : StatisticsColumnWidget.h
    Project              : LabPlot
	Description          : Widget showing statistics for column values
    --------------------------------------------------------------------
	Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef STATISTICSCOLUMNWIDGET_H
#define STATISTICSCOLUMNWIDGET_H

#include <QWidget>

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

	const QString isNanValue(const double);

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
