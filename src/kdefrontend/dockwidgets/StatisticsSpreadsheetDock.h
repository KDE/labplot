/*
	File                 : StatisticsSpreadsheetDock.h
	Project              : LabPlot
	Description          : widget for statistics spreadsheet properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATISTICSSPREADSHEETDOCK_H
#define STATISTICSSPREADSHEETDOCK_H

#include "backend/spreadsheet/StatisticsSpreadsheet.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_statisticsspreadsheetdock.h"

class KConfig;
class StatisticsSpreadsheet;

class StatisticsSpreadsheetDock : public BaseDock {
	Q_OBJECT

public:
	explicit StatisticsSpreadsheetDock(QWidget*);
	void setSpreadsheets(QList<StatisticsSpreadsheet*>);

private:
	Ui::StatisticsSpreadsheetDock ui;
	QList<StatisticsSpreadsheet*> m_spreadsheets;
	StatisticsSpreadsheet* m_spreadsheet{nullptr};
	QMap<QCheckBox*, StatisticsSpreadsheet::Metric> m_mappingComboBoxMetric;

	void load();

private Q_SLOTS:
	void metricChanged(bool);
	void selectAll();
	void selectNone();

	// save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif // STATISTICSSPREADSHEETDOCK_H
